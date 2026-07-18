# Documentation of McCPP

## TODO
- Fix/test packets within the Login state



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

Sky light propagating straight down through a transparent block doesn't attenuate, unlike every other direction (which decays by `max(opacity, 1)`) — this special case, along with the general propagation formula, was cross-checked against Pumpkin's `lighting/engine.rs` (see Licensing note below).

Seeding is done in two passes for performance: the first pass walks each column top-down and sets sky light values directly (15 down to the first opaque block) without touching the BFS queue at all; the second pass only *enqueues* cells that sit at an actual light/dark boundary (a horizontal neighbor not already at max light, or the buffer edge). A wide-open sky column can be a couple hundred blocks of uniform max-light air, and enqueueing every one of those cells individually made the BFS pop/check millions of neighbors that could never produce a new result.

### Chunk dispatch/delivery ordering
`UpdateLoadedChunks` (in `Play.cpp`) dispatches and delivers chunks nearest-to-center first, not in raster/`std::set` order. `Connection::sendPackets()` serializes and writes queued packets to the socket strictly in order, one at a time — so if a whole ring's worth of chunks becomes ready at once (e.g. all cache hits on reconnecting to an already-explored area), the chunk directly under the player's own feet could end up serialized behind ~200 others. The client's "waiting for chunks" gate (which holds off gravity until its own chunk arrives) would then release too late, and the player free-falls into the ground before their own chunk shows up. This was found from a real "stuck underground on reconnect" bug report and confirmed via the client's F3 debug screen, not assumed from reading the code.

### Performance history
Two performance passes, both 2026-07-18:

1. **Algorithmic fixes** — the terrain/lit cache split above; memoizing `NoiseChunkGenerator`'s per-column `isSolid()` results instead of recomputing them up to `DIRT_DEPTH+1` times per solid block for surface layering; and the two-pass sky-light seeding above. Combined: ~58s -> ~21s for a full 441-chunk view-distance batch (measured with an isolated timing harness linked against the real `.o` files).
2. **Build flags** — turned out to be a bigger factor than any single algorithmic fix. The Makefile's default build had no `-O` flag at all (effectively `-O0`). Adding `-O2` to the default build alone (no code changes) took `NoiseChunkGenerator::generate()` from 50.8ms to 11.8ms and `LightEngine::computeLighting()` (including its 8 neighbor terrain gens) from 362.6ms to 105.1ms — about 3.5x. `debug` now explicitly forces `-O0` back on top so gdb stepping stays reliable; the old separate `make fast`/`-Ofast` target was removed since `-O2` is just the default now, not an opt-in.

## Licensing note on reference server cross-checking
[Pumpkin](https://github.com/Pumpkin-MC/Pumpkin) (Rust, GPL-3.0 — confirmed by reading its `LICENSE` file directly, not assumed) is used as a cross-reference for wire-format details and algorithm behavior not fully pinned down by decompiling the client or by `network-protocol.md` (e.g. the palette container's Data Array Length field, the light propagation formula). MC++ is MIT-licensed, so this only stays clean as long as what's taken is *facts* — wire-format layout, propagation rules, field ordering, mostly describing vanilla Minecraft's own behavior rather than anything original to Pumpkin — reimplemented as genuinely original code (different data structures, different structure/control flow), never copied or mechanically translated line-for-line. Copyright protects expression, not the underlying facts/algorithms.

## Wire format gotchas found the hard way
A couple of `Chunk_Data_p` details that don't match a naive reading of the wiki summary, both confirmed against decompiled 1.21 client bytecode:
- A single-valued (Bits Per Entry = 0) paletted container is still followed by a Data Array Length VarInt (always 0 in that case) — it's present but zero-length, not omitted entirely.
- Each chunk section has exactly one non-empty-block-count short on the wire (blocks and fluids combined), not separate block/fluid counts.