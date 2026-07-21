# Documentation of McCPP

## TODO
- Fix/test packets within the Login state
- Intermittent join failure: client sometimes gets "Connection Lost" on first join attempt with a decode error naming an unrelated packet (seen: "Packet play/clientbound/minecraft:award_stats ... found 7 bytes extra"); this server never sends stats packets, so it's a symptom of some earlier packet's framing, not that packet itself. Pre-existing (not caused by the chat/commands feature), not yet reproduced reliably enough to debug -- retrying the connection has always worked so far.



## CUDA Documentation
- Found post about linking Cuda code with C++ code [here](https://stackoverflow.com/questions/9421108/how-can-i-compile-cuda-code-then-link-it-to-a-c-project)

## G++ Flags
- This page explains the possible optimization flags for g++ [here](https://clang.llvm.org/docs/CommandGuide/clang.html#code-generation-options)

## Minecraft Documentation
Much of the work done deconstructing minecraft and its network protocol has been done and documented on wiki.vg.  Sadly wiki.vg has been taken down by the owner and the pages have been moved [here](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge).  Included in this repo I have a markdown copy of the protocol documentation as a contingency.

## JSON data handling
We use a header library from this repo https://github.com/nlohmann/json many thanks for this ease of use.

## Networking

### Packets
For both my deserialization and serialization I use std::vectors since I use the flexibility of the vector in both methods various implementations.  Deserialization uses it to shave off the size of the packet and packet ID varints off of the packet before passing the data of the packet to the deserialize method.  The resizing of this vector may be slower with the logical approach of abstraction and removing responsilibity to repare the beginning of the packet within the deserialize method.  It may be faster to keep it one static std::array and re-parse as needed, or even slice array as needed.  For serialization I use the variable size of the vector because of the varying size of strings/JSONs being parsed.  Some of this is mitigated possibly by including some reserve method calls as needed.  Overall the amount of memory reallocations could be drastic here and should be considered in the future if these method calls are consuming a large amount of time.

### Packet Serialization (Outgoing packets)
These outgoing packets must be given their needed data at the time of their construction so that the serialize method does not need to fetch the data during serialization.  These methods are already a mess to look at.

For much of my packet serialization I append a +1 to the packet length.  This is because nearly all of the packet ID's are less than 128 (2^7) which is one varInt.  Regardless we can know the length of the varint as we write these serializations we can append the correct length saving a lot of unnecessary work.

### Packet Deserialization (Incoming packets)
As mentioned above, the use of ```std::arrays``` should be investigated here.

The packet deserialization methods are  closely coupled with the Connection class so that the second the server receives a connection change state packet it is done immediately so as to not create any race conditions between future expected packets and whatever would be handling the task of changing the state at a future time.  I have yet to think of a cleaner way of implementing this at the moment.

I might create a general incoming packet constructor to store a reference to the packets Connection parent so that it does not need to be passed as a argument to the deserialize method.  But I may just be nit picky.

### Packet Ordering
Here I will document my ordering for the packets from when the client first initiates the handshake up until the play state.


```
Handshake:

Client -> Server : Handshake_p [0x00]
- Create the client connection
- transition it to either Status or Login state

Status:

Client -> Server : Status_Request_p [0x00]
- Queue the Status Response packet

Server -> Client : Status_Response_p [0x00]
- Send the JSON with the server data

Client -> Server : Ping_Request_p [0x01]
- Receive timestamp as a long

Server -> Client : Ping_Response_p [0x01]
- Send back the timestamp long

Login:

Client -> Server : Login_Start_p [0x00]
- Receive player login info

Server -> Client : Encryption_Request_p [0x01]
- Send player Encryption data

Client -> Server : Encryption_Response_p [0x01]
- We receive verification from the client

Server -> Client : Set_Compression_p [0x03]
- Sends compression threshold value
- All further packets will be compressed now
- Optional Packet

Server -> Client : Login_Success_p [0x02]
- Send client info with confirmation

Client -> Server : Login_Acknowledge_p [0x03]
- No data
- Acks the login success
- Transition to Config state

Config:

Server -> Client : Registry_Data_p [0x07]
- Game registers biomes, dimensions, etc

Server -> Client : Feature_Flags_p [0x0C]
- Enabled game features

Server -> Client : Update_Tags_config_p [0x0D]
- Block/Item/Entity tags

Server -> Client : Clientbound_Known_Packs_p [0x0E]
- Datapacks on the server

Client -> Server : Client_Information_config_p [0x00]
- player view distance, chat settings, more...

Client -> Server : Serverbound_Known_Packs_p [0x07]
- The Clients data packs in use (?)

Server -> Finish_Config_p [0x03]
- We have finished configuration

Client -> Server : Acknowledge_Finish_Config_p [0x03]
- Client is ready for play
- Switch to play state
- Unsure if player only sends this after the packet above is sent
```

## JSON Parsing
Using [this](https://github.com/nlohmann/json) library to parse json's.  Its a header library in the ```include/lib``` directory.  

## Multithreading

### Singleton construction/destruction order
Meyer's singletons (`static X instance;` inside `getInstance()`) are destroyed in the *reverse* of their construction order at program exit. If a singleton's background thread/pool captures another singleton by raw pointer or reference (e.g. a lambda capturing `this`), that other singleton must be constructed *before* the pool starts, or a task still running/queued at shutdown can touch an already-destroyed object.

Two concrete cases in `MinecraftServer.cpp`'s `main()`:
- `World::getInstance()` is called explicitly before `WorldWorkerPool::getInstance().initialize()`, even though `World` would otherwise construct lazily on the first player connection (which happens *after* the pool exists). `WorldWorkerPool`'s queued chunk-generation tasks capture `this` as a `World*` and touch its members directly.
- `ConnectionManager::getInstance().initialize()` runs before `TickLoop::getInstance().initialize()`, since the tick thread reaches into `ConnectionManager` every tick and needs to stop first.

(`WorldWorkerPool`'s tasks also hold `shared_ptr<Connection>` captures rather than raw pointers, so they stay safe to run even after `ConnectionManager` itself is torn down — only the raw-pointer case needs the construction-order fix.)

This was reliably reproduced (2026-07-18) with an isolated test program linking the project's real compiled `.o` files: bad ordering crashed with an out-of-bounds read into the noise permutation table when a queued task ran after `World`'s destruction; correct ordering exited cleanly.

### ThreadPool shutdown semantics
`ThreadPool::~ThreadPool()` drops any *unstarted* queued tasks and only waits for already-running tasks to finish. This used to drain the entire queue before shutting down (a deliberate safety-first choice from early on), but that became a real problem once `WorldWorkerPool` had real, possibly large backlogs of chunk-gen/lighting work queued (e.g. stopping the server mid-exploration) — shutdown would hang waiting for the whole backlog instead of just in-flight work. Verified via a synthetic 400-task backlog test: close time dropped from a multi-second drain to ~374ms.

## World Generation & Lighting

### Terrain cache vs. lit cache
`World` keeps two separate chunk caches: `_terrainCache` (block data only, safe for concurrent reads since it's never mutated after generation) and `_chunkCache` (fully lit, what actually gets sent to players). `LightEngine` needs an 8-neighbor lookup of raw block data to compute a chunk's lighting correctly, and routing those lookups through `getOrGenerateTerrain` means each neighboring chunk's terrain is only ever generated once, no matter how many nearby chunks need it purely for lighting occlusion — instead of regenerating it from scratch on every lookup.

### LightEngine
Full BFS flood-fill for both sky and block light, computed once at chunk-generation time (no incremental re-lighting, since there's no block editing yet). It operates over a 3x3-chunk (48x48 block) neighborhood centered on the target chunk — this is *exactly* correct, never approximate, because the max light level (15) is less than chunk width (16), so light can never propagate further than one chunk in from the target's own borders.

Sky light propagating straight down through a transparent block doesn't attenuate, unlike every other direction (which decays by `max(opacity, 1)`) — this special case isn't spelled out by `network-protocol.md`, but has been confirmed correct through testing.

Seeding is done in two passes for performance: the first pass walks each column top-down and sets sky light values directly (15 down to the first opaque block) without touching the BFS queue at all; the second pass only *enqueues* cells that sit at an actual light/dark boundary (a horizontal neighbor not already at max light, or the buffer edge). A wide-open sky column can be a couple hundred blocks of uniform max-light air, and enqueueing every one of those cells individually made the BFS pop/check millions of neighbors that could never produce a new result.

### Chunk dispatch/delivery ordering
`UpdateLoadedChunks` (in `Play.cpp`) dispatches and delivers chunks nearest-to-center first, not in raster/`std::set` order. `Connection::sendPackets()` serializes and writes queued packets to the socket strictly in order, one at a time — so if a whole ring's worth of chunks becomes ready at once (e.g. all cache hits on reconnecting to an already-explored area), the chunk directly under the player's own feet could end up serialized behind ~200 others. The client's "waiting for chunks" gate (which holds off gravity until its own chunk arrives) would then release too late, and the player free-falls into the ground before their own chunk shows up. This was found from a real "stuck underground on reconnect" bug report and confirmed via the client's F3 debug screen, not assumed from reading the code.

### Heightmap encoding
`Chunk_Data_p` sends a `MOTION_BLOCKING` heightmap: for each of a chunk's 256 columns, the Y of the highest non-air block, packed as `(y - WORLD_MIN_Y + 1)` (0 meaning "nothing blocking in this column") at 9 bits/entry, 7 entries per long (never straddling a long boundary), column index `z*16 + x`. This is vanilla's own encoding scheme. Its practical effect today is minimal: it's mainly used client-side for weather-particle rendering (rain/snow stop at the heightmap surface instead of an assumed y=0), and there's no weather yet.

### Performance history
Two performance passes, both 2026-07-18:

1. **Algorithmic fixes** — the terrain/lit cache split above; memoizing `NoiseChunkGenerator`'s per-column `isSolid()` results instead of recomputing them up to `DIRT_DEPTH+1` times per solid block for surface layering; and the two-pass sky-light seeding above. Combined: ~58s -> ~21s for a full 441-chunk view-distance batch (measured with an isolated timing harness linked against the real `.o` files).
2. **Build flags** — turned out to be a bigger factor than any single algorithmic fix. The Makefile's default build had no `-O` flag at all (effectively `-O0`). Adding `-O2` to the default build alone (no code changes) took `NoiseChunkGenerator::generate()` from 50.8ms to 11.8ms and `LightEngine::computeLighting()` (including its 8 neighbor terrain gens) from 362.6ms to 105.1ms — about 3.5x. `debug` now explicitly forces `-O0` back on top so gdb stepping stays reliable; the old separate `make fast`/`-Ofast` target was removed since `-O2` is just the default now, not an opt-in.

## Wire format gotchas found the hard way
A couple of `Chunk_Data_p` details that don't match a naive reading of the wiki summary, both confirmed against decompiled 1.21 client bytecode:
- A single-valued (Bits Per Entry = 0) paletted container is still followed by a Data Array Length VarInt (always 0 in that case) — it's present but zero-length, not omitted entirely.
- Each chunk section has exactly one non-empty-block-count short on the wire (blocks and fluids combined), not separate block/fluid counts.

## Block breaking and placing
Breaking (`Player_Action_p`) and placing (`Use_Item_On_p`) edit the world via `World::setBlock`, which copies the target chunk, edits and relights the copy, then swaps it into the cache rather than mutating the shared original in place — `Chunk` has no internal locking, so this avoids racing other threads reading the same cached `shared_ptr`. Edits broadcast a `Block_Update_p` to every player with that chunk loaded, and ack the acting player's client (`Acknowledge_Block_Change_p`) so its local prediction doesn't flicker/revert.

`Player` has a minimal 9-slot hotbar (no full inventory, no Click Container support), seeded with a starting stack at join and sent via `Set_Container_Content_p`. Breaking a known block drops a visible item entity (`Spawn_Entity_p` + `Set_Entity_Metadata_p`), tracked server-side by `ItemEntityManager` so it can be picked up or despawned afterward.

Pickup is checked on every `Set_Player_Position_p`/`Set_Player_Position_and_Rotation_p`, matching vanilla's own documented behavior (`docs/network-protocol.md`'s Pickup Item section) rather than a per-tick position sweep: nearby claimed items merge into an existing hotbar stack or the first empty slot, all-or-nothing (no partial pickup across the ground/inventory boundary). `ItemEntityManager::tryClaim` makes the claim atomic so two players in range at once can't both collect the same drop. Unclaimed drops despawn after 5 minutes via `ItemDespawnSystem`, a `TickSystem` registered alongside `KeepAliveSystem`.

Dropped items also get real per-tick physics via `ItemPhysicsSystem` (gravity + single-column ground collision against `World`, re-checked every tick so breaking the block out from under a resting item makes it fall again rather than becoming permanently stuck at its stale spawn position). Pressing Q (`Player_Action_p` status 3/4, "Drop item stack"/"Drop item") spawns a tossed item using the same physics, launched from the player's facing direction (yaw/pitch) with a small upward kick — the toss speed/arc is a gameplay-feel approximation, not a wire-format detail, so it wasn't decompile-verified the way packet layouts are.

Q-drop's toss direction depends on `Player`'s stored yaw/pitch being current, which needed `Set_Player_Rotation_p` (0x1C) -- sent when the client turns without moving its feet, as opposed to `Set_Player_Position_and_Rotation_p` (0x1B) which only fires alongside a position change. It wasn't registered, so standing still and looking around before dropping used a stale facing direction.

`ItemPhysicsSystem` does *not* broadcast a position correction every tick — an early version did, and it visibly fought the client's own local physics prediction for the entity (barely noticeable for a straight vertical fall's tiny per-tick steps, but a visible forward stutter for a toss's larger horizontal ones). The client is expected to simulate gravity/drag locally from the last velocity it was told, so the server only needs to correct it (`Teleport_Entity_p` + `Set_Entity_Velocity_p`, sent together) when velocity changes by more than a small threshold (squared magnitude > 0.1) — e.g. once at toss/support-loss, once at landing — not continuously. `ItemEntityManager`'s internal position is still updated every tick regardless, since pickup-radius and despawn checks need it current even on ticks where nothing is broadcast.

Wire formats not covered by `docs/network-protocol.md`, verified through testing rather than guessed: the 1.20.5+ Slot/Data-Components format, the player inventory's 46-slot layout (hotbar at indices 36-44), and the item entity's metadata index/type for its displayed stack (index 8, type 7).

Known gaps: no reach or mining-time validation (breaking is instant), placing never replaces the clicked block itself (can't place into water/tall grass), and terrain used for lighting neighbors isn't updated by edits.

## Player entities, skins, and online-mode auth

Login is real online-mode Mojang authentication when `server.properties`' `online-mode` is true (the default): the server computes the vanilla join-hash (`computeServerHash` in `Crypto.hpp`, `SHA1(serverId + sharedSecret + publicKeyDer)` formatted as a signed hex `BigInteger`-style string) and calls `sessionserver.mojang.com`'s `hasJoined` endpoint over a small hand-rolled HTTPS client (`network/HttpsClient.hpp`, TLS via the already-linked OpenSSL, no new library dependency) before trusting the client's claimed UUID/username, replacing them with Mojang's verified values and storing the returned "textures" property (`PlayerProfileProperty` on `Player`) for skin rendering. The join-hash hex formatting was verified against the standard published SHA1("Notch")/"jeb_"/"simon" test vectors (including the negative two's-complement case), not trusted from memory alone. `online-mode=false` skips all of this and falls back to trusting the client, same as before this feature existed.

Two pre-existing bugs in `Crypto.cpp` were fixed as a prerequisite: the RSA keypair was being regenerated (not reused) on every call, which would have made the join-hash never match what was sent to the client; and the verify token was a single global instead of per-connection, letting concurrent logins clobber each other.

Other players become visible in two independent ways: a server-wide tab list (`Player_Info_Update_p`/`Player_Info_Remove_p`, synced once at join via `BroadcastPlayerJoin`) and chunk/proximity-based in-world entities (`PlayerVisibilityManager`, hooked into `Connection::deliverGeneratedChunks` — refreshing whichever connection's own loaded-chunk set just changed is enough to catch both directions of visibility, since the check itself is bidirectional). A joining player's own `Player_Info_Update_p` entry is sent back to themselves too, not just broadcast to others — the client needs it (skin variant, cape) to render its own third-person model, not only the tab list.

Wire formats not fully covered by `docs/network-protocol.md`, verified rather than guessed: the `Player Info Update` per-player field layout (UUID immediately followed by that player's own action fields, repeated per player — the vendored doc's table is ambiguous on this point), and the `minecraft:player` entity-type ID (128, from a freshly-generated vanilla data report, cross-checked against the already-known item entity-type ID). The player entity metadata index for the "Displayed Skin Parts" bitmask (hat/cape/sleeves/etc.) is index 17, type 0/Byte — confirmed against an archived minecraft.wiki revision explicitly labeled "Java Edition 1.20.2" (this field is stable across adjacent versions). The *live* wiki page shows 16 instead, but that reflects a much newer snapshot, not 1.21 — worth remembering when checking protocol details again: check that the version being shown actually matches 1.21, don't assume the "current" page/table applies.

`Acknowledge_Finish_Config_p::deserialize` also sends a player their own `Set_Player_Skin_Parts_Metadata_p` (targeting their own entity ID) right alongside `Login_Play_p`, since `PlayerVisibilityManager` never does this for a player about themselves (a player's own entity is never "spawned" to them). This mirrors the fix that got the skin *texture* showing correctly in a player's own third-person view (sending their own `Player_Info_Update_p` entry back to them, see above) — but unlike that case, this one did **not** fix the hat layer in the player's own F5 view; other players do correctly see it via the identical mechanism. Current best explanation (unconfirmed): the client's own third-person renderer may read the skin-parts bitmask from local client options directly rather than its own entity's networked metadata, for this one field specifically — an open question, not settled.

Player movement and rotation are broadcast to whoever currently has that player visible (`PlayerVisibilityManager::broadcastMovement`, called from `Set_Player_Position_p`/`Set_Player_Position_and_Rotation_p`/`Set_Player_Rotation_p` after `Player`'s own state is updated). Which packet gets sent depends on what changed: position-only movement within an 8-block delta sends `Update_Entity_Position_p`; position+rotation together sends `Update_Entity_Position_and_Rotation_p` plus `Set_Head_Rotation_p` (head yaw always mirrors body yaw here — there's no separate client-sent head-look packet to track independently); rotation-only (looking around without moving) sends `Update_Entity_Rotation_p` plus `Set_Head_Rotation_p`; any position delta exceeding 8 blocks in any axis falls back to an absolute `Teleport_Entity_p` instead, since the delta-encoded packets can't represent a jump that large. Rotation is always sent as an absolute angle, never a delta, so there's no "old rotation" to track the way position has an old/new pair for delta math.

Sneaking and sprinting are relayed the same way: `Player_Command_p` (client presses shift/sprint) updates `Player`'s state, then `PlayerVisibilityManager::broadcastPoseChange` tells every current viewer via two separate packets — `Set_Entity_Flags_Metadata_p` (index 0, bit `0x02` crouching / `0x08` sprinting) and `Set_Player_Pose_Metadata_p` (index 6, `SNEAKING`=5/`STANDING`=0) — both fields live on the base Entity class and are stable across versions, unlike the Player-specific skin-parts field. A newly-visible player's current sneak/sprint state is also sent alongside their `Spawn_Entity_p` (mirroring the skin-parts precedent), so someone already sneaking before a viewer arrives still shows up crouched immediately rather than only on the next state change.

Three real bugs along the way, all worth remembering for any future entity-metadata work: (1) Entity Flags and Pose must be sent as two separate Set Entity Metadata packets — combining both fields into one multi-entry packet made sprinting render correctly while sneaking silently didn't, even though the byte encoding was structurally valid. (2) Pose has its own dedicated metadata type ID, not the generic VarInt type (1) — its value is wire-identical to a plain VarInt either way, so this is easy to get wrong without a crash or obvious symptom: the client just silently ignores an entry whose declared type doesn't match what it expects for that index. (3) That type ID is itself version-specific, the same trap that hit the skin-parts index earlier: it's **21** for 1.21 through 1.21.7, but only **20** starting at 1.21.9 (renamed again in later snapshots) — a live wiki page defaults to showing the newest version's numbering, which gave 20 and silently failed for this project's actual 1.21 target. Neither wrong-type attempt produced any visible error, just no pose change, which is why bugs (1) and (2) each looked like the whole fix in turn before checking version-pinned data directly.

Known gaps: no player-entity metadata beyond skin parts/pose (e.g. elytra flying, riding), and a player's own F5 view doesn't show their hat layer even though other players see it correctly (see above).

## Chat and commands

Chat deliberately skips the whole 1.19+ signed-chat system (session public keys, per-message signatures, acknowledgement bitsets). `Chat_Message_p` (0x06) still has to deserialize those fields correctly since the client always sends them, but they're consumed and discarded rather than verified — this server already establishes a player's identity through online-mode Mojang verification at login, so message-level signing doesn't add anything it needs. Outgoing, plain chat and `/say` both go out as `Disguised_Chat_Message_p` (0x1E), which is what vanilla itself uses for command-originated chat precisely because it carries no signature — not `Player Chat Message`, which would require implementing signing to avoid the client showing an "unsigned"/insecure indicator.

`Disguised_Chat_Message_p` needs a `minecraft:chat_type` registry index (`"minecraft:chat"` for player chat, `"minecraft:say_command"` for `/say`), and — same trap as `dimension_type` in `Login_Play_p` — the registry's wire order isn't fixed, since `VanillaDataManager` loads it via `std::filesystem::directory_iterator`, whose iteration order isn't guaranteed. `BroadcastDisguisedChat` looks the id up by scanning `VanillaDataManager::getEntries("chat_type")` at send time rather than assuming a fixed index, mirroring the existing dimension-type lookup.

That index is **not** sent raw, though — a real bug found via live testing (client disconnect: "Failed to decode packet 'clientbound/minecraft:disguised_chat'"). Chat Type is a `Holder<ChatType>`, not a plain registry VarInt like `dimension_type` is: the wire value is `index + 1`, with `0` reserved to mean "an inline value follows" instead of a registry reference. The vendored doc's table just says "VarInt" with no mention of this, so sending the raw 0-based index made the client read index 0 ("minecraft:chat") as `0` on the wire, interpret that as "inline value," and try to parse the following Sender Name field's NBT as a `ChatType` definition instead of a text component — hence the decode failure. Not every registry-referencing field works this way (`dimension_type` in `Login_Play_p` is a plain index, confirmed by it already working); this is a per-field thing in Mojang's own codec, not a rule that generalizes, so it was verified by checking a reference server implementation's actual call sites (which do `chat_type + 1`) rather than assumed.

Text components are sent as network NBT (`{"text": ...}` via `NbtTag`), not JSON strings — that's only for the Login-state Disconnect packet, which predates the switch to NBT-encoded chat components.

`Chat_Command_p` (0x04, the unsigned "Chat Command" packet) is the only command path handled; the signed variant (0x05) is never sent by a real client here since this server never sends a Declare Commands packet marking any argument as requiring signing. Commands typed in chat and commands typed at the server console now go through the same `Command`/`CommandRegistry` path via a `CommandSender` abstraction (`ConsoleCommandSender`, always permission level 4 and bypassing op checks like vanilla; `PlayerCommandSender`, whose permission comes from `OpsList` looked up fresh on every command rather than cached on `Player`, so a hand-edited `ops.json` or a mid-session `/deop` takes effect immediately). `CommandRegistry::dispatch` checks `Command::getRequiredPermission()` against the sender before calling `execute`, and sends the sender feedback itself (unknown command / no permission) so callers don't have to.

`OpsList` is a minimal `ops.json`-backed store keyed by dashless-hex UUID (not username, so a Mojang account rename doesn't silently deop someone) — level 0 removes the entry entirely rather than writing a zero. Built-in commands: `help`, `say` (permission 2), `op`/`deop` (permission 3, and only look among currently-connected players — there's no offline username-to-UUID cache to resolve anyone else), `list`, and `stop` (permission 4, unchanged from before this feature except it now goes through the same permission-checked path as everything else).

Known gaps: no private messaging (`/tell`/`/msg`), no chat mode/coloring support (`Client_Information_config_p` still just discards the Chat Mode field), and `/op`/`/deop` can't target an offline player by name.