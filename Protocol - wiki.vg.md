This page presents a dissection of version 1.21 of the **[Minecraft](https://minecraft.net/) protocol**.

If you're having trouble, check out the [FAQ](https://wiki.vg/Protocol_FAQ "Protocol FAQ") or ask for help in the IRC channel [#mcdevs on irc.libera.chat](ircs://irc.libera.chat:6697) ([More Information](https://wiki.vg/MCDevs)).

**Note:** While you may use the contents of this page without restriction to create servers, clients, bots, etc; substantial reproductions of this page must be attributed IAW [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/).

The changes between versions may be viewed at [Protocol History](https://wiki.vg/Protocol_History "Protocol History").

## Contents

-   [1 Definitions](#definitions)
    -   [1.1 Data types](#data-types)
    -   [1.2 Identifier](#identifier)
    -   [1.3 VarInt and VarLong](#varint-and-varlong)
    -   [1.4 Position](#position)
    -   [1.5 Fixed-point numbers](#fixed-point-numbers)
    -   [1.6 Bit sets](#bit-sets)
        -   [1.6.1 BitSet](#bitset)
        -   [1.6.2 Fixed BitSet](#fixed-bitset)
    -   [1.7 Other definitions](#other-definitions)
-   [2 Packet format](#packet-format)
    -   [2.1 Without compression](#without-compression)
    -   [2.2 With compression](#with-compression)
-   [3 Handshaking](#handshaking)
    -   [3.1 Clientbound](#clientbound)
    -   [3.2 Serverbound](#serverbound)
        -   [3.2.1 Handshake](#handshake)
        -   [3.2.2 Legacy Server List Ping](#legacy-server-list-ping)
-   [4 Status](#status)
    -   [4.1 Clientbound](#clientbound-2)
        -   [4.1.1 Status Response](#status-response)
        -   [4.1.2 Ping Response (status)](#ping-response-status)
    -   [4.2 Serverbound](#serverbound-2)
        -   [4.2.1 Status Request](#status-request)
        -   [4.2.2 Ping Request (status)](#ping-request-status)
-   [5 Login](#login)
    -   [5.1 Clientbound](#clientbound-3)
        -   [5.1.1 Disconnect (login)](#disconnect-login)
        -   [5.1.2 Encryption Request](#encryption-request)
        -   [5.1.3 Login Success](#login-success)
        -   [5.1.4 Set Compression](#set-compression)
        -   [5.1.5 Login Plugin Request](#login-plugin-request)
        -   [5.1.6 Cookie Request (login)](#cookie-request-login)
    -   [5.2 Serverbound](#serverbound-3)
        -   [5.2.1 Login Start](#login-start)
        -   [5.2.2 Encryption Response](#encryption-response)
        -   [5.2.3 Login Plugin Response](#login-plugin-response)
        -   [5.2.4 Login Acknowledged](#login-acknowledged)
        -   [5.2.5 Cookie Response (login)](#cookie-response-login)
-   [6 Configuration](#configuration)
    -   [6.1 Clientbound](#clientbound-4)
        -   [6.1.1 Cookie Request (configuration)](#cookie-request-configuration)
        -   [6.1.2 Clientbound Plugin Message (configuration)](#clientbound-plugin-message-configuration)
        -   [6.1.3 Disconnect (configuration)](#disconnect-configuration)
        -   [6.1.4 Finish Configuration](#finish-configuration)
        -   [6.1.5 Clientbound Keep Alive (configuration)](#clientbound-keep-alive-configuration)
        -   [6.1.6 Ping (configuration)](#ping-configuration)
        -   [6.1.7 Reset Chat](#reset-chat)
        -   [6.1.8 Registry Data](#registry-data)
        -   [6.1.9 Remove Resource Pack (configuration)](#remove-resource-pack-configuration)
        -   [6.1.10 Add Resource Pack (configuration)](#add-resource-pack-configuration)
        -   [6.1.11 Store Cookie (configuration)](#store-cookie-configuration)
        -   [6.1.12 Transfer (configuration)](#transfer-configuration)
        -   [6.1.13 Feature Flags](#feature-flags)
        -   [6.1.14 Update Tags (configuration)](#update-tags-configuration)
        -   [6.1.15 Clientbound Known Packs](#clientbound-known-packs)
        -   [6.1.16 Custom Report Details (configuration)](#custom-report-details-configuration)
        -   [6.1.17 Server Links (configuration)](#server-links-configuration)
    -   [6.2 Serverbound](#serverbound-4)
        -   [6.2.1 Client Information (configuration)](#client-information-configuration)
        -   [6.2.2 Cookie Response (configuration)](#cookie-response-configuration)
        -   [6.2.3 Serverbound Plugin Message (configuration)](#serverbound-plugin-message-configuration)
        -   [6.2.4 Acknowledge Finish Configuration](#acknowledge-finish-configuration)
        -   [6.2.5 Serverbound Keep Alive (configuration)](#serverbound-keep-alive-configuration)
        -   [6.2.6 Pong (configuration)](#pong-configuration)
        -   [6.2.7 Resource Pack Response (configuration)](#resource-pack-response-configuration)
        -   [6.2.8 Serverbound Known Packs](#serverbound-known-packs)
-   [7 Play](#play)
    -   [7.1 Clientbound](#clientbound-5)
        -   [7.1.1 Bundle Delimiter](#bundle-delimiter)
        -   [7.1.2 Spawn Entity](#spawn-entity)
        -   [7.1.3 Spawn Experience Orb](#spawn-experience-orb)
        -   [7.1.4 Entity Animation](#entity-animation)
        -   [7.1.5 Award Statistics](#award-statistics)
        -   [7.1.6 Acknowledge Block Change](#acknowledge-block-change)
        -   [7.1.7 Set Block Destroy Stage](#set-block-destroy-stage)
        -   [7.1.8 Block Entity Data](#block-entity-data)
        -   [7.1.9 Block Action](#block-action)
        -   [7.1.10 Block Update](#block-update)
        -   [7.1.11 Boss Bar](#boss-bar)
        -   [7.1.12 Change Difficulty](#change-difficulty)
        -   [7.1.13 Chunk Batch Finished](#chunk-batch-finished)
        -   [7.1.14 Chunk Batch Start](#chunk-batch-start)
        -   [7.1.15 Chunk Biomes](#chunk-biomes)
        -   [7.1.16 Clear Titles](#clear-titles)
        -   [7.1.17 Command Suggestions Response](#command-suggestions-response)
        -   [7.1.18 Commands](#commands)
        -   [7.1.19 Close Container](#close-container)
        -   [7.1.20 Set Container Content](#set-container-content)
        -   [7.1.21 Set Container Property](#set-container-property)
        -   [7.1.22 Set Container Slot](#set-container-slot)
        -   [7.1.23 Cookie Request (play)](#cookie-request-play)
        -   [7.1.24 Set Cooldown](#set-cooldown)
        -   [7.1.25 Chat Suggestions](#chat-suggestions)
        -   [7.1.26 Clientbound Plugin Message (play)](#clientbound-plugin-message-play)
        -   [7.1.27 Damage Event](#damage-event)
        -   [7.1.28 Debug Sample](#debug-sample)
        -   [7.1.29 Delete Message](#delete-message)
        -   [7.1.30 Disconnect (play)](#disconnect-play)
        -   [7.1.31 Disguised Chat Message](#disguised-chat-message)
        -   [7.1.32 Entity Event](#entity-event)
        -   [7.1.33 Explosion](#explosion)
        -   [7.1.34 Unload Chunk](#unload-chunk)
        -   [7.1.35 Game Event](#game-event)
        -   [7.1.36 Open Horse Screen](#open-horse-screen)
        -   [7.1.37 Hurt Animation](#hurt-animation)
        -   [7.1.38 Initialize World Border](#initialize-world-border)
        -   [7.1.39 Clientbound Keep Alive (play)](#clientbound-keep-alive-play)
        -   [7.1.40 Chunk Data and Update Light](#chunk-data-and-update-light)
        -   [7.1.41 World Event](#world-event)
        -   [7.1.42 Particle](#particle)
        -   [7.1.43 Update Light](#update-light)
        -   [7.1.44 Login (play)](#login-play)
        -   [7.1.45 Map Data](#map-data)
        -   [7.1.46 Merchant Offers](#merchant-offers)
        -   [7.1.47 Update Entity Position](#update-entity-position)
        -   [7.1.48 Update Entity Position and Rotation](#update-entity-position-and-rotation)
        -   [7.1.49 Update Entity Rotation](#update-entity-rotation)
        -   [7.1.50 Move Vehicle](#move-vehicle)
        -   [7.1.51 Open Book](#open-book)
        -   [7.1.52 Open Screen](#open-screen)
        -   [7.1.53 Open Sign Editor](#open-sign-editor)
        -   [7.1.54 Ping (play)](#ping-play)
        -   [7.1.55 Ping Response (play)](#ping-response-play)
        -   [7.1.56 Place Ghost Recipe](#place-ghost-recipe)
        -   [7.1.57 Player Abilities (clientbound)](#player-abilities-clientbound)
        -   [7.1.58 Player Chat Message](#player-chat-message)
        -   [7.1.59 End Combat](#end-combat)
        -   [7.1.60 Enter Combat](#enter-combat)
        -   [7.1.61 Combat Death](#combat-death)
        -   [7.1.62 Player Info Remove](#player-info-remove)
        -   [7.1.63 Player Info Update](#player-info-update)
        -   [7.1.64 Look At](#look-at)
        -   [7.1.65 Synchronize Player Position](#synchronize-player-position)
        -   [7.1.66 Update Recipe Book](#update-recipe-book)
        -   [7.1.67 Remove Entities](#remove-entities)
        -   [7.1.68 Remove Entity Effect](#remove-entity-effect)
        -   [7.1.69 Reset Score](#reset-score)
        -   [7.1.70 Remove Resource Pack (play)](#remove-resource-pack-play)
        -   [7.1.71 Add Resource Pack (play)](#add-resource-pack-play)
        -   [7.1.72 Respawn](#respawn)
        -   [7.1.73 Set Head Rotation](#set-head-rotation)
        -   [7.1.74 Update Section Blocks](#update-section-blocks)
        -   [7.1.75 Select Advancements Tab](#select-advancements-tab)
        -   [7.1.76 Server Data](#server-data)
        -   [7.1.77 Set Action Bar Text](#set-action-bar-text)
        -   [7.1.78 Set Border Center](#set-border-center)
        -   [7.1.79 Set Border Lerp Size](#set-border-lerp-size)
        -   [7.1.80 Set Border Size](#set-border-size)
        -   [7.1.81 Set Border Warning Delay](#set-border-warning-delay)
        -   [7.1.82 Set Border Warning Distance](#set-border-warning-distance)
        -   [7.1.83 Set Camera](#set-camera)
        -   [7.1.84 Set Held Item (clientbound)](#set-held-item-clientbound)
        -   [7.1.85 Set Center Chunk](#set-center-chunk)
        -   [7.1.86 Set Render Distance](#set-render-distance)
        -   [7.1.87 Set Default Spawn Position](#set-default-spawn-position)
        -   [7.1.88 Display Objective](#display-objective)
        -   [7.1.89 Set Entity Metadata](#set-entity-metadata)
        -   [7.1.90 Link Entities](#link-entities)
        -   [7.1.91 Set Entity Velocity](#set-entity-velocity)
        -   [7.1.92 Set Equipment](#set-equipment)
        -   [7.1.93 Set Experience](#set-experience)
        -   [7.1.94 Set Health](#set-health)
        -   [7.1.95 Update Objectives](#update-objectives)
        -   [7.1.96 Set Passengers](#set-passengers)
        -   [7.1.97 Update Teams](#update-teams)
        -   [7.1.98 Update Score](#update-score)
        -   [7.1.99 Set Simulation Distance](#set-simulation-distance)
        -   [7.1.100 Set Subtitle Text](#set-subtitle-text)
        -   [7.1.101 Update Time](#update-time)
        -   [7.1.102 Set Title Text](#set-title-text)
        -   [7.1.103 Set Title Animation Times](#set-title-animation-times)
        -   [7.1.104 Entity Sound Effect](#entity-sound-effect)
        -   [7.1.105 Sound Effect](#sound-effect)
        -   [7.1.106 Start Configuration](#start-configuration)
        -   [7.1.107 Stop Sound](#stop-sound)
        -   [7.1.108 Store Cookie (play)](#store-cookie-play)
        -   [7.1.109 System Chat Message](#system-chat-message)
        -   [7.1.110 Set Tab List Header And Footer](#set-tab-list-header-and-footer)
        -   [7.1.111 Tag Query Response](#tag-query-response)
        -   [7.1.112 Pickup Item](#pickup-item)
        -   [7.1.113 Teleport Entity](#teleport-entity)
        -   [7.1.114 Set Ticking State](#set-ticking-state)
        -   [7.1.115 Step Tick](#step-tick)
        -   [7.1.116 Transfer (play)](#transfer-play)
        -   [7.1.117 Update Advancements](#update-advancements)
        -   [7.1.118 Update Attributes](#update-attributes)
        -   [7.1.119 Entity Effect](#entity-effect)
        -   [7.1.120 Update Recipes](#update-recipes)
        -   [7.1.121 Update Tags (play)](#update-tags-play)
        -   [7.1.122 Projectile Power](#projectile-power)
        -   [7.1.123 Custom Report Details](#custom-report-details)
        -   [7.1.124 Server Links](#server-links)
    -   [7.2 Serverbound](#serverbound-5)
        -   [7.2.1 Confirm Teleportation](#confirm-teleportation)
        -   [7.2.2 Query Block Entity Tag](#query-block-entity-tag)
        -   [7.2.3 Change Difficulty](#change-difficulty-2)
        -   [7.2.4 Acknowledge Message](#acknowledge-message)
        -   [7.2.5 Chat Command](#chat-command)
        -   [7.2.6 Signed Chat Command](#signed-chat-command)
        -   [7.2.7 Chat Message](#chat-message)
        -   [7.2.8 Player Session](#player-session)
        -   [7.2.9 Chunk Batch Received](#chunk-batch-received)
        -   [7.2.10 Client Status](#client-status)
        -   [7.2.11 Client Information (play)](#client-information-play)
        -   [7.2.12 Command Suggestions Request](#command-suggestions-request)
        -   [7.2.13 Acknowledge Configuration](#acknowledge-configuration)
        -   [7.2.14 Click Container Button](#click-container-button)
        -   [7.2.15 Click Container](#click-container)
        -   [7.2.16 Close Container](#close-container-2)
        -   [7.2.17 Change Container Slot State](#change-container-slot-state)
        -   [7.2.18 Cookie Response (play)](#cookie-response-play)
        -   [7.2.19 Serverbound Plugin Message (play)](#serverbound-plugin-message-play)
        -   [7.2.20 Debug Sample Subscription](#debug-sample-subscription)
        -   [7.2.21 Edit Book](#edit-book)
        -   [7.2.22 Query Entity Tag](#query-entity-tag)
        -   [7.2.23 Interact](#interact)
        -   [7.2.24 Jigsaw Generate](#jigsaw-generate)
        -   [7.2.25 Serverbound Keep Alive (play)](#serverbound-keep-alive-play)
        -   [7.2.26 Lock Difficulty](#lock-difficulty)
        -   [7.2.27 Set Player Position](#set-player-position)
        -   [7.2.28 Set Player Position and Rotation](#set-player-position-and-rotation)
        -   [7.2.29 Set Player Rotation](#set-player-rotation)
        -   [7.2.30 Set Player On Ground](#set-player-on-ground)
        -   [7.2.31 Move Vehicle](#move-vehicle-2)
        -   [7.2.32 Paddle Boat](#paddle-boat)
        -   [7.2.33 Pick Item](#pick-item)
        -   [7.2.34 Ping Request (play)](#ping-request-play)
        -   [7.2.35 Place Recipe](#place-recipe)
        -   [7.2.36 Player Abilities (serverbound)](#player-abilities-serverbound)
        -   [7.2.37 Player Action](#player-action)
        -   [7.2.38 Player Command](#player-command)
        -   [7.2.39 Player Input](#player-input)
        -   [7.2.40 Pong (play)](#pong-play)
        -   [7.2.41 Change Recipe Book Settings](#change-recipe-book-settings)
        -   [7.2.42 Set Seen Recipe](#set-seen-recipe)
        -   [7.2.43 Rename Item](#rename-item)
        -   [7.2.44 Resource Pack Response (play)](#resource-pack-response-play)
        -   [7.2.45 Seen Advancements](#seen-advancements)
        -   [7.2.46 Select Trade](#select-trade)
        -   [7.2.47 Set Beacon Effect](#set-beacon-effect)
        -   [7.2.48 Set Held Item (serverbound)](#set-held-item-serverbound)
        -   [7.2.49 Program Command Block](#program-command-block)
        -   [7.2.50 Program Command Block Minecart](#program-command-block-minecart)
        -   [7.2.51 Set Creative Mode Slot](#set-creative-mode-slot)
        -   [7.2.52 Program Jigsaw Block](#program-jigsaw-block)
        -   [7.2.53 Program Structure Block](#program-structure-block)
        -   [7.2.54 Update Sign](#update-sign)
        -   [7.2.55 Swing Arm](#swing-arm)
        -   [7.2.56 Teleport To Entity](#teleport-to-entity)
        -   [7.2.57 Use Item On](#use-item-on)
        -   [7.2.58 Use Item](#use-item)
        
## Definitions

The Minecraft server accepts connections from TCP clients and communicates with them using _packets_. A packet is a sequence of bytes sent over the TCP connection. The meaning of a packet depends both on its packet ID and the current state of the connection. The initial state of each connection is [Handshaking](https://wiki.vg/Protocol#Handshaking), and state is switched using the packets [Handshake](https://wiki.vg/Protocol#Handshake) and [Login Success](https://wiki.vg/Protocol#Login_Success).

### Data types

All data sent over the network (except for VarInt and VarLong) is [big-endian](http://en.wikipedia.org/wiki/Endianness#Big-endian "wikipedia:Endianness"), that is the bytes are sent from most significant byte to least significant byte. The majority of everyday computers are little-endian, therefore it may be necessary to change the endianness before sending data over the network.

| Name | Size (bytes) | Encodes | Notes |
| --- | --- | --- | --- |
| [Boolean](https://wiki.vg/Protocol#Type:Boolean) | 1 | Either false or true | True is encoded as `0x01`, false as `0x00`. |
| [Byte](https://wiki.vg/Protocol#Type:Byte) | 1 | An integer between -128 and 127 | Signed 8-bit integer, [two's complement](http://en.wikipedia.org/wiki/Two%27s_complement "wikipedia:Two's complement") |
| [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | 1 | An integer between 0 and 255 | Unsigned 8-bit integer |
| [Short](https://wiki.vg/Protocol#Type:Short) | 2 | An integer between -32768 and 32767 | Signed 16-bit integer, two's complement |
| [Unsigned Short](https://wiki.vg/Protocol#Type:Unsigned_Short) | 2 | An integer between 0 and 65535 | Unsigned 16-bit integer |
| [Int](https://wiki.vg/Protocol#Type:Int) | 4 | An integer between -2147483648 and 2147483647 | Signed 32-bit integer, two's complement |
| [Long](https://wiki.vg/Protocol#Type:Long) | 8 | An integer between -9223372036854775808 and 9223372036854775807 | Signed 64-bit integer, two's complement |
| [Float](https://wiki.vg/Protocol#Type:Float) | 4 | A [single-precision 32-bit IEEE 754 floating point number](http://en.wikipedia.org/wiki/Single-precision_floating-point_format "wikipedia:Single-precision floating-point format") |  |
| [Double](https://wiki.vg/Protocol#Type:Double) | 8 | A [double-precision 64-bit IEEE 754 floating point number](http://en.wikipedia.org/wiki/Double-precision_floating-point_format "wikipedia:Double-precision floating-point format") |  |
| [String](https://wiki.vg/Protocol#Type:String) (n) | ≥ 1  
≤ (n×3) + 3 | A sequence of [Unicode](http://en.wikipedia.org/wiki/Unicode "wikipedia:Unicode") [scalar values](http://unicode.org/glossary/#unicode_scalar_value) | [UTF-8](http://en.wikipedia.org/wiki/UTF-8 "wikipedia:UTF-8") string prefixed with its size in bytes as a VarInt. Maximum length of `n` characters, which varies by context. The encoding used on the wire is regular UTF-8, _not_ [Java's "slight modification"](https://docs.oracle.com/en/java/javase/18/docs/api/java.base/java/io/DataInput.html#modified-utf-8). However, the length of the string for purposes of the length limit is its number of [UTF-16](http://en.wikipedia.org/wiki/UTF-16 "wikipedia:UTF-16") code units, that is, scalar values > U+FFFF are counted as two. Up to `n × 3` bytes can be used to encode a UTF-8 string comprising `n` code units when converted to UTF-16, and both of those limits are checked. Maximum `n` value is 32767. The + 3 is due to the max size of a valid length VarInt. |
| [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Varies | See [Text formatting#Text components](https://wiki.vg/Text_formatting#Text_components "Text formatting") | Encoded as a [NBT Tag](https://wiki.vg/NBT "NBT"), with the type of tag used depending on the case:
-   As a [String Tag](https://wiki.vg/NBT#Specification:string_tag "NBT"): For components only containing text (no styling, no events etc.).
-   As a [Compound Tag](https://wiki.vg/NBT#Specification:compound_tag "NBT"): Every other case.

 |
| [JSON Text Component](https://wiki.vg/Protocol#Type:JSON_Text_Component) | ≥ 1  
≤ (262144×3) + 3 | See [Text formatting#Text components](https://wiki.vg/Text_formatting#Text_components "Text formatting") | The maximum permitted length when decoding is 262144, but the Notchian server since 1.20.3 refuses to encode longer than 32767. This may be a bug. |
| [Identifier](https://wiki.vg/Protocol#Type:Identifier) | ≥ 1  
≤ (32767×3) + 3 | See [Identifier](https://wiki.vg/Protocol#Identifier) below | Encoded as a String with max length of 32767. |
| [VarInt](https://wiki.vg/Protocol#Type:VarInt) | ≥ 1  
≤ 5 | An integer between -2147483648 and 2147483647 | Variable-length data encoding a two's complement signed 32-bit integer; more info in [their section](https://wiki.vg/Protocol#VarInt_and_VarLong) |
| [VarLong](https://wiki.vg/Protocol#Type:VarLong) | ≥ 1  
≤ 10 | An integer between -9223372036854775808 and 9223372036854775807 | Variable-length data encoding a two's complement signed 64-bit integer; more info in [their section](https://wiki.vg/Protocol#VarInt_and_VarLong) |
| [Entity Metadata](https://wiki.vg/Protocol#Type:Entity_Metadata) | Varies | Miscellaneous information about an entity | See [Entity\_metadata#Entity Metadata Format](https://wiki.vg/Entity_metadata#Entity_Metadata_Format "Entity metadata") |
| [Slot](https://wiki.vg/Protocol#Type:Slot) | Varies | An item stack in an inventory or container | See [Slot Data](https://wiki.vg/Slot_Data "Slot Data") |
| [NBT](https://wiki.vg/Protocol#Type:NBT) | Varies | Depends on context | See [NBT](https://wiki.vg/NBT "NBT") |
| [Position](https://wiki.vg/Protocol#Type:Position) | 8 | An integer/block position: x (-33554432 to 33554431), z (-33554432 to 33554431), y (-2048 to 2047) | x as a 26-bit integer, followed by z as a 26-bit integer, followed by y as a 12-bit integer (all signed, two's complement). See also [the section below](https://wiki.vg/Protocol#Position). |
| [Angle](https://wiki.vg/Protocol#Type:Angle) | 1 | A rotation angle in steps of 1/256 of a full turn | Whether or not this is signed does not matter, since the resulting angles are the same. |
| [UUID](https://wiki.vg/Protocol#Type:UUID) | 16 | A [UUID](http://en.wikipedia.org/wiki/Universally_unique_identifier "wikipedia:Universally unique identifier") | Encoded as an unsigned 128-bit integer (or two unsigned 64-bit integers: the most significant 64 bits and then the least significant 64 bits) |
| [BitSet](https://wiki.vg/Protocol#Type:BitSet) | Varies | See [#BitSet](https://wiki.vg/Protocol#BitSet) below | A length-prefixed bit set. |
| [Fixed BitSet](https://wiki.vg/Protocol#Type:Fixed_BitSet) (n) | ceil(n / 8) | See [#Fixed BitSet](https://wiki.vg/Protocol#Fixed_BitSet) below | A bit set with a fixed length of n bits. |
| [Optional](https://wiki.vg/Protocol#Type:Optional) X | 0 or size of X | A field of type X, or nothing | Whether or not the field is present must be known from the context. |
| [Array](https://wiki.vg/Protocol#Type:Array) of X | count times size of X | Zero or more fields of type X | The count must be known from the context. |
| X [Enum](https://wiki.vg/Protocol#Type:Enum) | size of X | A specific value from a given list | The list of possible values and how each is encoded as an X must be known from the context. An invalid value sent by either side will usually result in the client being disconnected with an error or even crashing. |
| [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | Varies | Depends on context | This is just a sequence of zero or more bytes, its meaning should be explained somewhere else, e.g. in the packet description. The length must also be known from the context. |

### Identifier

Identifiers are a namespaced location, in the form of `minecraft:thing`. If the namespace is not provided, it defaults to `minecraft` (i.e. `thing` is `minecraft:thing`). Custom content should always be in its own namespace, not the default one. Both the namespace and value can use all lowercase alphanumeric characters (a-z and 0-9), dot (`.`), dash (`-`), and underscore (`_`). In addition, values can use slash (`/`). The naming convention is `lower_case_with_underscores`. [More information](https://minecraft.net/en-us/article/minecraft-snapshot-17w43a). For ease of determining whether a namespace or value is valid, here are regular expressions for each:

-   Namespace: `[a-z0-9.-_]`
-   Value: `[a-z0-9.-_/]`

### VarInt and VarLong

Variable-length format such that smaller numbers use fewer bytes. These are very similar to [Protocol Buffer Varints](http://developers.google.com/protocol-buffers/docs/encoding#varints): the 7 least significant bits are used to encode the value and the most significant bit indicates whether there's another byte after it for the next part of the number. The least significant group is written first, followed by each of the more significant groups; thus, VarInts are effectively little endian (however, groups are 7 bits, not 8).

VarInts are never longer than 5 bytes, and VarLongs are never longer than 10 bytes. Within these limits, unnecessarily long encodings (e.g. `81 00` to encode 1) are allowed.

Pseudocode to read and write VarInts and VarLongs:

```C++
private static final int SEGMENT_BITS = 0x7F;
private static final int CONTINUE_BIT = 0x80;
```

```C++
public int readVarInt() {
    int value = 0;
    int position = 0;
    byte currentByte;

    while (true) {
        currentByte = readByte();
        value |= (currentByte & SEGMENT_BITS) << position;

        if ((currentByte & CONTINUE_BIT) == 0) break;

        position += 7;

        if (position >= 32) throw new RuntimeException("VarInt is too big");
    }

    return value;
}
```

```C++
public long readVarLong() {
    long value = 0;
    int position = 0;
    byte currentByte;

    while (true) {
        currentByte = readByte();
        value |= (long) (currentByte & SEGMENT_BITS) << position;

        if ((currentByte & CONTINUE_BIT) == 0) break;

        position += 7;

        if (position >= 64) throw new RuntimeException("VarLong is too big");
    }

    return value;
}
```

```C++
public void writeVarInt(int value) {
    while(true) {
        if ((value & ~SEGMENT_BITS) == 0) {
            writeByte(value);
            return;
        }
        writeByte((value & SEGMENT_BITS) | CONTINUE_BIT);

        // Note: means that the sign bit is shifted with the rest of the number rather than being left alone
        value >>>= 7;
    }
}
```

```
<span></span><span>public</span> <span>void</span> <span>writeVarLong</span><span>(</span><span>long</span> <span>value</span><span>)</span> <span>{</span>
    <span>while</span> <span>(</span><span>true</span><span>)</span> <span>{</span>
        <span>if</span> <span>((</span><span>value</span> <span>&amp;</span> <span>~((</span><span>long</span><span>)</span> <span>SEGMENT_BITS</span><span>))</span> <span>==</span> <span>0</span><span>)</span> <span>{</span>
            <span>writeByte</span><span>(</span><span>value</span><span>);</span>
            <span>return</span><span>;</span>
        <span>}</span>

        <span>writeByte</span><span>((</span><span>value</span> <span>&amp;</span> <span>SEGMENT_BITS</span><span>)</span> <span>|</span> <span>CONTINUE_BIT</span><span>);</span>

        <span>// Note: &gt;&gt;&gt; means that the sign bit is shifted with the rest of the number rather than being left alone</span>
        <span>value</span> <span>&gt;&gt;&gt;=</span> <span>7</span><span>;</span>
    <span>}</span>
<span>}</span>
```

[![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) Note Minecraft's VarInts are identical to [LEB128](https://en.wikipedia.org/wiki/LEB128) with the slight change of throwing a exception if it goes over a set amount of bytes.

[![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) Note that Minecraft's VarInts are not encoded using Protocol Buffers; it's just similar. If you try to use Protocol Buffers Varints with Minecraft's VarInts, you'll get incorrect results in some cases. The major differences:

-   Minecraft's VarInts are all signed, but do not use the ZigZag encoding. Protocol buffers have 3 types of Varints: `uint32` (normal encoding, unsigned), `sint32` (ZigZag encoding, signed), and `int32` (normal encoding, signed). Minecraft's are the `int32` variety. Because Minecraft uses the normal encoding instead of ZigZag encoding, negative values always use the maximum number of bytes.
-   Minecraft's VarInts are never longer than 5 bytes and its VarLongs will never be longer than 10 bytes, while Protocol Buffer Varints will always use 10 bytes when encoding negative numbers, even if it's an `int32`.

Sample VarInts:

| Value | Hex bytes | Decimal bytes |
| --- | --- | --- |
| 0 | 0x00 | 0 |
| 1 | 0x01 | 1 |
| 2 | 0x02 | 2 |
| 127 | 0x7f | 127 |
| 128 | 0x80 0x01 | 128 1 |
| 255 | 0xff 0x01 | 255 1 |
| 25565 | 0xdd 0xc7 0x01 | 221 199 1 |
| 2097151 | 0xff 0xff 0x7f | 255 255 127 |
| 2147483647 | 0xff 0xff 0xff 0xff 0x07 | 255 255 255 255 7 |
| \-1 | 0xff 0xff 0xff 0xff 0x0f | 255 255 255 255 15 |
| \-2147483648 | 0x80 0x80 0x80 0x80 0x08 | 128 128 128 128 8 |

Sample VarLongs:

| Value | Hex bytes | Decimal bytes |
| --- | --- | --- |
| 0 | 0x00 | 0 |
| 1 | 0x01 | 1 |
| 2 | 0x02 | 2 |
| 127 | 0x7f | 127 |
| 128 | 0x80 0x01 | 128 1 |
| 255 | 0xff 0x01 | 255 1 |
| 2147483647 | 0xff 0xff 0xff 0xff 0x07 | 255 255 255 255 7 |
| 9223372036854775807 | 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0x7f | 255 255 255 255 255 255 255 255 127 |
| \-1 | 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0xff 0x01 | 255 255 255 255 255 255 255 255 255 1 |
| \-2147483648 | 0x80 0x80 0x80 0x80 0xf8 0xff 0xff 0xff 0xff 0x01 | 128 128 128 128 248 255 255 255 255 1 |
| \-9223372036854775808 | 0x80 0x80 0x80 0x80 0x80 0x80 0x80 0x80 0x80 0x01 | 128 128 128 128 128 128 128 128 128 1 |

### Position

**Note:** What you are seeing here is the latest version of the [Data types](https://wiki.vg/Data_types "Data types") article, but the position type was [different before 1.14](https://wiki.vg/index.php?title=Data_types&oldid=14345#Position).

64-bit value split into three **signed** integer parts:

-   x: 26 MSBs
-   z: 26 middle bits
-   y: 12 LSBs

For example, a 64-bit position can be broken down as follows:

Example value (big endian): `01000110000001110110001100 10110000010101101101001000 001100111111`  

-   The red value is the X coordinate, which is `18357644` in this example.  
    
-   The blue value is the Z coordinate, which is `-20882616` in this example.  
    
-   The green value is the Y coordinate, which is `831` in this example.  
    

Encoded as follows:

```
((x &amp; 0x3FFFFFF) &lt;&lt; 38) | ((z &amp; 0x3FFFFFF) &lt;&lt; 12) | (y &amp; 0xFFF)
```

And decoded as:

```
val = read_long();
x = val &gt;&gt; 38;
y = val &lt;&lt; 52 &gt;&gt; 52;
z = val &lt;&lt; 26 &gt;&gt; 38;
```

Note: The above assumes that the right shift operator sign extends the value (this is called an [arithmetic shift](https://en.wikipedia.org/wiki/Arithmetic_shift)), so that the signedness of the coordinates is preserved. In many languages, this requires the integer type of `val` to be signed. In the absence of such an operator, the following may be useful:

```
if x &gt;= 1 &lt;&lt; 25 { x -= 1 &lt;&lt; 26 }
if y &gt;= 1 &lt;&lt; 11 { y -= 1 &lt;&lt; 12 }
if z &gt;= 1 &lt;&lt; 25 { z -= 1 &lt;&lt; 26 }
```

### Fixed-point numbers

Some fields may be stored as [fixed-point numbers](https://en.wikipedia.org/wiki/Fixed-point_arithmetic), where a certain number of bits represent the signed integer part (number to the left of the decimal point) and the rest represent the fractional part (to the right). Floating point numbers (float and double), in contrast, keep the number itself (mantissa) in one chunk, while the location of the decimal point (exponent) is stored beside it. Essentially, while fixed-point numbers have lower range than floating point numbers, their fractional precision is greater for higher values.

Prior to version 1.9 a fixed-point format with 5 fraction bits and 27 integer bits was used to send entity positions to the client. Some uses of fixed point remain in modern versions, but they differ from that format.

Most programming languages lack support for fractional integers directly, but you can represent them as integers. The following C or Java-like pseudocode converts a double to a fixed-point integer with n fraction bits:

```
 x_fixed = (int)(x_double * (1 &lt;&lt; n));
```

And back again:

```
 x_double = (double)x_fixed / (1 &lt;&lt; n);
```

### Bit sets

The types [BitSet](https://wiki.vg/Protocol#Type:BitSet) and [Fixed BitSet](https://wiki.vg/Protocol#Type:Fixed_BitSet) represent packed lists of bits. The Notchian implementation uses Java's [`BitSet`](https://docs.oracle.com/javase/8/docs/api/java/util/BitSet.html) class.

#### BitSet

Bit sets of type BitSet are prefixed by their length in longs.

| Field Name | Field Type | Meaning |
| --- | --- | --- |
| Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of longs in the following array. May be 0 (if no bits are set). |
| Data | [Array](https://wiki.vg/Protocol#Type:Array) of [Long](https://wiki.vg/Protocol#Type:Long) | A packed representation of the bit set as created by [`BitSet.toLongArray`](https://docs.oracle.com/javase/8/docs/api/java/util/BitSet.html#toLongArray--). |

The ith bit is set when `(Data[i / 64] & (1 << (i % 64))) != 0`, where i starts at 0.

#### Fixed BitSet

Bit sets of type Fixed BitSet (n) have a fixed length of n bits, encoded as `ceil(n / 8)` bytes. Note that this is different from BitSet, which uses longs.

| Field Name | Field Type | Meaning |
| --- | --- | --- |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (n) | A packed representation of the bit set as created by [`BitSet.toByteArray`](https://docs.oracle.com/javase/8/docs/api/java/util/BitSet.html#toByteArray--), padded with zeroes at the end to fit the specified length. |

The ith bit is set when `(Data[i / 8] & (1 << (i % 8))) != 0`, where i starts at 0. This encoding is _not_ equivalent to the long array in BitSet.

### Other definitions

| Term | Definition |
| --- | --- |
| Player | When used in the singular, Player always refers to the client connected to the server. |
| Entity | Entity refers to any item, player, mob, minecart or boat etc. See [the Minecraft Wiki article](https://minecraft.wiki/w/Entity) for a full list. |
| EID | An EID — or Entity ID — is a 4-byte sequence used to identify a specific entity. An entity's EID is unique on the entire server. |
| XYZ | In this document, the axis names are the same as those shown in the debug screen (F3). Y points upwards, X points east, and Z points south. |
| Meter | The meter is Minecraft's base unit of length, equal to the length of a vertex of a solid block. The term “block” may be used to mean “meter” or “cubic meter”. |
| Registry | A table describing static, gameplay-related objects of some kind, such as the types of entities, block states or biomes. The entries of a registry are typically associated with textual or numeric identifiers, or both.
Minecraft has a unified registry system used to implement most of the registries, including blocks, items, entities, biomes and dimensions. These "ordinary" registries associate entries with both namespaced textual identifiers (see [#Identifier](https://wiki.vg/Protocol#Identifier)), and signed (positive) 32-bit numeric identifiers. There is also a registry of registries listing all of the registries in the registry system. Some other registries, most notably the [block state registry](https://wiki.vg/Chunk_Format#Block_state_registry "Chunk Format"), are however implemented in a more ad-hoc fashion.

Some registries, such as biomes and dimensions, can be customized at runtime by the server (see [Registry Data](https://wiki.vg/Registry_Data "Registry Data")), while others, such as blocks, items and entities, are hardcoded. The contents of the hardcoded registries can be extracted via the built-in [Data Generators](https://wiki.vg/Data_Generators "Data Generators") system.

 |
| Block state | Each block in Minecraft has 0 or more properties, which in turn may have any number of possible values. These represent, for example, the orientations of blocks, poweredness states of redstone components, and so on. Each of the possible permutations of property values for a block is a distinct block state. The block state registry assigns a numeric identifier to every block state of every block.

A current list of properties and state ID ranges is found on [burger](https://pokechu22.github.io/Burger/1.21.html).

Alternatively, the vanilla server now includes an option to export the current block state ID mapping, by running `java -cp minecraft_server.jar net.minecraft.data.Main --reports`. See [Data Generators](https://wiki.vg/Data_Generators "Data Generators") for more information.

 |
| Notchian | The official implementation of vanilla Minecraft as developed and released by Mojang. |
| Sequence | The action number counter for local block changes, incremented by one when clicking a block with a hand, right clicking an item, or starting or finishing digging a block. Counter handles latency to avoid applying outdated block changes to the local world. Also is used to revert ghost blocks created when placing blocks, using buckets, or breaking blocks. |

## Packet format

Packets cannot be larger than 2<sup>21</sup> − 1 or 2097151 bytes (the maximum that can be sent in a 3-byte [VarInt](https://wiki.vg/Protocol#Type:VarInt)). Moreover, the length field must not be longer than 3 bytes, even if the encoded value is within the limit. Unnecessarily long encodings at 3 bytes or below are still allowed. For compressed packets, this applies to the Packet Length field, i.e. the compressed length.

### Without compression

| Field Name | Field Type | Notes |
| --- | --- | --- |
| Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of Packet ID + Data |
| Packet ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | Depends on the connection state and packet ID, see the sections below |

### With compression

Once a [Set Compression](https://wiki.vg/Protocol#Set_Compression) packet (with a non-negative threshold) is sent, [zlib](http://en.wikipedia.org/wiki/Zlib "wikipedia:Zlib") compression is enabled for all following packets. The format of a packet changes slightly to include the size of the uncompressed packet.

| Present? | Compressed? | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- |
| always | No | Packet Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of (Data Length) + length of compressed (Packet ID + Data) |
| if size >= threshold | No | Data Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of uncompressed (Packet ID + Data) |
| Yes | Packet ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | zlib compressed packet ID (see the sections below) |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | zlib compressed packet data (see the sections below) |
| if size < threshold | No | Data Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | 0 to indicate uncompressed |
| Packet ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | packet ID (see the sections below) |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | packet data (see the sections below) |

For serverbound packets, the uncompressed length of (Packet ID + Data) must not be greater than 2<sup>23</sup> or 8388608 bytes. Not that a length equal to 2<sup>23</sup> is permitted, which differs from the compressed length limit. The Notchian client, on the other hand, has no limit for the uncompressed length of incoming compressed packets.

If the size of the buffer containing the packet data and ID (as a [VarInt](https://wiki.vg/Protocol#Type:VarInt)) is smaller than the threshold specified in the packet [Set Compression](https://wiki.vg/Protocol#Set_Compression). It will be sent as uncompressed. This is done by setting the data length as 0. (Comparable to sending a non-compressed format with an extra 0 between the length, and packet data).

If it's larger than or equal to the threshold, then it follows the regular compressed protocol format.

The Notchian server (but not client) rejects compressed packets smaller than the threshold. Uncompressed packets exceeding the threshold, however, are accepted.

Compression can be disabled by sending the packet [Set Compression](https://wiki.vg/Protocol#Set_Compression) with a negative Threshold, or not sending the Set Compression packet at all.

## Handshaking

### Clientbound

There are no clientbound packets in the Handshaking state, since the protocol immediately switches to a different state after the client sends the first packet.

### Serverbound

#### Handshake

This causes the server to switch into the target state.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Handshaking | Server | Protocol Version | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | See [protocol version numbers](https://wiki.vg/Protocol_version_numbers "Protocol version numbers") (currently 767 in Minecraft 1.21). |
| Server Address | [String](https://wiki.vg/Protocol#Type:String) (255) | Hostname or IP, e.g. localhost or 127.0.0.1, that was used to connect. The Notchian server does not use this information. Note that SRV records are a simple redirect, e.g. if \_minecraft.\_tcp.example.com points to mc.example.org, users connecting to example.com will provide example.org as server address in addition to connecting to it. |
| Server Port | [Unsigned Short](https://wiki.vg/Protocol#Type:Unsigned_Short) | Default is 25565. The Notchian server does not use this information. |
| Next State | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 1 for [Status](https://wiki.vg/Protocol#Status), 2 for [Login](https://wiki.vg/Protocol#Login), 3 for [Transfer](https://wiki.vg/Protocol#Login). |

#### Legacy Server List Ping

 [![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) This packet uses a nonstandard format. It is never length-prefixed, and the packet ID is an [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) instead of a [VarInt](https://wiki.vg/Protocol#Type:VarInt).

While not technically part of the current protocol, legacy clients may send this packet to initiate [Server List Ping](https://wiki.vg/Server_List_Ping "Server List Ping"), and modern servers should handle it correctly. The format of this packet is a remnant of the pre-Netty age, before the switch to Netty in 1.7 brought the standard format that is recognized now. This packet merely exists to inform legacy clients that they can't join our modern server.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0xFE | Handshaking | Server | Payload | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | always 1 (`0x01`). |

See [Server List Ping#1.6](https://wiki.vg/Server_List_Ping#1.6 "Server List Ping") for the details of the protocol that follows this packet.

## Status

_Main article: [Server List Ping](https://wiki.vg/Server_List_Ping "Server List Ping")_

### Clientbound

#### Status Response

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Status | Client |
| JSON Response | [String](https://wiki.vg/Protocol#Type:String) (32767) | See [Server List Ping#Status Response](https://wiki.vg/Server_List_Ping#Status_Response "Server List Ping"); as with all strings this is prefixed by its length as a [VarInt](https://wiki.vg/Protocol#Type:VarInt). |

#### Ping Response (status)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Status | Client | Payload | [Long](https://wiki.vg/Protocol#Type:Long) | Should be the same as sent by the client. |

### Serverbound

#### Status Request

The status can only be requested once immediately after the handshake, before any ping. The server won't respond otherwise.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Status | Server | _no fields_ |

#### Ping Request (status)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Status | Server | Payload | [Long](https://wiki.vg/Protocol#Type:Long) | May be any number. Notchian clients use a system-dependent time value which is counted in milliseconds. |

## Login

The login process is as follows:

1.  C→S: [Handshake](https://wiki.vg/Protocol#Handshake) with Next State set to 2 (login)
2.  C→S: [Login Start](https://wiki.vg/Protocol#Login_Start)
3.  S→C: [Encryption Request](https://wiki.vg/Protocol#Encryption_Request)
4.  Client auth (if enabled)
5.  C→S: [Encryption Response](https://wiki.vg/Protocol#Encryption_Response)
6.  Server auth (if enabled)
7.  Both enable encryption
8.  S→C: [Set Compression](https://wiki.vg/Protocol#Set_Compression) (optional)
9.  S→C: [Login Success](https://wiki.vg/Protocol#Login_Success)
10.  C→S: [Login Acknowledged](https://wiki.vg/Protocol#Login_Acknowledged)

Set Compression, if present, must be sent before Login Success. Note that anything sent after Set Compression must use the [Post Compression packet format](https://wiki.vg/Protocol#With_compression).

Three modes of operation are possible depending on how the packets are sent:

-   Online-mode with encryption
-   Offline-mode with encryption
-   Offline-mode without encryption

For online-mode servers (the ones with authentication enabled), encryption is always mandatory, and the entire process described above needs to be followed.

For offline-mode servers (the ones with authentication disabled), encryption is optional, and part of the process can be skipped. In that case [Login Start](https://wiki.vg/Protocol#Login_Start) is directly followed by [Login Success](https://wiki.vg/Protocol#Login_Success). The Notchian server uses UUID v3 for offline player UUIDs, with the namespace “OfflinePlayer” and the value as the player’s username. For example, Notch’s offline UUID would be derived from the string “OfflinePlayer:Notch”. This is not a requirement however, the UUID may be anything.

As of 1.21, the Notchian server never uses encryption in offline mode.

See [Protocol Encryption](https://wiki.vg/Protocol_Encryption "Protocol Encryption") for details.

### Clientbound

#### Disconnect (login)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Login | Client | Reason | [JSON Text Component](https://wiki.vg/Protocol#Type:JSON_Text_Component) | The reason why the player was disconnected. |

#### Encryption Request

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Login | Client | Server ID | [String](https://wiki.vg/Protocol#Type:String) (20) | Appears to be empty. |
| Public Key Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of Public Key. |
| Public Key | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | The server's public key, in bytes. |
| Verify Token Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of Verify Token. Always 4 for Notchian servers. |
| Verify Token | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | A sequence of random bytes generated by the server. |
| Should authenticate | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Wheather the client should attempt to [authenticate through mojang servers](https://wiki.vg/Protocol_Encryption#Authentication "Protocol Encryption"). |

See [Protocol Encryption](https://wiki.vg/Protocol_Encryption "Protocol Encryption") for details.

#### Login Success

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x02 | Login | Client | UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) |  |
| Username | [String](https://wiki.vg/Protocol#Type:String) (16) |  |
| Number Of Properties | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Property | Name | [Array](https://wiki.vg/Protocol#Type:Array) | [String](https://wiki.vg/Protocol#Type:String) (32767) |  |
| Value | [String](https://wiki.vg/Protocol#Type:String) (32767) |  |
| Is Signed | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Signature | [Optional](https://wiki.vg/Protocol#Type:Optional) [String](https://wiki.vg/Protocol#Type:String) (32767) | Only if Is Signed is true. |
| Strict Error Handling | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the client should immediately disconnect upon a packet processing error. The Notchian client silently ignores them when this flag is false.
[![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) This field was temporarily added in 1.20.5 as a [way to aid modded servers with the transition to the new data pack & registry system](https://www.minecraft.net/en-us/article/minecraft-java-edition-1-20-5), allowing them to tell the client to silently ignore packets containing inconsistent data. **It will likely be removed soon.**



 |

The properties included in this packet are the same as in [Player Info Update](https://wiki.vg/Protocol#Player_Info_Update), for the current player.

#### Set Compression

Enables compression. If compression is enabled, all following packets are encoded in the [compressed packet format](https://wiki.vg/Protocol#With_compression). Negative values will disable compression, meaning the packet format should remain in the [uncompressed packet format](https://wiki.vg/Protocol#Without_compression). However, this packet is entirely optional, and if not sent, compression will also not be enabled (the notchian server does not send the packet when compression is disabled).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x03 | Login | Client | Threshold | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Maximum size of a packet before it is compressed. |

#### Login Plugin Request

Used to implement a custom handshaking flow together with [Login Plugin Response](https://wiki.vg/Protocol#Login_Plugin_Response).

Unlike plugin messages in "play" mode, these messages follow a lock-step request/response scheme, where the client is expected to respond to a request indicating whether it understood. The notchian client always responds that it hasn't understood, and sends an empty payload.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x04 | Login | Client | Message ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Generated by the server - should be unique to the connection. |
| Channel | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the [plugin channel](https://wiki.vg/Plugin_channel "Plugin channel") used to send the data. |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (1048576) | Any data, depending on the channel. The length of this array must be inferred from the packet length. |

In Notchian client, the maximum data length is 1048576 bytes.

#### Cookie Request (login)

Requests a cookie that was previously stored.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x05 | Login | Client | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |

### Serverbound

#### Login Start

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Login | Server | Name | [String](https://wiki.vg/Protocol#Type:String) (16) | Player's Username. |
| Player UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) | The [UUID](https://wiki.vg/Protocol#Type:UUID) of the player logging in. Unused by the Notchian server. |

#### Encryption Response

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Login | Server | Shared Secret Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of Shared Secret. |
| Shared Secret | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | Shared Secret value, encrypted with the server's public key. |
| Verify Token Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of Verify Token. |
| Verify Token | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | Verify Token value, encrypted with the same public key as the shared secret. |

See [Protocol Encryption](https://wiki.vg/Protocol_Encryption "Protocol Encryption") for details.

#### Login Plugin Response

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x02 | Login | Server | Message ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Should match ID from server. |
| Successful | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | `true` if the client understood the request, `false` otherwise. When `false`, no payload follows. |
| Data | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (1048576) | Any data, depending on the channel. The length of this array must be inferred from the packet length. |

In Notchian server, the maximum data length is 1048576 bytes.

#### Login Acknowledged

Acknowledgement to the [Login Success](https://wiki.vg/Protocol#Login_Success "Protocol") packet sent by the server.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x03 | Login | Server | _no fields_ |

This packet switches the connection state to [configuration](https://wiki.vg/Protocol#Configuration).

#### Cookie Response (login)

Response to a [Cookie Request (login)](https://wiki.vg/Protocol#Cookie_Request_.28login.29) from the server. The Notchian server only accepts responses of up to 5 kiB in size.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x04 | Login | Server | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |
| Has Payload | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | The payload is only present if the cookie exists on the client. |
| Payload Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following byte array. |
| Payload | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (5120) | The data of the cookie, if any. |

## Configuration

### Clientbound

#### Cookie Request (configuration)

Requests a cookie that was previously stored.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Configuration | Client | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |

#### Clientbound Plugin Message (configuration)

_Main article: [Plugin channels](https://wiki.vg/Plugin_channels "Plugin channels")_

Mods and plugins can use this to send their data. Minecraft itself uses several [plugin channels](https://wiki.vg/Plugin_channel "Plugin channel"). These internal channels are in the `minecraft` namespace.

More information on how it works on [Dinnerbone's blog](https://web.archive.org/web/20220831140929/https://dinnerbone.com/blog/2012/01/13/minecraft-plugin-channels-messaging/). More documentation about internal and popular registered channels are [here](https://wiki.vg/Plugin_channel "Plugin channel").

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Configuration | Client | Channel | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the [plugin channel](https://wiki.vg/Plugin_channel "Plugin channel") used to send the data. |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (1048576) | Any data. The length of this array must be inferred from the packet length. |

In Notchian client, the maximum data length is 1048576 bytes.

#### Disconnect (configuration)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x02 | Configuration | Client | Reason | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The reason why the player was disconnected. |

#### Finish Configuration

Sent by the server to notify the client that the configuration process has finished. The client answers with [Acknowledge Finish Configuration](https://wiki.vg/Protocol#Acknowledge_Finish_Configuration) whenever it is ready to continue.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x03 | Configuration | Client | _no fields_ |

This packet switches the connection state to [play](https://wiki.vg/Protocol#Play).

#### Clientbound Keep Alive (configuration)

The server will frequently send out a keep-alive, each containing a random ID. The client must respond with the same payload (see [Serverbound Keep Alive](https://wiki.vg/Protocol#Serverbound_Keep_Alive_.28configuration.29)). If the client does not respond to a Keep Alive packet within 15 seconds after it was sent, the server kicks the client. Vice versa, if the server does not send any keep-alives for 20 seconds, the client will disconnect and yields a "Timed out" exception.

The Notchian server uses a system-dependent time in milliseconds to generate the keep alive ID value.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x04 | Configuration | Client | Keep Alive ID | [Long](https://wiki.vg/Protocol#Type:Long) |  |

#### Ping (configuration)

Packet is not used by the Notchian server. When sent to the client, client responds with a [Pong](https://wiki.vg/Protocol#Pong_.28configuration.29) packet with the same id.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x05 | Configuration | Client | ID | [Int](https://wiki.vg/Protocol#Type:Int) |  |

#### Reset Chat

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x06 | Configuration | Client | _no fields_ |

#### Registry Data

Represents certain registries that are sent from the server and are applied on the client.

See [Registry Data](https://wiki.vg/Registry_Data "Registry Data") for details.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x07 | Configuration | Client |
| Registry ID | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Entry Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of entries in the following array. |
| Entries | Entry ID | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Has Data | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the entry has any data following. |
| Data | [NBT](https://wiki.vg/Protocol#Type:NBT) | Entry data. Only present if Has Data is true. |

#### Remove Resource Pack (configuration)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x08 | Configuration | Client | Has UUID | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether a specific resource pack should be removed, or all of them. |
| UUID | [Optional](https://wiki.vg/Protocol#Type:Optional) [UUID](https://wiki.vg/Protocol#Type:UUID) | The [UUID](https://wiki.vg/Protocol#Type:UUID) of the resource pack to be removed. Only present if the previous field is true. |

#### Add Resource Pack (configuration)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x09 | Configuration | Client | UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) | The unique identifier of the resource pack. |
| URL | [String](https://wiki.vg/Protocol#Type:String) (32767) | The URL to the resource pack. |
| Hash | [String](https://wiki.vg/Protocol#Type:String) (40) | A 40 character hexadecimal, case-insensitive [SHA-1](http://en.wikipedia.org/wiki/SHA-1 "wikipedia:SHA-1") hash of the resource pack file.  
If it's not a 40 character hexadecimal string, the client will not use it for hash verification and likely waste bandwidth. |
| Forced | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | The Notchian client will be forced to use the resource pack from the server. If they decline they will be kicked from the server. |
| Has Prompt Message | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether a custom message should be used on the resource pack prompt. |
| Prompt Message | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | This is shown in the prompt making the client accept or decline the resource pack. Only present if the previous field is true. |

#### Store Cookie (configuration)

Stores some arbitrary data on the client, which persists between server transfers. The Notchian client only accepts cookies of up to 5 kiB in size.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0A | Configuration | Client | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |
| Payload | [ByteArray](https://wiki.vg/Protocol#Type:ByteArray) (5120) | The data of the cookie. |

#### Transfer (configuration)

Notifies the client that it should transfer to the given server. Cookies previously stored are preserved between server transfers.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0B | Configuration | Client | Host | [String](https://wiki.vg/Protocol#Type:String) | The hostname or IP of the server. |
| Port | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The port of the server. |

#### Feature Flags

Used to enable and disable features, generally experimental ones, on the client.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0C | Configuration | Client | Total Features | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of features that appear in the array below. |
| Feature Flags | [Array](https://wiki.vg/Protocol#Type:Array) of [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |

As of 1.21, the following feature flags are available:

-   minecraft:vanilla - enables vanilla features
-   minecraft:bundle - enables support for the bundle
-   minecraft:trade\_rebalance - enables support for the rebalanced villager trades

#### Update Tags (configuration)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0D | Configuration | Client | Length of the array | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Array of tags | Registry | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Registry identifier (Vanilla expects tags for the registries `minecraft:block`, `minecraft:item`, `minecraft:fluid`, `minecraft:entity_type`, and `minecraft:game_event`) |
| Array of Tag | (See below) |  |

Tag arrays look like:

| Field Name | Field Type | Notes |
| --- | --- | --- |
| Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array |
| Tags | Tag name | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array |
| Entries | [Array](https://wiki.vg/Protocol#Type:Array) of [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Numeric IDs of the given type (block, item, etc.). This list replaces the previous list of IDs for the given tag. If some preexisting tags are left unmentioned, a warning is printed. |

See [Tag](https://minecraft.wiki/w/Tag) on the Minecraft Wiki for more information, including a list of vanilla tags.

#### Clientbound Known Packs

Informs the client of which data packs are present on the server. The client is expected to respond with its own [Serverbound Known Packs](https://wiki.vg/Protocol#Serverbound_Known_Packs) packet. The Notchian server does not continue with Configuration until it receives a response.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0E | Configuration | Client | Known Pack Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The number of known packs in the following array. |
| Known Packs | Namespace | [Array](https://wiki.vg/Protocol#Type:Array) | [String](https://wiki.vg/Protocol#Type:String) |  |
| ID | [String](https://wiki.vg/Protocol#Type:String) |  |
| Version | [String](https://wiki.vg/Protocol#Type:String) |  |

#### Custom Report Details (configuration)

Contains a list of key-value text entries that are included in any crash or disconnection report generated during connection to the server.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0F | Configuration | Client | Details Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) (32) | The number of details in the following array. |
| Details | Title | [Array](https://wiki.vg/Protocol#Type:Array) | [String](https://wiki.vg/Protocol#Type:String) (128) |  |
| Description | [String](https://wiki.vg/Protocol#Type:String) (4096) |  |

#### Server Links (configuration)

This packet contains a list of links that the Notchian client will display in the menu available from the pause menu. Link labels can be built-in or custom (i.e., any text).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x10 | Configuration | Client | Links Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The number of links in the following array. |
| Links | Is built-in | [Array](https://wiki.vg/Protocol#Type:Array) | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Determines if the following label is built-in (from enum) or custom (text component). |
| Label | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) / [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | See below. |
| URL | [String](https://wiki.vg/Protocol#Type:String) | Valid URL. |

| ID | Name | Notes |
| --- | --- | --- |
| 0 | Bug Report | Displayed on connection error screen; included as a comment in the disconnection report. |
| 1 | Community Guidelines |  |
| 2 | Support |  |
| 3 | Status |  |
| 4 | Feedback |  |
| 5 | Community |  |
| 6 | Website |  |
| 7 | Forums |  |
| 8 | News |  |
| 9 | Announcements |  |

### Serverbound

#### Client Information (configuration)

Sent when the player connects, or when settings are changed.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Configuration | Server | Locale | [String](https://wiki.vg/Protocol#Type:String) (16) | e.g. `en_GB`. |
| View Distance | [Byte](https://wiki.vg/Protocol#Type:Byte) | Client-side render distance, in chunks. |
| Chat Mode | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: enabled, 1: commands only, 2: hidden. See [Chat#Client chat mode](https://wiki.vg/Chat#Client_chat_mode "Chat") for more information. |
| Chat Colors | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | “Colors” multiplayer setting. Can the chat be colored? |
| Displayed Skin Parts | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Bit mask, see below. |
| Main Hand | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: Left, 1: Right. |
| Enable text filtering | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Enables filtering of text on signs and written book titles. Currently always false (i.e. the filtering is disabled) |
| Allow server listings | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Servers usually list online players, this option should let you not show up in that list. |

_Displayed Skin Parts_ flags:

-   Bit 0 (0x01): Cape enabled
-   Bit 1 (0x02): Jacket enabled
-   Bit 2 (0x04): Left Sleeve enabled
-   Bit 3 (0x08): Right Sleeve enabled
-   Bit 4 (0x10): Left Pants Leg enabled
-   Bit 5 (0x20): Right Pants Leg enabled
-   Bit 6 (0x40): Hat enabled

The most significant bit (bit 7, 0x80) appears to be unused.

#### Cookie Response (configuration)

Response to a [Cookie Request (configuration)](https://wiki.vg/Protocol#Cookie_Request_.28configuration.29) from the server. The Notchian server only accepts responses of up to 5 kiB in size.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Configuration | Server | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |
| Has Payload | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | The payload is only present if the cookie exists on the client. |
| Payload Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following byte array. |
| Payload | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (5120) | The data of the cookie, if any. |

#### Serverbound Plugin Message (configuration)

_Main article: [Plugin channels](https://wiki.vg/Plugin_channels "Plugin channels")_

Mods and plugins can use this to send their data. Minecraft itself uses some [plugin channels](https://wiki.vg/Plugin_channel "Plugin channel"). These internal channels are in the `minecraft` namespace.

More documentation on this: [https://dinnerbone.com/blog/2012/01/13/minecraft-plugin-channels-messaging/](https://dinnerbone.com/blog/2012/01/13/minecraft-plugin-channels-messaging/)

Note that the length of Data is known only from the packet length, since the packet has no length field of any kind.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x02 | Configuration | Server | Channel | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the [plugin channel](https://wiki.vg/Plugin_channel "Plugin channel") used to send the data. |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (32767) | Any data, depending on the channel. `minecraft:` channels are documented [here](https://wiki.vg/Plugin_channel "Plugin channel"). The length of this array must be inferred from the packet length. |

In Notchian server, the maximum data length is 32767 bytes.

#### Acknowledge Finish Configuration

Sent by the client to notify the server that the configuration process has finished. It is sent in response to the server's [Finish Configuration](https://wiki.vg/Protocol#Finish_Configuration).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x03 | Configuration | Server | _no fields_ |

This packet switches the connection state to [play](https://wiki.vg/Protocol#Play).

#### Serverbound Keep Alive (configuration)

The server will frequently send out a keep-alive (see [Clientbound Keep Alive](https://wiki.vg/Protocol#Clientbound_Keep_Alive_.28configuration.29)), each containing a random ID. The client must respond with the same packet.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x04 | Configuration | Server | Keep Alive ID | [Long](https://wiki.vg/Protocol#Type:Long) |  |

#### Pong (configuration)

Response to the clientbound packet ([Ping](https://wiki.vg/Protocol#Ping_.28configuration.29)) with the same id.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x05 | Configuration | Server | ID | [Int](https://wiki.vg/Protocol#Type:Int) | id is the same as the ping packet |

#### Resource Pack Response (configuration)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x06 | Configuration | Server |
| UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) | The unique identifier of the resource pack received in the [Add Resource Pack (configuration)](https://wiki.vg/Protocol#Add_Resource_Pack_.28configuration.29) request. |
| Result | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Result ID (see below). |

Result can be one of the following values:

| ID | Result |
| --- | --- |
| 0 | Successfully downloaded |
| 1 | Declined |
| 2 | Failed to download |
| 3 | Accepted |
| 4 | Downloaded |
| 5 | Invalid URL |
| 6 | Failed to reload |
| 7 | Discarded |

#### Serverbound Known Packs

Informs the server of which data packs are present on the client. The client sends this in response to [Clientbound Known Packs](https://wiki.vg/Protocol#Clientbound_Known_Packs).

If the client specifies a pack in this packet, the server should omit its contained data from the [Registry Data](https://wiki.vg/Protocol#Registry_Data) packet.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x07 | Configuration | Server | Known Pack Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The number of known packs in the following array. |
| Known Packs | Namespace | [Array](https://wiki.vg/Protocol#Type:Array) | [String](https://wiki.vg/Protocol#Type:String) |  |
| ID | [String](https://wiki.vg/Protocol#Type:String) |  |
| Version | [String](https://wiki.vg/Protocol#Type:String) |  |

## Play

### Clientbound

#### Bundle Delimiter

The delimiter for a bundle of packets. When received, the client should store every subsequent packet it receives, and wait until another delimiter is received. Once that happens, the client is guaranteed to process every packet in the bundle on the same tick, and the client should stop storing packets.

As of 1.20.6, the Notchian server only uses this to ensure [Spawn Entity](https://wiki.vg/Protocol#Spawn_Entity) and associated packets used to configure the entity happen on the same tick. Each entity gets a separate bundle.

The Notchian client doesn't allow more than 4096 packets in the same bundle.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Play | Client | _no fields_ |

#### Spawn Entity

Sent by the server when an entity (aside from [Experince Orb](https://wiki.vg/Protocol#Spawn_Experience_Orb)) is created.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | A unique integer ID mostly used in the protocol to identify the entity. |
| Entity UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) | A unique identifier that is mostly used in persistence and places where the uniqueness matters more. |
| Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | ID in the `minecraft:entity_type` registry (see "type" field in [Entity metadata#Entities](https://wiki.vg/Entity_metadata#Entities "Entity metadata")). |
| X | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Pitch | [Angle](https://wiki.vg/Protocol#Type:Angle) | To get the real pitch, you must divide this by (256.0F / 360.0F) |
| Yaw | [Angle](https://wiki.vg/Protocol#Type:Angle) | To get the real yaw, you must divide this by (256.0F / 360.0F) |
| Head Yaw | [Angle](https://wiki.vg/Protocol#Type:Angle) | Only used by living entities, where the head of the entity may differ from the general body rotation. |
| Data | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Meaning dependent on the value of the Type field, see [Object Data](https://wiki.vg/Object_Data "Object Data") for details. |
| Velocity X | [Short](https://wiki.vg/Protocol#Type:Short) | Same units as [Set Entity Velocity](https://wiki.vg/Protocol#Set_Entity_Velocity). |
| Velocity Y | [Short](https://wiki.vg/Protocol#Type:Short) |
| Velocity Z | [Short](https://wiki.vg/Protocol#Type:Short) |

[![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) The points listed below should be considered when this packet is used to spawn a player entity.

When in [online mode](https://minecraft.wiki/w/Server.properties%23online-mode), the UUIDs must be valid and have valid skin blobs. In offline mode, [UUID v3](http://en.wikipedia.org/wiki/Universally_unique_identifier#Versions_3_and_5_.28namespace_name-based.29 "wikipedia:Universally unique identifier") is used with the String `OfflinePlayer:<player name>`, encoded in UTF-8 (and case-sensitive). The Notchian server uses `UUID.nameUUIDFromBytes`, implemented by OpenJDK [here](https://github.com/AdoptOpenJDK/openjdk-jdk8u/blob/9a91972c76ddda5c1ce28b50ca38cbd8a30b7a72/jdk/src/share/classes/java/util/UUID.java#L153-L175).

For NPCs UUID v2 should be used. Note:

```
&lt;+Grum&gt; i will never confirm this as a feature you know that&nbsp;:)
```

In an example UUID, `xxxxxxxx-xxxx-Yxxx-xxxx-xxxxxxxxxxxx`, the UUID version is specified by `Y`. So, for UUID v3, `Y` will always be `3`, and for UUID v2, `Y` will always be `2`.

#### Spawn Experience Orb

Spawns one or more experience orbs.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x02 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| X | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Count | [Short](https://wiki.vg/Protocol#Type:Short) | The amount of experience this orb will reward once collected. |

#### Entity Animation

Sent whenever an entity should change animation.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x03 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Player ID. |
| Animation | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Animation ID (see below). |

Animation can be one of the following values:

| ID | Animation |
| --- | --- |
| 0 | Swing main arm |
| 2 | Leave bed |
| 3 | Swing offhand |
| 4 | Critical effect |
| 5 | Magic critical effect |

#### Award Statistics

Sent as a response to [Client Command](https://wiki.vg/Protocol#Client_Command) (id 1). Will only send the changed values if previously requested.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x04 | Play | Client | Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Statistic | Category ID | [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | See below. |
| Statistic ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | See below. |
| Value | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The amount to set it to. |

Categories (these are namespaced, but with `:` replaced with `.`):

| Name | ID | Registry |
| --- | --- | --- |
| `minecraft.mined` | 0 | Blocks |
| `minecraft.crafted` | 1 | Items |
| `minecraft.used` | 2 | Items |
| `minecraft.broken` | 3 | Items |
| `minecraft.picked_up` | 4 | Items |
| `minecraft.dropped` | 5 | Items |
| `minecraft.killed` | 6 | Entities |
| `minecraft.killed_by` | 7 | Entities |
| `minecraft.custom` | 8 | Custom |

Blocks, Items, and Entities use block (not block state), item, and entity ids.

Custom has the following (unit only matters for clients):

| Name | ID | Unit |
| --- | --- | --- |
| `minecraft.leave_game` | 0 | None |
| `minecraft.play_one_minute` | 1 | Time |
| `minecraft.time_since_death` | 2 | Time |
| `minecraft.time_since_rest` | 3 | Time |
| `minecraft.sneak_time` | 4 | Time |
| `minecraft.walk_one_cm` | 5 | Distance |
| `minecraft.crouch_one_cm` | 6 | Distance |
| `minecraft.sprint_one_cm` | 7 | Distance |
| `minecraft.walk_on_water_one_cm` | 8 | Distance |
| `minecraft.fall_one_cm` | 9 | Distance |
| `minecraft.climb_one_cm` | 10 | Distance |
| `minecraft.fly_one_cm` | 11 | Distance |
| `minecraft.walk_under_water_one_cm` | 12 | Distance |
| `minecraft.minecart_one_cm` | 13 | Distance |
| `minecraft.boat_one_cm` | 14 | Distance |
| `minecraft.pig_one_cm` | 15 | Distance |
| `minecraft.horse_one_cm` | 16 | Distance |
| `minecraft.aviate_one_cm` | 17 | Distance |
| `minecraft.swim_one_cm` | 18 | Distance |
| `minecraft.strider_one_cm` | 19 | Distance |
| `minecraft.jump` | 20 | None |
| `minecraft.drop` | 21 | None |
| `minecraft.damage_dealt` | 22 | Damage |
| `minecraft.damage_dealt_absorbed` | 23 | Damage |
| `minecraft.damage_dealt_resisted` | 24 | Damage |
| `minecraft.damage_taken` | 25 | Damage |
| `minecraft.damage_blocked_by_shield` | 26 | Damage |
| `minecraft.damage_absorbed` | 27 | Damage |
| `minecraft.damage_resisted` | 28 | Damage |
| `minecraft.deaths` | 29 | None |
| `minecraft.mob_kills` | 30 | None |
| `minecraft.animals_bred` | 31 | None |
| `minecraft.player_kills` | 32 | None |
| `minecraft.fish_caught` | 33 | None |
| `minecraft.talked_to_villager` | 34 | None |
| `minecraft.traded_with_villager` | 35 | None |
| `minecraft.eat_cake_slice` | 36 | None |
| `minecraft.fill_cauldron` | 37 | None |
| `minecraft.use_cauldron` | 38 | None |
| `minecraft.clean_armor` | 39 | None |
| `minecraft.clean_banner` | 40 | None |
| `minecraft.clean_shulker_box` | 41 | None |
| `minecraft.interact_with_brewingstand` | 42 | None |
| `minecraft.interact_with_beacon` | 43 | None |
| `minecraft.inspect_dropper` | 44 | None |
| `minecraft.inspect_hopper` | 45 | None |
| `minecraft.inspect_dispenser` | 46 | None |
| `minecraft.play_noteblock` | 47 | None |
| `minecraft.tune_noteblock` | 48 | None |
| `minecraft.pot_flower` | 49 | None |
| `minecraft.trigger_trapped_chest` | 50 | None |
| `minecraft.open_enderchest` | 51 | None |
| `minecraft.enchant_item` | 52 | None |
| `minecraft.play_record` | 53 | None |
| `minecraft.interact_with_furnace` | 54 | None |
| `minecraft.interact_with_crafting_table` | 55 | None |
| `minecraft.open_chest` | 56 | None |
| `minecraft.sleep_in_bed` | 57 | None |
| `minecraft.open_shulker_box` | 58 | None |
| `minecraft.open_barrel` | 59 | None |
| `minecraft.interact_with_blast_furnace` | 60 | None |
| `minecraft.interact_with_smoker` | 61 | None |
| `minecraft.interact_with_lectern` | 62 | None |
| `minecraft.interact_with_campfire` | 63 | None |
| `minecraft.interact_with_cartography_table` | 64 | None |
| `minecraft.interact_with_loom` | 65 | None |
| `minecraft.interact_with_stonecutter` | 66 | None |
| `minecraft.bell_ring` | 67 | None |
| `minecraft.raid_trigger` | 68 | None |
| `minecraft.raid_win` | 69 | None |
| `minecraft.interact_with_anvil` | 70 | None |
| `minecraft.interact_with_grindstone` | 71 | None |
| `minecraft.target_hit` | 72 | None |
| `minecraft.interact_with_smithing_table` | 73 | None |

Units:

-   None: just a normal number (formatted with 0 decimal places)
-   Damage: value is 10 times the normal amount
-   Distance: a distance in centimeters (hundredths of blocks)
-   Time: a time span in ticks

#### Acknowledge Block Change

Acknowledges a user-initiated block change. After receiving this packet, the client will display the block state sent by the server instead of the one predicted by the client.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x05 | Play | Client | Sequence ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Represents the sequence to acknowledge, this is used for properly syncing block changes to the client after interactions. |

#### Set Block Destroy Stage

0–9 are the displayable destroy stages and each other number means that there is no animation on this coordinate.

Block break animations can still be applied on air; the animation will remain visible although there is no block being broken. However, if this is applied to a transparent block, odd graphical effects may happen, including water losing its transparency. (An effect similar to this can be seen in normal gameplay when breaking ice blocks)

If you need to display several break animations at the same time you have to give each of them a unique Entity ID. The entity ID does not need to correspond to an actual entity on the client. It is valid to use a randomly generated number.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x06 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of the entity breaking the block. |
| Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block Position. |
| Destroy Stage | [Byte](https://wiki.vg/Protocol#Type:Byte) | 0–9 to set it, any other value to remove it. |

#### Block Entity Data

Sets the block entity associated with the block at the given location.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x07 | Play | Client | Location | [Position](https://wiki.vg/Protocol#Type:Position) |  |
| Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The type of the block entity |
| NBT Data | [NBT](https://wiki.vg/Protocol#Type:NBT) | Data to set. May be a TAG\_END (0), in which case the block entity at the given location is removed (though this is not required since the client will remove the block entity automatically on chunk unload or block removal). |

#### Block Action

This packet is used for a number of actions and animations performed by blocks, usually non-persistent. The client ignores the provided block type and instead uses the block state in their world.

See [Block Actions](https://wiki.vg/Block_Actions "Block Actions") for a list of values.

[![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) This packet uses a block ID from the `minecraft:block` registry, not a block state.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x08 | Play | Client | Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block coordinates. |
| Action ID (Byte 1) | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Varies depending on block — see [Block Actions](https://wiki.vg/Block_Actions "Block Actions"). |
| Action Parameter (Byte 2) | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Varies depending on block — see [Block Actions](https://wiki.vg/Block_Actions "Block Actions"). |
| Block Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The block type ID for the block. This value is unused by the Notchian client, as it will infer the type of block based on the given position. |

#### Block Update

Fired whenever a block is changed within the render distance.

[![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) Changing a block in a chunk that is not loaded is not a stable action. The Notchian client currently uses a _shared_ empty chunk which is modified for all block changes in unloaded chunks; while in 1.9 this chunk never renders in older versions the changed block will appear in all copies of the empty chunk. Servers should avoid sending block changes in unloaded chunks and clients should ignore such packets.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x09 | Play | Client | Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block Coordinates. |
| Block ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The new block state ID for the block as given in the [block state registry](https://wiki.vg/Chunk_Format#Block_state_registry "Chunk Format"). |

#### Boss Bar

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0A | Play | Client | UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) | Unique ID for this bar. |
| Action | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Determines the layout of the remaining packet. |
| Action | Field Name |  |  |
| 0: add | Title | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| Health | [Float](https://wiki.vg/Protocol#Type:Float) | From 0 to 1. Values greater than 1 do not crash a Notchian client, and start [rendering part of a second health bar](https://i.johni0702.de/nA.png) at around 1.5. |
| Color | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Color ID (see below). |
| Division | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Type of division (see below). |
| Flags | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Bit mask. 0x1: should darken sky, 0x2: is dragon bar (used to play end music), 0x04: create fog (previously was also controlled by 0x02). |
| 1: remove | _no fields_ | _no fields_ | Removes this boss bar. |
| 2: update health | Health | [Float](https://wiki.vg/Protocol#Type:Float) | _as above_ |
| 3: update title | Title | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| 4: update style | Color | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Color ID (see below). |
| Dividers | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | _as above_ |
| 5: update flags | Flags | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | _as above_ |

| ID | Color |
| --- | --- |
| 0 | Pink |
| 1 | Blue |
| 2 | Red |
| 3 | Green |
| 4 | Yellow |
| 5 | Purple |
| 6 | White |

| ID | Type of division |
| --- | --- |
| 0 | No division |
| 1 | 6 notches |
| 2 | 10 notches |
| 3 | 12 notches |
| 4 | 20 notches |

#### Change Difficulty

Changes the difficulty setting in the client's option menu

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0B | Play | Client | Difficulty | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | 0: peaceful, 1: easy, 2: normal, 3: hard. |
| Difficulty locked? | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Chunk Batch Finished

Marks the end of a chunk batch. The Notchian client marks the time it receives this packet and calculates the elapsed duration since the [beginning of the chunk batch](https://wiki.vg/Protocol#Chunk_Batch_Start). The server uses this duration and the batch size received in this packet to estimate the number of milliseconds elapsed per chunk received. This value is then used to calculate the desired number of chunks per tick through the formula `25 / millisPerChunk`, which is reported to the server through [Chunk Batch Received](https://wiki.vg/Protocol#Chunk_Batch_Received). This likely uses `25` instead of the normal tick duration of `50` so chunk processing will only use half of the client's and network's bandwidth.

The Notchian client uses the samples from the latest 15 batches to estimate the milliseconds per chunk number.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0C | Play | Client | Batch size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of chunks. |

#### Chunk Batch Start

Marks the start of a chunk batch. The Notchian client marks and stores the time it receives this packet.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0D | Play | Client | _no fields_ |

#### Chunk Biomes

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0E | Play | Client |
| Number of chunks | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array |
| Chunk biome data | Chunk Z | [Array](https://wiki.vg/Protocol#Type:Array) | [Int](https://wiki.vg/Protocol#Type:Int) | Chunk coordinate (block coordinate divided by 16, rounded down) |
| Chunk X | [Int](https://wiki.vg/Protocol#Type:Int) | Chunk coordinate (block coordinate divided by 16, rounded down) |
| Size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Size of Data in bytes |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | Chunk [data structure](https://wiki.vg/Chunk_Format#Data_structure "Chunk Format"), with [sections](https://wiki.vg/Chunk_Format#Chunk_Section "Chunk Format") containing only the `Biomes` field |

Note: The order of X and Z is inverted, because the client reads them as one big-endian [Long](https://wiki.vg/Protocol#Type:Long), with Z being the upper 32 bits.

#### Clear Titles

Clear the client's current title information, with the option to also reset it.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0F | Play | Client | Reset | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Command Suggestions Response

The server responds with a list of auto-completions of the last word sent to it. In the case of regular chat, this is a player username. Command names and parameters are also supported. The client sorts these alphabetically before listing them.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x10 | Play | Client |
| ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Transaction ID. |
| Start | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Start of the text to replace. |
| Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the text to replace. |
| Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Matches | Match | [Array](https://wiki.vg/Protocol#Type:Array) | [String](https://wiki.vg/Protocol#Type:String) (32767) | One eligible value to insert, note that each command is sent separately instead of in a single string, hence the need for Count. Note that for instance this doesn't include a leading `/` on commands. |
| Has tooltip | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the following is present. |
| Tooltip | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Tooltip to display; only present if previous boolean is true. |

#### Commands

Lists all of the commands on the server, and how they are parsed.

This is a directed graph, with one root node. Each redirect or child node must refer only to nodes that have already been declared.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x11 | Play | Client | Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Nodes | [Array](https://wiki.vg/Protocol#Type:Array) of [Node](https://wiki.vg/Command_Data "Command Data") | An array of nodes. |
| Root index | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Index of the `root` node in the previous array. |

For more information on this packet, see the [Command Data](https://wiki.vg/Command_Data "Command Data") article.

#### Close Container

This packet is sent from the server to the client when a window is forcibly closed, such as when a chest is destroyed while it's open. The notchian client disregards the provided window ID and closes any active window.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x12 | Play | Client | Window ID | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | This is the ID of the window that was closed. 0 for inventory. |

#### Set Container Content

[![](https://wiki.vg/images/1/13/Inventory-slots.png)](https://wiki.vg/File:Inventory-slots.png)

Replaces the contents of a container window. Sent by the server upon initialization of a container window or the player's inventory, and in response to state ID mismatches (see [#Click Container](https://wiki.vg/Protocol#Click_Container)).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x13 | Play | Client | Window ID | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | The ID of window which items are being sent for. 0 for player inventory. The client ignores any packets targeting a Window ID other than the current one. However, an exception is made for the player inventory, which may be targeted at any time. (The Notchian server does not appear to utilize this special case.) |
| State ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | A server-managed sequence number used to avoid desynchronization; see [#Click Container](https://wiki.vg/Protocol#Click_Container). |
| Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Slot Data | [Array](https://wiki.vg/Protocol#Type:Array) of [Slot](https://wiki.vg/Slot_Data "Slot Data") |
| Carried Item | [Slot](https://wiki.vg/Protocol#Type:Slot) | Item being dragged with the mouse. |

See [inventory windows](https://wiki.vg/Inventory#Windows "Inventory") for further information about how slots are indexed. Use [Open Screen](https://wiki.vg/Protocol#Open_Screen) to open the container on the client.

#### Set Container Property

This packet is used to inform the client that part of a GUI window should be updated.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x14 | Play | Client | Window ID | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) |  |
| Property | [Short](https://wiki.vg/Protocol#Type:Short) | The property to be updated, see below. |
| Value | [Short](https://wiki.vg/Protocol#Type:Short) | The new value for the property, see below. |

The meaning of the Property field depends on the type of the window. The following table shows the known combinations of window type and property, and how the value is to be interpreted.

| Window type | Property | Value |
| --- | --- | --- |
| Furnace | 0: Fire icon (fuel left) | counting from fuel burn time down to 0 (in-game ticks) |
| 1: Maximum fuel burn time | fuel burn time or 0 (in-game ticks) |
| 2: Progress arrow | counting from 0 to maximum progress (in-game ticks) |
| 3: Maximum progress | always 200 on the notchian server |
| Enchantment Table | 0: Level requirement for top enchantment slot | The enchantment's xp level requirement |
| 1: Level requirement for middle enchantment slot |
| 2: Level requirement for bottom enchantment slot |
| 3: The enchantment seed | Used for drawing the enchantment names (in [SGA](http://en.wikipedia.org/wiki/Standard_Galactic_Alphabet "wikipedia:Standard Galactic Alphabet")) clientside. The same seed _is_ used to calculate enchantments, but some of the data isn't sent to the client to prevent easily guessing the entire list (the seed value here is the regular seed bitwise and `0xFFFFFFF0`). |
| 4: Enchantment ID shown on mouse hover over top enchantment slot | The enchantment id (set to -1 to hide it), see below for values |
| 5: Enchantment ID shown on mouse hover over middle enchantment slot |
| 6: Enchantment ID shown on mouse hover over bottom enchantment slot |
| 7: Enchantment level shown on mouse hover over the top slot | The enchantment level (1 = I, 2 = II, 6 = VI, etc.), or -1 if no enchant |
| 8: Enchantment level shown on mouse hover over the middle slot |
| 9: Enchantment level shown on mouse hover over the bottom slot |
| Beacon | 0: Power level | 0-4, controls what effect buttons are enabled |
| 1: First potion effect | [Potion effect ID](https://minecraft.wiki/w/Data_values%23Status_effects) for the first effect, or -1 if no effect |
| 2: Second potion effect | [Potion effect ID](https://minecraft.wiki/w/Data_values%23Status_effects) for the second effect, or -1 if no effect |
| Anvil | 0: Repair cost | The repair's cost in xp levels |
| Brewing Stand | 0: Brew time | 0 – 400, with 400 making the arrow empty, and 0 making the arrow full |
| 1: Fuel time | 0 - 20, with 0 making the arrow empty, and 20 making the arrow full |
| Stonecutter | 0: Selected recipe | The index of the selected recipe. -1 means none is selected. |
| Loom | 0: Selected pattern | The index of the selected pattern. 0 means none is selected, 0 is also the internal id of the "base" pattern. |
| Lectern | 0: Page number | The current page number, starting from 0. |

For an enchanting table, the following numerical IDs are used:

| Numerical ID | Enchantment ID | Enchantment Name |
| --- | --- | --- |
| 0 | minecraft:protection | Protection |
| 1 | minecraft:fire\_protection | Fire Protection |
| 2 | minecraft:feather\_falling | Feather Falling |
| 3 | minecraft:blast\_protection | Blast Protection |
| 4 | minecraft:projectile\_protection | Projectile Protection |
| 5 | minecraft:respiration | Respiration |
| 6 | minecraft:aqua\_affinity | Aqua Affinity |
| 7 | minecraft:thorns | Thorns |
| 8 | minecraft:depth\_strider | Depth Strider |
| 9 | minecraft:frost\_walker | Frost Walker |
| 10 | minecraft:binding\_curse | Curse of Binding |
| 11 | minecraft:soul\_speed | Soul Speed |
| 12 | minecraft:swift\_sneak | Swift Sneak |
| 13 | minecraft:sharpness | Sharpness |
| 14 | minecraft:smite | Smite |
| 15 | minecraft:bane\_of\_arthropods | Bane of Arthropods |
| 16 | minecraft:knockback | Knockback |
| 17 | minecraft:fire\_aspect | Fire Aspect |
| 18 | minecraft:looting | Looting |
| 19 | minecraft:sweeping\_edge | Sweeping Edge |
| 20 | minecraft:efficiency | Efficiency |
| 21 | minecraft:silk\_touch | Silk Touch |
| 22 | minecraft:unbreaking | Unbreaking |
| 23 | minecraft:fortune | Fortune |
| 24 | minecraft:power | Power |
| 25 | minecraft:punch | Punch |
| 26 | minecraft:flame | Flame |
| 27 | minecraft:infinity | Infinity |
| 28 | minecraft:luck\_of\_the\_sea | Luck of the Sea |
| 29 | minecraft:lure | Lure |
| 30 | minecraft:loyalty | Loyalty |
| 31 | minecraft:impaling | Impaling |
| 32 | minecraft:riptide | Riptide |
| 33 | minecraft:channeling | Channeling |
| 34 | minecraft:multishot | Multishot |
| 35 | minecraft:quick\_charge | Quick Charge |
| 36 | minecraft:piercing | Piercing |
| 37 | minecraft:density | Density |
| 38 | minecraft:breach | Breach |
| 39 | minecraft:wind\_burst | Wind Burst |
| 40 | minecraft:mending | Mending |
| 41 | minecraft:vanishing\_curse | Curse of Vanishing |

#### Set Container Slot

Sent by the server when an item in a slot (in a window) is added/removed.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x15 | Play | Client | Window ID | [Byte](https://wiki.vg/Protocol#Type:Byte) | The window which is being updated. 0 for player inventory. The client ignores any packets targeting a Window ID other than the current one; see below for exceptions. |
| State ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | A server-managed sequence number used to avoid desynchronization; see [#Click Container](https://wiki.vg/Protocol#Click_Container). |
| Slot | [Short](https://wiki.vg/Protocol#Type:Short) | The slot that should be updated. |
| Slot Data | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |

If Window ID is 0, the hotbar and offhand slots (slots 36 through 45) may be updated even when a different container window is open. (The Notchian server does not appear to utilize this special case.) Updates are also restricted to those slots when the player is looking at a creative inventory tab other than the survival inventory. (The Notchian server does _not_ handle this restriction in any way, leading to [MC-242392](https://bugs.mojang.com/browse/MC-242392).)

If Window ID is -1, the item being dragged with the mouse is set. In this case, State ID and Slot are ignored.

If Window ID is -2, any slot in the player's inventory can be updated irrespective of the current container window. In this case, State ID is ignored, and the Notchian server uses a bogus value of 0. Used by the Notchian server to implement the [#Pick Item](https://wiki.vg/Protocol#Pick_Item) functionality.

When a container window is open, the server never sends updates targeting Window ID 0—all of the [window types](https://wiki.vg/Inventory "Inventory") include slots for the player inventory. The client must automatically apply changes targeting the inventory portion of a container window to the main inventory; the server does not resend them for ID 0 when the window is closed. However, since the armor and offhand slots are only present on ID 0, updates to those slots occurring while a window is open must be deferred by the server until the window's closure.

#### Cookie Request (play)

Requests a cookie that was previously stored.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x16 | Play | Client | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |

#### Set Cooldown

Applies a cooldown period to all items with the given type. Used by the Notchian server with enderpearls. This packet should be sent when the cooldown starts and also when the cooldown ends (to compensate for lag), although the client will end the cooldown automatically. Can be applied to any item, note that interactions still get sent to the server with the item but the client does not play the animation nor attempt to predict results (i.e block placing).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x17 | Play | Client | Item ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Numeric ID of the item to apply a cooldown to. |
| Cooldown Ticks | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of ticks to apply a cooldown for, or 0 to clear the cooldown. |

#### Chat Suggestions

Unused by the Notchian server. Likely provided for custom servers to send chat message completions to clients.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x18 | Play | Client | Action | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: Add, 1: Remove, 2: Set |
| Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Entries | [Array](https://wiki.vg/Protocol#Type:Array) of [String](https://wiki.vg/Protocol#Type:String) (32767) |  |

#### Clientbound Plugin Message (play)

_Main article: [Plugin channels](https://wiki.vg/Plugin_channels "Plugin channels")_

Mods and plugins can use this to send their data. Minecraft itself uses several [plugin channels](https://wiki.vg/Plugin_channel "Plugin channel"). These internal channels are in the `minecraft` namespace.

More information on how it works on [Dinnerbone's blog](https://dinnerbone.com/blog/2012/01/13/minecraft-plugin-channels-messaging/). More documentation about internal and popular registered channels are [here](https://wiki.vg/Plugin_channel "Plugin channel").

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x19 | Play | Client | Channel | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the [plugin channel](https://wiki.vg/Plugin_channel "Plugin channel") used to send the data. |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (1048576) | Any data. The length of this array must be inferred from the packet length. |

In Notchian client, the maximum data length is 1048576 bytes.

#### Damage Event

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1A | Play | Client |
| Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of the entity taking damage |
| Source Type ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The type of damage in the `minecraft:damage_type` registry, defined by the [Registry Data](https://wiki.vg/Protocol#Registry_Data "Protocol") packet. |
| Source Cause ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID + 1 of the entity responsible for the damage, if present. If not present, the value is 0 |
| Source Direct ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID + 1 of the entity that directly dealt the damage, if present. If not present, the value is 0. If this field is present:
-   and damage was dealt indirectly, such as by the use of a projectile, this field will contain the ID of such projectile;
-   and damage was dealt dirctly, such as by manually attacking, this field will contain the same value as Source Cause ID.

 |
| Has Source Position | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Indicates the presence of the three following fields.

The Notchian server sends the Source Position when the damage was dealt by the /damage command and a position was specified

 |
| Source Position X | [Optional](https://wiki.vg/Protocol#Type:Optional) [Double](https://wiki.vg/Protocol#Type:Double) | Only present if Has Source Position is true |
| Source Position Y | [Optional](https://wiki.vg/Protocol#Type:Optional) [Double](https://wiki.vg/Protocol#Type:Double) | Only present if Has Source Position is true |
| Source Position Z | [Optional](https://wiki.vg/Protocol#Type:Optional) [Double](https://wiki.vg/Protocol#Type:Double) | Only present if Has Source Position is true |

#### Debug Sample

Sample data that is sent periodically after the client has subscribed with [Debug Sample Subscription](https://wiki.vg/Protocol#Debug_Sample_Subscription).

The Notchian server only sends debug samples to players that are server operators.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1B | Play | Client | Sample Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The length of the following array. |
| Sample | [Long Array](https://wiki.vg/Protocol#Type:Long_Array) | Array of type-dependent samples. |
| Sample Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | See below. |

Types:

| Id | Name | Description |
| --- | --- | --- |
| 0 | Tick time | Four different tick-related metrics, each one represented by one long on the array.
They are measured in nano-seconds, and are as follows:

-   Full tick time: Aggregate of the three times below;
-   Server tick time: Main server tick logic;
-   Tasks time: Tasks scheduled to execute after the main logic;
-   Idle time: Time idling to complete the full 50ms tick cycle.

 |

#### Delete Message

Removes a message from the client's chat. This only works for messages with signatures, system messages cannot be deleted with this packet.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1C | Play | Client | Message ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The message Id + 1, used for validating message signature. The next field is present only when value of this field is equal to 0. |
| Signature | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (256) | The previous message's signature. Always 256 bytes and not length-prefixed. |

#### Disconnect (play)

Sent by the server before it disconnects a client. The client assumes that the server has already closed the connection by the time the packet arrives.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1D | Play | Client | Reason | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Displayed to the client when the connection terminates. |

#### Disguised Chat Message

Sends the client a chat message, but without any message signing information.

The Notchian server uses this packet when the console is communicating with players through commands, such as `/say`, `/tell`, `/me`, among others.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1E | Play | Client | Message | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | This is used as the `content` parameter when formatting the message on the client. |
| Chat Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The type of chat in the `minecraft:chat_type` registry, defined by the [Registry Data](https://wiki.vg/Protocol#Registry_Data "Protocol") packet. |
| Sender Name | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The name of the one sending the message, usually the sender's display name.
This is used as the `sender` parameter when formatting the message on the client.

 |
| Has Target Name | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if target name is present. |
| Target Name | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The name of the one receiving the message, usually the receiver's display name. Only present if previous boolean is true.

This is used as the `target` parameter when formatting the message on the client.

 |

#### Entity Event

Entity statuses generally trigger an animation for an entity. The available statuses vary by the entity's type (and are available to subclasses of that type as well).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1F | Play | Client | Entity ID | [Int](https://wiki.vg/Protocol#Type:Int) |  |
| Entity Status | [Byte](https://wiki.vg/Protocol#Type:Byte) [Enum](https://wiki.vg/Protocol#Type:Enum) | See [Entity statuses](https://wiki.vg/Entity_statuses "Entity statuses") for a list of which statuses are valid for each type of entity. |

#### Explosion

Sent when an explosion occurs (creepers, TNT, and ghast fireballs).

Each block in Records is set to air. Coordinates for each axis in record is int(X) + record.x

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x20 | Play | Client | X | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Strength | [Float](https://wiki.vg/Protocol#Type:Float) | If the strength is greater or equal to 2.0, or the block interaction is not 0 (keep), large explosion particles are used. Otherwise, small explosion particles are used. |
| Record Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Records | [Array](https://wiki.vg/Protocol#Type:Array) of ([Byte](https://wiki.vg/Protocol#Type:Byte), [Byte](https://wiki.vg/Protocol#Type:Byte), [Byte](https://wiki.vg/Protocol#Type:Byte)) | Each record is 3 signed bytes long; the 3 bytes are the XYZ (respectively) signed offsets of affected blocks. |
| Player Motion X | [Float](https://wiki.vg/Protocol#Type:Float) | X velocity of the player being pushed by the explosion. |
| Player Motion Y | [Float](https://wiki.vg/Protocol#Type:Float) | Y velocity of the player being pushed by the explosion. |
| Player Motion Z | [Float](https://wiki.vg/Protocol#Type:Float) | Z velocity of the player being pushed by the explosion. |
| Block Interaction | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0 = keep, 1 = destroy, 2 = destroy\_with\_decay, 3 = trigger\_block. |
| Small Explosion Particle ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The particle ID listed in [Particles](https://wiki.vg/Particles "Particles"). |
| Small Explosion Particle Data | Varies | Particle data as specified in [Particles](https://wiki.vg/Particles "Particles"). |
| Large Explosion Particle ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The particle ID listed in [Particles](https://wiki.vg/Particles "Particles"). |
| Large Explosion Particle Data | Varies | Particle data as specified in [Particles](https://wiki.vg/Particles "Particles"). |
| Explosion Sound | Sound Name | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The name of the sound played. |
| Has Fixed Range | [Optional](https://wiki.vg/Protocol#Type:Optional) [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether is has fixed range. |
| Range | [Optional](https://wiki.vg/Protocol#Type:Optional) [Float](https://wiki.vg/Protocol#Type:Float) | The fixed range of the sound. Only present if previous boolean is true. |

#### Unload Chunk

Tells the client to unload a chunk column.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x21 | Play | Client | Chunk Z | [Int](https://wiki.vg/Protocol#Type:Int) | Block coordinate divided by 16, rounded down. |
| Chunk X | [Int](https://wiki.vg/Protocol#Type:Int) | Block coordinate divided by 16, rounded down. |

Note: The order is inverted, because the client reads this packet as one big-endian [Long](https://wiki.vg/Protocol#Type:Long), with Z being the upper 32 bits.

It is legal to send this packet even if the given chunk is not currently loaded.

#### Game Event

Used for a wide variety of game events, from weather to bed use to game mode to demo messages.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x22 | Play | Client | Event | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | See below. |
| Value | [Float](https://wiki.vg/Protocol#Type:Float) | Depends on Event. |

_Events_:

| Event | Effect | Value |
| --- | --- | --- |
| 0 | No respawn block available | Note: Displays message 'block.minecraft.spawn.not\_valid' (You have no home bed or charged respawn anchor, or it was obstructed) to the player. |
| 1 | Begin raining |  |
| 2 | End raining |  |
| 3 | Change game mode | 0: Survival, 1: Creative, 2: Adventure, 3: Spectator. |
| 4 | Win game | 0: Just respawn player.  
1: Roll the credits and respawn player.  
Note that 1 is only sent by notchian server when player has not yet achieved advancement "The end?", else 0 is sent. |
| 5 | Demo event | 0: Show welcome to demo screen.  
101: Tell movement controls.  
102: Tell jump control.  
103: Tell inventory control.  
104: Tell that the demo is over and print a message about how to take a screenshot. |
| 6 | Arrow hit player | Note: Sent when any player is struck by an arrow. |
| 7 | Rain level change | Note: Seems to change both sky color and lighting.  
Rain level ranging from 0 to 1. |
| 8 | Thunder level change | Note: Seems to change both sky color and lighting (same as Rain level change, but doesn't start rain). It also requires rain to render by notchian client.  
Thunder level ranging from 0 to 1. |
| 9 | Play pufferfish sting sound |
| 10 | Play elder guardian mob appearance (effect and sound) |  |
| 11 | Enable respawn screen | 0: Enable respawn screen.  
1: Immediately respawn (sent when the `doImmediateRespawn` gamerule changes). |
| 12 | Limited crafting | 0: Disable limited crafting.  
1: Enable limited crafting (sent when the `doLimitedCrafting` gamerule changes). |
| 13 | Start waiting for level chunks | Instructs the client to begin the waiting process for the level chunks.  
Sent by the server after the level is cleared on the client and is being re-sent (either during the first, or subsequent reconfigurations). |

#### Open Horse Screen

This packet is used exclusively for opening the horse GUI. [Open Screen](https://wiki.vg/Protocol#Open_Screen) is used for all other GUIs. The client will not open the inventory if the Entity ID does not point to an horse-like animal.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x23 | Play | Client | Window ID | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) |  |
| Slot count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Entity ID | [Int](https://wiki.vg/Protocol#Type:Int) |  |

#### Hurt Animation

Plays a bobbing animation for the entity receiving damage.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x24 | Play | Client |
| Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of the entity taking damage |
| Yaw | [Float](https://wiki.vg/Protocol#Type:Float) | The direction the damage is coming from in relation to the entity |

#### Initialize World Border

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x25 | Play | Client | X | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Old Diameter | [Double](https://wiki.vg/Protocol#Type:Double) | Current length of a single side of the world border, in meters. |
| New Diameter | [Double](https://wiki.vg/Protocol#Type:Double) | Target length of a single side of the world border, in meters. |
| Speed | [VarLong](https://wiki.vg/Protocol#Type:VarLong) | Number of real-time _milli_seconds until New Diameter is reached. It appears that Notchian server does not sync world border speed to game ticks, so it gets out of sync with server lag. If the world border is not moving, this is set to 0. |
| Portal Teleport Boundary | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Resulting coordinates from a portal teleport are limited to ±value. Usually 29999984. |
| Warning Blocks | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | In meters. |
| Warning Time | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | In seconds as set by `/worldborder warning time`. |

The Notchian client determines how solid to display the warning by comparing to whichever is higher, the warning distance or whichever is lower, the distance from the current diameter to the target diameter or the place the border will be after warningTime seconds. In pseudocode:

```
<span></span><span>distance</span> <span>=</span> <span>max</span><span>(</span><span>min</span><span>(</span><span>resizeSpeed</span> <span>*</span> <span>1000</span> <span>*</span> <span>warningTime</span><span>,</span> <span>abs</span><span>(</span><span>targetDiameter</span> <span>-</span> <span>currentDiameter</span><span>)),</span> <span>warningDistance</span><span>);</span>
<span>if</span> <span>(</span><span>playerDistance</span> <span>&lt;</span> <span>distance</span><span>)</span> <span>{</span>
    <span>warning</span> <span>=</span> <span>1.0</span> <span>-</span> <span>playerDistance</span> <span>/</span> <span>distance</span><span>;</span>
<span>}</span> <span>else</span> <span>{</span>
    <span>warning</span> <span>=</span> <span>0.0</span><span>;</span>
<span>}</span>
```

#### Clientbound Keep Alive (play)

The server will frequently send out a keep-alive, each containing a random ID. The client must respond with the same payload (see [Serverbound Keep Alive](https://wiki.vg/Protocol#Serverbound_Keep_Alive_.28play.29)). If the client does not respond to a Keep Alive packet within 15 seconds after it was sent, the server kicks the client. Vice versa, if the server does not send any keep-alives for 20 seconds, the client will disconnect and yields a "Timed out" exception.

The Notchian server uses a system-dependent time in milliseconds to generate the keep alive ID value.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x26 | Play | Client | Keep Alive ID | [Long](https://wiki.vg/Protocol#Type:Long) |  |

#### Chunk Data and Update Light

_Main article: [Chunk Format](https://wiki.vg/Chunk_Format "Chunk Format")_

_See also: [#Unload Chunk](https://wiki.vg/Protocol#Unload_Chunk)_

Sent when a chunk comes into the client's view distance, specifying its terrain, lighting and block entities.

The chunk must be within the view area previously specified with [Set Center Chunk](https://wiki.vg/Protocol#Set_Center_Chunk); see that packet for details.

It is not strictly necessary to send all block entities in this packet; it is still legal to send them with [Block Entity Data](https://wiki.vg/Protocol#Block_Entity_Data) later.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x27 | Play | Client | Chunk X | [Int](https://wiki.vg/Protocol#Type:Int) | Chunk coordinate (block coordinate divided by 16, rounded down) |
| Chunk Z | [Int](https://wiki.vg/Protocol#Type:Int) | Chunk coordinate (block coordinate divided by 16, rounded down) |
| Heightmaps | [NBT](https://wiki.vg/NBT "NBT") | See [Chunk Format#Heightmaps structure](https://wiki.vg/Chunk_Format#Heightmaps_structure "Chunk Format") |
| Size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Size of Data in bytes |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | See [Chunk Format#Data structure](https://wiki.vg/Chunk_Format#Data_structure "Chunk Format") |
| Number of block entities | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array |
| Block Entity | Packed XZ | [Array](https://wiki.vg/Protocol#Type:Array) | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | The packed section coordinates are relative to the chunk they are in. Values 0-15 are valid.
```
packed_xz = ((blockX &amp; 15) &lt;&lt; 4) | (blockZ &amp; 15) // encode
x = packed_xz &gt;&gt; 4, z = packed_xz &amp; 15 // decode
```

 |
| Y | [Short](https://wiki.vg/Protocol#Type:Short) | The height relative to the world |
| Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The type of block entity |
| Data | [NBT](https://wiki.vg/NBT "NBT") | The block entity's data, without the X, Y, and Z values |
| Sky Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has data in the Sky Light array below. The least significant bit is for blocks 16 blocks to 1 block below the min world height (one section below the world), while the most significant bit covers blocks 1 to 16 blocks above the max world height (one section above the world). |
| Block Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has data in the Block Light array below. The order of bits is the same as in Sky Light Mask. |
| Empty Sky Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has all zeros for its Sky Light data. The order of bits is the same as in Sky Light Mask. |
| Empty Block Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has all zeros for its Block Light data. The order of bits is the same as in Sky Light Mask. |
| Sky Light array count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of entries in the following array; should match the number of bits set in Sky Light Mask |
| Sky Light arrays | Length | [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following array in bytes (always 2048) |
| Sky Light array | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (2048) | There is 1 array for each bit set to true in the sky light mask, starting with the lowest value. Half a byte per light value. Indexed `((y<<8) | (z<<4) | x) / 2` If there's a remainder, masked 0xF0 else 0x0F. |
| Block Light array count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of entries in the following array; should match the number of bits set in Block Light Mask |
| Block Light arrays | Length | [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following array in bytes (always 2048) |
| Block Light array | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (2048) | There is 1 array for each bit set to true in the block light mask, starting with the lowest value. Half a byte per light value. Indexed `((y<<8) | (z<<4) | x) / 2` If there's a remainder, masked 0xF0 else 0x0F. |

Unlike the [Update Light](https://wiki.vg/Protocol#Update_Light) packet which uses the same format, setting the bit corresponding to a section to 0 in both of the block light or sky light masks does not appear to be useful, and the results in testing have been highly inconsistent.

#### World Event

<table><tbody><tr><td rowspan="2"><a href="https://wiki.vg/File:Huh.png"><img alt="Huh.png" src="https://wiki.vg/images/4/4f/Huh.png" decoding="async" width="40" height="40"></a></td><td>The following information needs to be added to this page:</td></tr><tr><td><b>The events listed below are not up-to-date with the latest release version, being either improperly documented or missing from the list altogether.</b></td></tr></tbody></table>

Sent when a client is to play a sound or particle effect.

By default, the Minecraft client adjusts the volume of sound effects based on distance. The final boolean field is used to disable this, and instead the effect is played from 2 blocks away in the correct direction. Currently this is only used for effect 1023 (wither spawn), effect 1028 (enderdragon death), and effect 1038 (end portal opening); it is ignored on other effects.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x28 | Play | Client | Event | [Int](https://wiki.vg/Protocol#Type:Int) | The event, see below. |
| Location | [Position](https://wiki.vg/Protocol#Type:Position) | The location of the event. |
| Data | [Int](https://wiki.vg/Protocol#Type:Int) | Extra data for certain events, see below. |
| Disable Relative Volume | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | See above. |

Events:

| ID | Name | Data |
| --- | --- | --- |
| Sound |
| 1000 | Dispenser dispenses |  |
| 1001 | Dispenser fails to dispense |  |
| 1002 | Dispenser shoots |  |
| 1003 | Ender eye launched |  |
| 1004 | Firework shot |  |
| 1009 | Fire extinguished |  |
| 1010 | Play record | An ID in the `minecraft:item` registry, corresponding to a [record item](https://minecraft.wiki/w/Music_Disc). If the ID doesn't correspond to a record, the packet is ignored. Any record already being played at the given location is overwritten. See [Data Generators](https://wiki.vg/Data_Generators "Data Generators") for information on item IDs. |
| 1011 | Stop record |  |
| 1015 | Ghast warns |  |
| 1016 | Ghast shoots |  |
| 1017 | Enderdragon shoots |  |
| 1018 | Blaze shoots |  |
| 1019 | Zombie attacks wood door |  |
| 1020 | Zombie attacks iron door |  |
| 1021 | Zombie breaks wood door |  |
| 1022 | Wither breaks block |  |
| 1023 | Wither spawned |  |
| 1024 | Wither shoots |  |
| 1025 | Bat takes off |  |
| 1026 | Zombie infects |  |
| 1027 | Zombie villager converted |  |
| 1028 | Ender dragon death |  |
| 1029 | Anvil destroyed |  |
| 1030 | Anvil used |  |
| 1031 | Anvil landed |  |
| 1032 | Portal travel |  |
| 1033 | Chorus flower grown |  |
| 1034 | Chorus flower died |  |
| 1035 | Brewing stand brewed |  |
| 1038 | End portal created in overworld |  |
| 1039 | Phantom bites |  |
| 1040 | Zombie converts to drowned |  |
| 1041 | Husk converts to zombie by drowning |  |
| 1042 | Grindstone used |  |
| 1043 | Book page turned |  |
| 1044 | Smithing table used |  |
| 1045 | Pointed dripstone landing |  |
| 1046 | Lava dripping on cauldron from dripstone |  |
| 1047 | Water dripping on cauldron from dripstone |  |
| 1048 | Skeleton converts to stray |  |
| 1049 | Crafter successfully crafts item |  |
| 1050 | Crafter fails to craft item |  |
| Particle |
| 1500 | Composter composts |  |
| 1501 | Lava converts block (either water to stone, or removes existing blocks such as torches) |  |
| 1502 | Redstone torch burns out |  |
| 1503 | Ender eye placed |  |
| 1504 | Fluid drips from dripstone |  |
| 1505 | Bonemeal particles | How many particles to spawn. |
| 2000 | Spawns 10 smoke particles, e.g. from a fire | Direction, see below. |
| 2001 | Block break + block break sound | Block state ID (see [Chunk Format#Block state registry](https://wiki.vg/Chunk_Format#Block_state_registry "Chunk Format")). |
| 2002 | Splash potion. Particle effect + glass break sound. | RGB color as an integer (e.g. 8364543 for #7FA1FF). |
| 2003 | Eye of Ender entity break animation — particles and sound |  |
| 2004 | Mob spawn particle effect: smoke + flames |  |
| 2006 | Dragon breath |  |
| 2007 | Instant splash potion. Particle effect + glass break sound. | RGB color as an integer (e.g. 8364543 for #7FA1FF). |
| 2008 | Ender dragon destroys block |  |
| 2009 | Wet sponge vaporizes in nether |  |
| 3000 | End gateway spawn |  |
| 3001 | Enderdragon growl |  |
| 3002 | Electric spark |  |
| 3003 | Copper apply wax |  |
| 3004 | Copper remove wax |  |
| 3005 | Copper scrape oxidation |  |

Smoke directions:

| ID | Direction |
| --- | --- |
| 0 | Down |
| 1 | Up |
| 2 | North |
| 3 | South |
| 4 | West |
| 5 | East |

#### Particle

Displays the named particle

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x29 | Play | Client | Particle ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The particle ID listed in [Particles](https://wiki.vg/Particles "Particles"). |
| Long Distance | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, particle distance increases from 256 to 65536. |
| X | [Double](https://wiki.vg/Protocol#Type:Double) | X position of the particle. |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) | Y position of the particle. |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) | Z position of the particle. |
| Offset X | [Float](https://wiki.vg/Protocol#Type:Float) | This is added to the X position after being multiplied by `random.nextGaussian()`. |
| Offset Y | [Float](https://wiki.vg/Protocol#Type:Float) | This is added to the Y position after being multiplied by `random.nextGaussian()`. |
| Offset Z | [Float](https://wiki.vg/Protocol#Type:Float) | This is added to the Z position after being multiplied by `random.nextGaussian()`. |
| Max Speed | [Float](https://wiki.vg/Protocol#Type:Float) |  |
| Particle Count | [Int](https://wiki.vg/Protocol#Type:Int) | The number of particles to create. |
| Data | Varies | Particle data as specified in [Particles](https://wiki.vg/Particles "Particles"). |

#### Update Light

Updates light levels for a chunk. See [Light](https://minecraft.wiki/w/Light) for information on how lighting works in Minecraft.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2A | Play | Client | Chunk X | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Chunk coordinate (block coordinate divided by 16, rounded down) |
| Chunk Z | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Chunk coordinate (block coordinate divided by 16, rounded down) |
| Sky Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has data in the Sky Light array below. The least significant bit is for blocks 16 blocks to 1 block below the min world height (one section below the world), while the most significant bit covers blocks 1 to 16 blocks above the max world height (one section above the world). |
| Block Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has data in the Block Light array below. The order of bits is the same as in Sky Light Mask. |
| Empty Sky Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has all zeros for its Sky Light data. The order of bits is the same as in Sky Light Mask. |
| Empty Block Light Mask | [BitSet](https://wiki.vg/Protocol#Type:BitSet) | BitSet containing bits for each section in the world + 2. Each set bit indicates that the corresponding 16×16×16 chunk section has all zeros for its Block Light data. The order of bits is the same as in Sky Light Mask. |
| Sky Light array count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of entries in the following array; should match the number of bits set in Sky Light Mask |
| Sky Light arrays | Length | [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following array in bytes (always 2048) |
| Sky Light array | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (2048) | There is 1 array for each bit set to true in the sky light mask, starting with the lowest value. Half a byte per light value. |
| Block Light array count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of entries in the following array; should match the number of bits set in Block Light Mask |
| Block Light arrays | Length | [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following array in bytes (always 2048) |
| Block Light array | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (2048) | There is 1 array for each bit set to true in the block light mask, starting with the lowest value. Half a byte per light value. |

A bit will never be set in both the block light mask and the empty block light mask, though it may be present in neither of them (if the block light does not need to be updated for the corresponding chunk section). The same applies to the sky light mask and the empty sky light mask.

#### Login (play)

<table><tbody><tr><td rowspan="2"><a href="https://wiki.vg/File:Huh.png"><img alt="Huh.png" src="https://wiki.vg/images/4/4f/Huh.png" decoding="async" width="40" height="40"></a></td><td>The following information needs to be added to this page:</td></tr><tr><td><b>Although the number of portal cooldown ticks is included in this packet, the whole portal usage process is still dictated entirely by the server. What kind of effect does this value have on the client, if any?</b></td></tr></tbody></table>

See [Protocol Encryption](https://wiki.vg/Protocol_Encryption "Protocol Encryption") for information on logging in.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2B | Play | Client | Entity ID | [Int](https://wiki.vg/Protocol#Type:Int) | The player's Entity ID (EID). |
| Is hardcore | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Dimension Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Size of the following array. |
| Dimension Names | [Array](https://wiki.vg/Protocol#Type:Array) of [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Identifiers for all dimensions on the server. |
| Max Players | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Was once used by the client to draw the player list, but now is ignored. |
| View Distance | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Render distance (2-32). |
| Simulation Distance | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The distance that the client will process specific things, such as entities. |
| Reduced Debug Info | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, a Notchian client shows reduced information on the [debug screen](https://minecraft.wiki/w/Debug_screen). For servers in development, this should almost always be false. |
| Enable respawn screen | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Set to false when the doImmediateRespawn gamerule is true. |
| Do limited crafting | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether players can only craft recipes they have already unlocked. Currently unused by the client. |
| Dimension Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of the type of dimension in the `minecraft:dimension_type` registry, defined by the Registry Data packet. |
| Dimension Name | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the dimension being spawned into. |
| Hashed seed | [Long](https://wiki.vg/Protocol#Type:Long) | First 8 bytes of the SHA-256 hash of the world's seed. Used client side for biome noise |
| Game mode | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | 0: Survival, 1: Creative, 2: Adventure, 3: Spectator. |
| Previous Game mode | [Byte](https://wiki.vg/Protocol#Type:Byte) | \-1: Undefined (null), 0: Survival, 1: Creative, 2: Adventure, 3: Spectator. The previous game mode. Vanilla client uses this for the debug (F3 + N & F3 + F4) game mode switch. (More information needed) |
| Is Debug | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the world is a [debug mode](https://minecraft.wiki/w/Debug_mode) world; debug mode worlds cannot be modified and have predefined blocks. |
| Is Flat | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the world is a [superflat](https://minecraft.wiki/w/Superflat) world; flat worlds have different void fog and a horizon at y=0 instead of y=63. |
| Has death location | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the next two fields are present. |
| Death dimension name | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the dimension the player died in. |
| Death location | [Optional](https://wiki.vg/Protocol#Type:Optional) [Position](https://wiki.vg/Protocol#Type:Position) | The location that the player died at. |
| Portal cooldown | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The number of ticks until the player can use the portal again. |
| Enforces Secure Chat | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Map Data

Updates a rectangular area on a [map](https://minecraft.wiki/w/Map) item.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2C | Play | Client | Map ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Map ID of the map being modified |
| Scale | [Byte](https://wiki.vg/Protocol#Type:Byte) | From 0 for a fully zoomed-in map (1 block per pixel) to 4 for a fully zoomed-out map (16 blocks per pixel) |
| Locked | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the map has been locked in a cartography table |
| Has Icons | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Icon Count | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. Only present if previous Boolean is true. |
| Icon | Type | [Optional](https://wiki.vg/Protocol#Type:Optional) [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | See below |
| X | [Byte](https://wiki.vg/Protocol#Type:Byte) | Map coordinates: -128 for furthest left, +127 for furthest right |
| Z | [Byte](https://wiki.vg/Protocol#Type:Byte) | Map coordinates: -128 for highest, +127 for lowest |
| Direction | [Byte](https://wiki.vg/Protocol#Type:Byte) | 0-15 |
| Has Display Name | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Display Name | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Only present if previous Boolean is true |
| Color Patch | Columns | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Number of columns updated |
| Rows | [Optional](https://wiki.vg/Protocol#Type:Optional) [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Only if Columns is more than 0; number of rows updated |
| X | [Optional](https://wiki.vg/Protocol#Type:Optional) [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Only if Columns is more than 0; x offset of the westernmost column |
| Z | [Optional](https://wiki.vg/Protocol#Type:Optional) [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Only if Columns is more than 0; z offset of the northernmost row |
| Length | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Only if Columns is more than 0; length of the following array |
| Data | [Optional](https://wiki.vg/Protocol#Type:Optional) [Array](https://wiki.vg/Protocol#Type:Array) of [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Only if Columns is more than 0; see [Map item format](https://minecraft.wiki/w/Map_item_format) |

For icons, a direction of 0 is a vertical icon and increments by 22.5° (360/16).

Types are based off of rows and columns in `map_icons.png`:

| Icon type | Result |
| --- | --- |
| 0 | White arrow (players) |
| 1 | Green arrow (item frames) |
| 2 | Red arrow |
| 3 | Blue arrow |
| 4 | White cross |
| 5 | Red pointer |
| 6 | White circle (off-map players) |
| 7 | Small white circle (far-off-map players) |
| 8 | Mansion |
| 9 | Monument |
| 10 | White Banner |
| 11 | Orange Banner |
| 12 | Magenta Banner |
| 13 | Light Blue Banner |
| 14 | Yellow Banner |
| 15 | Lime Banner |
| 16 | Pink Banner |
| 17 | Gray Banner |
| 18 | Light Gray Banner |
| 19 | Cyan Banner |
| 20 | Purple Banner |
| 21 | Blue Banner |
| 22 | Brown Banner |
| 23 | Green Banner |
| 24 | Red Banner |
| 25 | Black Banner |
| 26 | Treasure marker |

#### Merchant Offers

The list of trades a villager NPC is offering.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2D | Play | Client | Window ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of the window that is open; this is an int rather than a byte. |
| Size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The number of trades in the following array. |
| Trades | Input item 1 | [Array](https://wiki.vg/Protocol#Type:Array) | [Slot](https://wiki.vg/Protocol#Type:Slot) | The first item the player has to supply for this villager trade. The count of the item stack is the default "price" of this trade. |
| Output item | [Slot](https://wiki.vg/Protocol#Type:Slot) | The item the player will receive from this villager trade. |
| Input item 2 | [Slot](https://wiki.vg/Protocol#Type:Slot) | The second item the player has to supply for this villager trade. May be an empty slot. |
| Trade disabled | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the trade is disabled; false if the trade is enabled. |
| Number of trade uses | [Int](https://wiki.vg/Protocol#Type:Int) | Number of times the trade has been used so far. If equal to the maximum number of trades, the client will display a red X. |
| Maximum number of trade uses | [Int](https://wiki.vg/Protocol#Type:Int) | Number of times this trade can be used before it's exhausted. |
| XP | [Int](https://wiki.vg/Protocol#Type:Int) | Amount of XP the villager will earn each time the trade is used. |
| Special Price | [Int](https://wiki.vg/Protocol#Type:Int) | Can be zero or negative. The number is added to the price when an item is discounted due to player reputation or other effects. |
| Price Multiplier | [Float](https://wiki.vg/Protocol#Type:Float) | Can be low (0.05) or high (0.2). Determines how much demand, player reputation, and temporary effects will adjust the price. |
| Demand | [Int](https://wiki.vg/Protocol#Type:Int) | If positive, causes the price to increase. Negative values seem to be treated the same as zero. |
| Villager level | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Appears on the trade GUI; meaning comes from the translation key `merchant.level.` + level.
1: Novice, 2: Apprentice, 3: Journeyman, 4: Expert, 5: Master.

 |
| Experience | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Total experience for this villager (always 0 for the wandering trader). |
| Is regular villager | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if this is a regular villager; false for the wandering trader. When false, hides the villager level and some other GUI elements. |
| Can restock | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True for regular villagers and false for the wandering trader. If true, the "Villagers restock up to two times per day." message is displayed when hovering over disabled trades. |

Modifiers can increase or decrease the number of items for the first input slot. The second input slot and the output slot never change the nubmer of items. The number of items may never be less than 1, and never more than the stack size. If special price and demand are both zero, only the default price is displayed. If either is non-zero, then the adjusted price is displayed next to the crossed-out default price. The adjusted prices is calculated as follows:

Adjusted price = default price + floor(default price x multiplier x demand) + special price

[![](https://wiki.vg/images/6/67/1.14-merchant-slots.png)](https://wiki.vg/File:1.14-merchant-slots.png)

The merchant UI, for reference

#### Update Entity Position

This packet is sent by the server when an entity moves a small distance. The change in position is represented as a [fixed-point number](https://wiki.vg/Protocol#Fixed-point_numbers) with 12 fraction bits and 4 integer bits. As such, the maximum movement distance along each axis is 8 blocks in the negative direction, or 7.999755859375 blocks in the positive direction. If the movement exceeds these limits, [Teleport Entity](https://wiki.vg/Protocol#Teleport_Entity) should be sent instead.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2E | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Delta X | [Short](https://wiki.vg/Protocol#Type:Short) | Change in X position as `currentX * 4096 - prevX * 4096`. |
| Delta Y | [Short](https://wiki.vg/Protocol#Type:Short) | Change in Y position as `currentY * 4096 - prevY * 4096`. |
| Delta Z | [Short](https://wiki.vg/Protocol#Type:Short) | Change in Z position as `currentZ * 4096 - prevZ * 4096`. |
| On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Update Entity Position and Rotation

This packet is sent by the server when an entity rotates and moves. See [#Update Entity Position](https://wiki.vg/Protocol#Update_Entity_Position) for how the position is encoded.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2F | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Delta X | [Short](https://wiki.vg/Protocol#Type:Short) | Change in X position as `currentX * 4096 - prevX * 4096`. |
| Delta Y | [Short](https://wiki.vg/Protocol#Type:Short) | Change in Y position as `currentY * 4096 - prevY * 4096`. |
| Delta Z | [Short](https://wiki.vg/Protocol#Type:Short) | Change in Z position as `currentZ * 4096 - prevZ * 4096`. |
| Yaw | [Angle](https://wiki.vg/Protocol#Type:Angle) | New angle, not a delta. |
| Pitch | [Angle](https://wiki.vg/Protocol#Type:Angle) | New angle, not a delta. |
| On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Update Entity Rotation

This packet is sent by the server when an entity rotates.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x30 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Yaw | [Angle](https://wiki.vg/Protocol#Type:Angle) | New angle, not a delta. |
| Pitch | [Angle](https://wiki.vg/Protocol#Type:Angle) | New angle, not a delta. |
| On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Move Vehicle

Note that all fields use absolute positioning and do not allow for relative positioning.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x31 | Play | Client | X | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position (X coordinate). |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position (Y coordinate). |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position (Z coordinate). |
| Yaw | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the vertical axis, in degrees. |
| Pitch | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the horizontal axis, in degrees. |

#### Open Book

Sent when a player right clicks with a signed book. This tells the client to open the book GUI.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x32 | Play | Client | Hand | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: Main hand, 1: Off hand . |

#### Open Screen

This is sent to the client when it should open an inventory, such as a chest, workbench, furnace, or other container. Resending this packet with already existing window id, will update the window title and window type without closing the window.

This message is not sent to clients opening their own inventory, nor do clients inform the server in any way when doing so. From the server's perspective, the inventory is always "open" whenever no other windows are.

For horses, use [Open Horse Screen](https://wiki.vg/Protocol#Open_Horse_Screen).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x33 | Play | Client | Window ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | An identifier for the window to be displayed. Notchian server implementation is a counter, starting at 1. There can only be one window at a time; this is only used to ignore outdated packets targeting already-closed windows. Note also that the Window ID field in most other packets is only a single byte, and indeed, the Notchian server wraps around after 100. |
| Window Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The window type to use for display. Contained in the `minecraft:menu` registry; see [Inventory](https://wiki.vg/Inventory "Inventory") for the different values. |
| Window Title | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The title of the window. |

#### Open Sign Editor

Sent when the client has placed a sign and is allowed to send [Update Sign](https://wiki.vg/Protocol#Update_Sign). There must already be a sign at the given location (which the client does not do automatically) - send a [Block Update](https://wiki.vg/Protocol#Block_Update) first.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x34 | Play | Client | Location | [Position](https://wiki.vg/Protocol#Type:Position) |  |
| Is Front Text | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the opened editor is for the front or on the back of the sign |

#### Ping (play)

Packet is not used by the Notchian server. When sent to the client, client responds with a [Pong](https://wiki.vg/Protocol#Pong_.28play.29) packet with the same id.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x35 | Play | Client | ID | [Int](https://wiki.vg/Protocol#Type:Int) |  |

#### Ping Response (play)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x36 | Play | Client | Payload | [Long](https://wiki.vg/Protocol#Type:Long) | Should be the same as sent by the client. |

#### Place Ghost Recipe

Response to the serverbound packet ([Place Recipe](https://wiki.vg/Protocol#Place_Recipe)), with the same recipe ID. Appears to be used to notify the UI.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x37 | Play | Client | Window ID | [Byte](https://wiki.vg/Protocol#Type:Byte) |  |
| Recipe | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | A recipe ID. |

#### Player Abilities (clientbound)

The latter 2 floats are used to indicate the flying speed and field of view respectively, while the first byte is used to determine the value of 4 booleans.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x38 | Play | Client | Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | Bit field, see below. |
| Flying Speed | [Float](https://wiki.vg/Protocol#Type:Float) | 0.05 by default. |
| Field of View Modifier | [Float](https://wiki.vg/Protocol#Type:Float) | Modifies the field of view, like a speed potion. A Notchian server will use the same value as the movement speed sent in the [Update Attributes](https://wiki.vg/Protocol#Update_Attributes) packet, which defaults to 0.1 for players. |

About the flags:

| Field | Bit |
| --- | --- |
| Invulnerable | 0x01 |
| Flying | 0x02 |
| Allow Flying | 0x04 |
| Creative Mode (Instant Break) | 0x08 |

#### Player Chat Message

Sends the client a chat message from a player.

Currently a lot is unknown about this packet, blank descriptions are for those that are unknown

| Packet ID | State | Bound To | Sector | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| 0x39 | Play | Client | Header | Sender | [UUID](https://wiki.vg/Protocol#Type:UUID) | Used by the Notchian client for the disableChat launch option. Setting both longs to 0 will always display the message regardless of the setting. |
| Index | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Message Signature Present | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | States if a message signature is present |
| Message Signature bytes | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (256) | Only present if `Message Signature Present` is true. Cryptography, the signature consists of the Sender UUID, Session UUID from the [Player Session](https://wiki.vg/Protocol#Player_Session) packet, Index, Salt, Timestamp in epoch seconds, the length of the original chat content, the original content itself, the length of Previous Messages, and all of the Previous message signatures. These values are hashed with [SHA-256](https://en.wikipedia.org/wiki/SHA-2) and signed using the [RSA](https://en.wikipedia.org/wiki/RSA_(cryptosystem)) cryptosystem. Modifying any of these values in the packet will cause this signature to fail. This buffer is always 256 bytes long and it is not length-prefixed. |
| Body | Message | [String](https://wiki.vg/Protocol#Type:String) (256) | Raw (optionally) signed sent message content.
This is used as the `content` parameter when formatting the message on the client.

 |
| Timestamp | [Long](https://wiki.vg/Protocol#Type:Long) | Represents the time the message was signed as milliseconds since the [epoch](https://en.wikipedia.org/wiki/Unix_time), used to check if the message was received within 2 minutes of it being sent. |
| Salt | [Long](https://wiki.vg/Protocol#Type:Long) | Cryptography, used for validating the message signature. |
| Previous Messages | Total Previous Messages | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The maximum length is 20 in Notchian client. |  |
| [Array](https://wiki.vg/Protocol#Type:Array) (20) | Message ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The message Id + 1, used for validating message signature. The next field is present only when value of this field is equal to 0. |
| Signature | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (256) | The previous message's signature. Contains the same type of data as `Message Signature bytes` (256 bytes) above. Not length-prefxied. |
| Other | Unsigned Content Present | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the next field is present |
| Unsigned Content | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| Filter Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | If the message has been filtered |
| Filter Type Bits | [Optional](https://wiki.vg/Protocol#Type:Optional) [BitSet](https://wiki.vg/Protocol#Type:BitSet) | Only present if the Filter Type is Partially Filtered. Specifies the indexes at which characters in the original message string should be replaced with the `#` symbol (i.e. filtered) by the Notchian client |
| Chat Formatting | Chat Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The type of chat in the `minecraft:chat_type` registry, defined by the [Registry Data](https://wiki.vg/Protocol#Registry_Data "Protocol") packet. |
| Sender Name | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The name of the one sending the message, usually the sender's display name.

This is used as the `sender` parameter when formatting the message on the client.

 |
| Has Target Name | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if target name is present. |
| Target Name | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The name of the one receiving the message, usually the receiver's display name. Only present if previous boolean is true.

This is used as the `target` parameter when formatting the message on the client.

 |

[![](https://wiki.vg/images/f/f4/MinecraftChat.drawio4.png)](https://wiki.vg/File:MinecraftChat.drawio4.png)

Player Chat Handling Logic

Filter Types:

The filter type mask should NOT be specified unless partially filtered is selected

| ID | Name | Description |
| --- | --- | --- |
| 0 | PASS\_THROUGH | Message is not filtered at all |
| 1 | FULLY\_FILTERED | Message is fully filtered |
| 2 | PARTIALLY\_FILTERED | Only some characters in the message are filtered |

#### End Combat

Unused by the Notchian client. This data was once used for twitch.tv metadata circa 1.8.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x3A | Play | Client | Duration | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the combat in ticks. |

#### Enter Combat

Unused by the Notchian client. This data was once used for twitch.tv metadata circa 1.8.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x3B | Play | Client | _no fields_ |

#### Combat Death

Used to send a respawn screen.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x3C | Play | Client | Player ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Entity ID of the player that died (should match the client's entity ID). |
| Message | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The death message. |

#### Player Info Remove

Used by the server to remove players from the player list.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x3D | Play | Client | Number of Players | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Player | Player Id | [Array](https://wiki.vg/Protocol#Type:Array) of [UUID](https://wiki.vg/Protocol#Type:UUID) | UUIDs of players to remove. |

#### Player Info Update

Sent by the server to update the user list (<tab> in the client).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x3E | Play | Client | Actions | [Byte](https://wiki.vg/Protocol#Type:Byte) | Determines what actions are present. |
| Number Of Players | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Players | UUID | [Array](https://wiki.vg/Protocol#Type:Array) | [UUID](https://wiki.vg/Protocol#Type:UUID) | The player UUID |
| Player Actions | [Array](https://wiki.vg/Protocol#Type:Array) of [Player Actions](https://wiki.vg/Protocol#player-info:player-actions) | The length of this array is determined by the number of [Player Actions](https://wiki.vg/Protocol#player-info:player-actions) that give a non-zero value when applying its mask to the actions flag. For example given the decimal number 5, binary 00000101. The masks 0x01 and 0x04 would return a non-zero value, meaning the Player Actions array would include two actions: Add Player and Update Game Mode. |

<table><caption id="player-info:player-actions">Player Actions</caption><tbody><tr><th>Action</th><th>Mask</th><th colspan="2">Field Name</th><th colspan="2">Type</th><th>Notes</th></tr><tr><td rowspan="6">Add Player</td><td rowspan="6">0x01</td><td colspan="2">Name</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:String">String</a> (16)</td></tr><tr><td colspan="2">Number Of Properties</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:VarInt">VarInt</a></td><td>Number of elements in the following array.</td></tr><tr><td rowspan="4">Property</td><td>Name</td><td rowspan="4"><a href="https://wiki.vg/Protocol#Type:Array">Array</a></td><td><a href="https://wiki.vg/Protocol#Type:String">String</a> (32767)</td><td></td></tr><tr><td>Value</td><td><a href="https://wiki.vg/Protocol#Type:String">String</a> (32767)</td><td></td></tr><tr><td>Is Signed</td><td><a href="https://wiki.vg/Protocol#Type:Boolean">Boolean</a></td><td></td></tr><tr><td>Signature</td><td><a href="https://wiki.vg/Protocol#Type:Optional">Optional</a> <a href="https://wiki.vg/Protocol#Type:String">String</a> (32767)</td><td>Only if Is Signed is true.</td></tr><tr><td rowspan="7">Initialize Chat</td><td rowspan="7">0x02</td><td colspan="2">Has Signature Data</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:Boolean">Boolean</a></td></tr><tr><td colspan="2">Chat session ID</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:UUID">UUID</a></td><td>Only sent if Has Signature Data is true.</td></tr><tr><td colspan="2">Public key expiry time</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:Long">Long</a></td><td>Key expiry time, as a UNIX timestamp in milliseconds. Only sent if Has Signature Data is true.</td></tr><tr><td colspan="2">Encoded public key size</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:VarInt">VarInt</a></td><td>Size of the following array. Only sent if Has Signature Data is true. Maximum length is 512 bytes.</td></tr><tr><td colspan="2">Encoded public key</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:Byte_Array">Byte Array</a> (512)</td><td>The player's public key, in bytes. Only sent if Has Signature Data is true.</td></tr><tr><td colspan="2">Public key signature size</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:VarInt">VarInt</a></td><td>Size of the following array. Only sent if Has Signature Data is true. Maximum length is 4096 bytes.</td></tr><tr><td colspan="2">Public key signature</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:Byte_Array">Byte Array</a> (4096)</td><td>The public key's digital signature. Only sent if Has Signature Data is true.</td></tr><tr><td>Update Game Mode</td><td>0x04</td><td colspan="2">Game Mode</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:VarInt">VarInt</a></td></tr><tr><td>Update Listed</td><td>0x08</td><td colspan="2">Listed</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:Boolean">Boolean</a></td><td>Whether the player should be listed on the player list.</td></tr><tr><td>Update Latency</td><td>0x10</td><td colspan="2">Ping</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:VarInt">VarInt</a></td><td>Measured in milliseconds.</td></tr><tr><td rowspan="2">Update Display Name</td><td rowspan="2">0x20</td><td colspan="2">Has Display Name</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:Boolean">Boolean</a></td></tr><tr><td colspan="2">Display Name</td><td colspan="2"><a href="https://wiki.vg/Protocol#Type:Optional">Optional</a> <a href="https://wiki.vg/Protocol#Type:Text_Component">Text Component</a></td><td>Only sent if Has Display Name is true.</td></tr></tbody></table>

The Property field looks as in the response of [Mojang API#UUID -> Profile + Skin/Cape](https://wiki.vg/Mojang_API#UUID_-.3E_Profile_.2B_Skin.2FCape "Mojang API"), except of course using the protocol format instead of JSON. That is, each player will usually have one property with Name “textures” and Value being a base64-encoded JSON string as documented at [Mojang API#UUID -> Profile + Skin/Cape](https://wiki.vg/Mojang_API#UUID_-.3E_Profile_.2B_Skin.2FCape "Mojang API"). An empty properties array is also acceptable, and will cause clients to display the player with one of the two default skins depending on UUID.

Ping values correspond with icons in the following way:

-   A ping that negative (i.e. not known to the server yet) will result in the no connection icon.
-   A ping under 150 milliseconds will result in 5 bars
-   A ping under 300 milliseconds will result in 4 bars
-   A ping under 600 milliseconds will result in 3 bars
-   A ping under 1000 milliseconds (1 second) will result in 2 bars
-   A ping greater than or equal to 1 second will result in 1 bar.

#### Look At

Used to rotate the client player to face the given location or entity (for `/teleport [<targets>] <x> <y> <z> facing`).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x3F | Play | Client |
| Feet/eyes | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Values are feet=0, eyes=1. If set to eyes, aims using the head position; otherwise aims using the feet position. |
| Target x | [Double](https://wiki.vg/Protocol#Type:Double) | x coordinate of the point to face towards. |
| Target y | [Double](https://wiki.vg/Protocol#Type:Double) | y coordinate of the point to face towards. |
| Target z | [Double](https://wiki.vg/Protocol#Type:Double) | z coordinate of the point to face towards. |
| Is entity | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, additional information about an entity is provided. |
| Entity ID | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Only if is entity is true — the entity to face towards. |
| Entity feet/eyes | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Whether to look at the entity's eyes or feet. Same values and meanings as before, just for the entity's head/feet. |

If the entity given by entity ID cannot be found, this packet should be treated as if is entity was false.

#### Synchronize Player Position

Teleports the client, e.g. during login, when using an ender pearl, in response to invalid move packets, etc.

Due to latency, the server may receive outdated movement packets sent before the client was aware of the teleport. To account for this, the server ignores all movement packets from the client until a [Confirm Teleportation](https://wiki.vg/Protocol#Confirm_Teleportation) packet with an ID matching the one sent in the teleport packet is received.

Yaw is measured in degrees, and does not follow classical trigonometry rules. The unit circle of yaw on the XZ-plane starts at (0, 1) and turns counterclockwise, with 90 at (-1, 0), 180 at (0, -1) and 270 at (1, 0). Additionally, yaw is not clamped to between 0 and 360 degrees; any number is valid, including negative numbers and numbers greater than 360.

Pitch is measured in degrees, where 0 is looking straight ahead, -90 is looking straight up, and 90 is looking straight down.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x40 | Play | Client | X | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute or relative position, depending on Flags. |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute or relative position, depending on Flags. |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute or relative position, depending on Flags. |
| Yaw | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute or relative rotation on the X axis, in degrees. |
| Pitch | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute or relative rotation on the Y axis, in degrees. |
| Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | Reference the Flags table below. When the value of the this byte masked is zero the field is absolute, otherwise relative. |
| Teleport ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Client should confirm this packet with [Confirm Teleportation](https://wiki.vg/Protocol#Confirm_Teleportation) containing the same Teleport ID. |

<table><caption>Flags</caption><tbody><tr><th>Field</th><th>Hex Mask</th></tr><tr><td>X</td><td>0x01</td></tr><tr><td>Y</td><td>0x02</td></tr><tr><td>Z</td><td>0x04</td></tr><tr><td>Y_ROT (Pitch)</td><td>0x08</td></tr><tr><td>X_ROT (Yaw)</td><td>0x10</td></tr></tbody></table>

#### Update Recipe Book

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x41 | Play | Client |
| Action | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | 0: init, 1: add, 2: remove. |
| Crafting Recipe Book Open | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the crafting recipe book will be open when the player opens its inventory. |
| Crafting Recipe Book Filter Active | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the filtering option is active when the players opens its inventory. |
| Smelting Recipe Book Open | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the smelting recipe book will be open when the player opens its inventory. |
| Smelting Recipe Book Filter Active | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the filtering option is active when the players opens its inventory. |
| Blast Furnace Recipe Book Open | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the blast furnace recipe book will be open when the player opens its inventory. |
| Blast Furnace Recipe Book Filter Active | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the filtering option is active when the players opens its inventory. |
| Smoker Recipe Book Open | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the smoker recipe book will be open when the player opens its inventory. |
| Smoker Recipe Book Filter Active | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the filtering option is active when the players opens its inventory. |
| Array size 1 | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Recipe IDs | [Array](https://wiki.vg/Protocol#Type:Array) of [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Array size 2 | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array, only present if action is 0 (init). |
| Recipe IDs | [Optional](https://wiki.vg/Protocol#Type:Optional) [Array](https://wiki.vg/Protocol#Type:Array) of [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Only present if mode is 0 (init) |

Action:

-   0 (init) = All the recipes in list 1 will be tagged as displayed, and all the recipes in list 2 will be added to the recipe book. Recipes that aren't tagged will be shown in the notification.
-   1 (add) = All the recipes in the list are added to the recipe book and their icons will be shown in the notification.
-   2 (remove) = Remove all the recipes in the list. This allows them to be re-displayed when they are re-added.

#### Remove Entities

Sent by the server when an entity is to be destroyed on the client.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x42 | Play | Client | Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Entity IDs | [Array](https://wiki.vg/Protocol#Type:Array) of [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The list of entities to destroy. |

#### Remove Entity Effect

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x43 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Effect ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | See [this table](https://minecraft.wiki/w/Status_effect%23Effect_list). |

#### Reset Score

This is sent to the client when it should remove a scoreboard item.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x44 | Play | Client | Entity Name | [String](https://wiki.vg/Protocol#Type:String) (32767) | The entity whose score this is. For players, this is their username; for other entities, it is their UUID. |
| Has Objective Name | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the score should be removed for the specified objective, or for all of them. |
| Objective Name | [Optional](https://wiki.vg/Protocol#Type:Optional) [String](https://wiki.vg/Protocol#Type:String) (32767) | The name of the objective the score belongs to. Only present if the previous field is true. |

#### Remove Resource Pack (play)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x45 | Play | Client | Has UUID | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether a specific resource pack should be removed, or all of them. |
| UUID | [Optional](https://wiki.vg/Protocol#Type:Optional) [UUID](https://wiki.vg/Protocol#Type:UUID) | The UUID of the resource pack to be removed. Only present if the previous field is true. |

#### Add Resource Pack (play)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x46 | Play | Client | UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) | The unique identifier of the resource pack. |
| URL | [String](https://wiki.vg/Protocol#Type:String) (32767) | The URL to the resource pack. |
| Hash | [String](https://wiki.vg/Protocol#Type:String) (40) | A 40 character hexadecimal, case-insensitive [SHA-1](http://en.wikipedia.org/wiki/SHA-1 "wikipedia:SHA-1") hash of the resource pack file.  
If it's not a 40 character hexadecimal string, the client will not use it for hash verification and likely waste bandwidth. |
| Forced | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | The Notchian client will be forced to use the resource pack from the server. If they decline they will be kicked from the server. |
| Has Prompt Message | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether a custom message should be used on the resource pack prompt. |
| Prompt Message | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | This is shown in the prompt making the client accept or decline the resource pack. Only present if the previous field is true. |

#### Respawn

<table><tbody><tr><td rowspan="2"><a href="https://wiki.vg/File:Huh.png"><img alt="Huh.png" src="https://wiki.vg/images/4/4f/Huh.png" decoding="async" width="40" height="40"></a></td><td>The following information needs to be added to this page:</td></tr><tr><td><b>Although the number of portal cooldown ticks is included in this packet, the whole portal usage process is still dictated entirely by the server. What kind of effect does this value have on the client, if any?</b></td></tr></tbody></table>

To change the player's dimension (overworld/nether/end), send them a respawn packet with the appropriate dimension, followed by prechunks/chunks for the new dimension, and finally a position and look packet. You do not need to unload chunks, the client will do it automatically.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x47 | Play | Client | Dimension Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of type of dimension in the `minecraft:dimension_type` registry, defined by the [Registry Data](https://wiki.vg/Protocol#Registry_Data "Protocol") packet. |
| Dimension Name | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the dimension being spawned into. |
| Hashed seed | [Long](https://wiki.vg/Protocol#Type:Long) | First 8 bytes of the SHA-256 hash of the world's seed. Used client side for biome noise |
| Game mode | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | 0: Survival, 1: Creative, 2: Adventure, 3: Spectator. |
| Previous Game mode | [Byte](https://wiki.vg/Protocol#Type:Byte) | \-1: Undefined (null), 0: Survival, 1: Creative, 2: Adventure, 3: Spectator. The previous game mode. Vanilla client uses this for the debug (F3 + N & F3 + F4) game mode switch. (More information needed) |
| Is Debug | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the world is a [debug mode](https://minecraft.wiki/w/Debug_mode) world; debug mode worlds cannot be modified and have predefined blocks. |
| Is Flat | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the world is a [superflat](https://minecraft.wiki/w/Superflat) world; flat worlds have different void fog and a horizon at y=0 instead of y=63. |
| Has death location | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, then the next two fields are present. |
| Death dimension Name | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the dimension the player died in. |
| Death location | [Optional](https://wiki.vg/Protocol#Type:Optional) [Position](https://wiki.vg/Protocol#Type:Position) | The location that the player died at. |
| Portal cooldown | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The number of ticks until the player can use the portal again. |
| Data kept | [Byte](https://wiki.vg/Protocol#Type:Byte) | Bit mask. 0x01: Keep attributes, 0x02: Keep metadata. Tells which data should be kept on the client side once the player has respawned.
In the Notchian implementation, this is context dependent:

-   normal respawns (after death) keep no data;
-   exiting the end poem/credits keeps the attributes;
-   other dimension changes (portals or teleports) keep all data.

 |

[![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) Avoid changing player's dimension to same dimension they were already in unless they are dead. If you change the dimension to one they are already in, weird bugs can occur, such as the player being unable to attack other players in new world (until they die and respawn).

Before 1.16, if you must respawn a player in the same dimension without killing them, send two respawn packets, one to a different world and then another to the world you want. You do not need to complete the first respawn; it only matters that you send two packets.

#### Set Head Rotation

Changes the direction an entity's head is facing.

While sending the Entity Look packet changes the vertical rotation of the head, sending this packet appears to be necessary to rotate the head horizontally.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x48 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Head Yaw | [Angle](https://wiki.vg/Protocol#Type:Angle) | New angle, not a delta. |

#### Update Section Blocks

Fired whenever 2 or more blocks are changed within the same chunk on the same tick.

 [![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) Changing blocks in chunks not loaded by the client is unsafe (see note on [Block Update](https://wiki.vg/Protocol#Block_Update)).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x49 | Play | Client | Chunk section position | [Long](https://wiki.vg/Protocol#Type:Long) | Chunk section coordinate (encoded chunk x and z with each 22 bits, and section y with 20 bits, from left to right). |
| Blocks array size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Blocks | [Array](https://wiki.vg/Protocol#Type:Array) of [VarLong](https://wiki.vg/Protocol#Type:VarLong) | Each entry is composed of the block state id, shifted left by 12, and the relative block position in the chunk section (4 bits for x, z, and y, from left to right). |

Chunk section position is encoded:

```
<span></span><span>((</span><span>sectionX</span> <span>&amp;</span> <span>0x3FFFFF</span><span>)</span> <span>&lt;&lt;</span> <span>42</span><span>)</span> <span>|</span> <span>(</span><span>sectionY</span> <span>&amp;</span> <span>0xFFFFF</span><span>)</span> <span>|</span> <span>((</span><span>sectionZ</span> <span>&amp;</span> <span>0x3FFFFF</span><span>)</span> <span>&lt;&lt;</span> <span>20</span><span>);</span>
```

and decoded:

```
<span></span><span>sectionX</span> <span>=</span> <span>long</span> <span>&gt;&gt;</span> <span>42</span><span>;</span>
<span>sectionY</span> <span>=</span> <span>long</span> <span>&lt;&lt;</span> <span>44</span> <span>&gt;&gt;</span> <span>44</span><span>;</span>
<span>sectionZ</span> <span>=</span> <span>long</span> <span>&lt;&lt;</span> <span>22</span> <span>&gt;&gt;</span> <span>42</span><span>;</span>
```

Blocks are encoded:

```
<span></span><span>blockStateId</span> <span>&lt;&lt;</span> <span>12</span> <span>|</span> <span>(</span><span>blockLocalX</span> <span>&lt;&lt;</span> <span>8</span> <span>|</span> <span>blockLocalZ</span> <span>&lt;&lt;</span> <span>4</span> <span>|</span> <span>blockLocalY</span><span>)</span>
<span>//Uses the local position of the given block position relative to its respective chunk section</span>
```

and decoded:

```
<span></span><span>blockStateId</span> <span>=</span> <span>long</span> <span>&gt;&gt;</span> <span>12</span><span>;</span>
<span>blockLocalX</span> <span>=</span> <span>(</span><span>long</span> <span>&gt;&gt;</span> <span>8</span><span>)</span> <span>&amp;</span> <span>0xF</span><span>;</span>
<span>blockLocalY</span> <span>=</span> <span>long</span> <span>&amp;</span> <span>0xF</span><span>;</span>
<span>blockLocalZ</span> <span>=</span> <span>(</span><span>long</span> <span>&gt;&gt;</span> <span>4</span><span>)</span> <span>&amp;</span> <span>0xF</span><span>;</span>
```

#### Select Advancements Tab

Sent by the server to indicate that the client should switch advancement tab. Sent either when the client switches tab in the GUI or when an advancement in another tab is made.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x4A | Play | Client | Has id | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Indicates if the next field is present. |
| Identifier | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | See below. |

The [Identifier](https://wiki.vg/Protocol#Type:Identifier) can be one of the following:

| Identifier |
| --- |
| minecraft:story/root |
| minecraft:nether/root |
| minecraft:end/root |
| minecraft:adventure/root |
| minecraft:husbandry/root |

If no or an invalid identifier is sent, the client will switch to the first tab in the GUI.

#### Server Data

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x4B | Play | Client | MOTD | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| Has Icon | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Size | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of bytes in the following array. Only present if Has Icon is true. |
| Icon | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) | Icon bytes in the PNG format. Only present is Has Icon is true. |

#### Set Action Bar Text

Displays a message above the hotbar. Equivalent to [System Chat Message](https://wiki.vg/Protocol#System_Chat_Message) with Overlay set to true, except that [chat message blocking](https://wiki.vg/Chat#Social_Interactions_.28blocking.29 "Chat") isn't performed. Used by the Notchian server only to implement the `/title` command.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x4C | Play | Client | Action bar text | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |

#### Set Border Center

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x4D | Play | Client | X | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) |  |

#### Set Border Lerp Size

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x4E | Play | Client | Old Diameter | [Double](https://wiki.vg/Protocol#Type:Double) | Current length of a single side of the world border, in meters. |
| New Diameter | [Double](https://wiki.vg/Protocol#Type:Double) | Target length of a single side of the world border, in meters. |
| Speed | [VarLong](https://wiki.vg/Protocol#Type:VarLong) | Number of real-time _milli_seconds until New Diameter is reached. It appears that Notchian server does not sync world border speed to game ticks, so it gets out of sync with server lag. If the world border is not moving, this is set to 0. |

#### Set Border Size

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x4F | Play | Client | Diameter | [Double](https://wiki.vg/Protocol#Type:Double) | Length of a single side of the world border, in meters. |

#### Set Border Warning Delay

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x50 | Play | Client | Warning Time | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | In seconds as set by `/worldborder warning time`. |

#### Set Border Warning Distance

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x51 | Play | Client | Warning Blocks | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | In meters. |

#### Set Camera

Sets the entity that the player renders from. This is normally used when the player left-clicks an entity while in spectator mode.

The player's camera will move with the entity and look where it is looking. The entity is often another player, but can be any type of entity. The player is unable to move this entity (move packets will act as if they are coming from the other entity).

If the given entity is not loaded by the player, this packet is ignored. To return control to the player, send this packet with their entity ID.

The Notchian server resets this (sends it back to the default entity) whenever the spectated entity is killed or the player sneaks, but only if they were spectating an entity. It also sends this packet whenever the player switches out of spectator mode (even if they weren't spectating an entity).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x52 | Play | Client | Camera ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | ID of the entity to set the client's camera to. |

The notchian client also loads certain shaders for given entities:

-   Creeper → `shaders/post/creeper.json`
-   Spider (and cave spider) → `shaders/post/spider.json`
-   Enderman → `shaders/post/invert.json`
-   Anything else → the current shader is unloaded

#### Set Held Item (clientbound)

Sent to change the player's slot selection.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x53 | Play | Client | Slot | [Byte](https://wiki.vg/Protocol#Type:Byte) | The slot which the player has selected (0–8). |

#### Set Center Chunk

Sets the center position of the client's chunk loading area. The area is square-shaped, spanning 2 × server view distance + 7 chunks on both axes (width, not radius!). Since the area's width is always an odd number, there is no ambiguity as to which chunk is the center.

The Notchian client ignores attempts to send chunks located outside the loading area, and immediately unloads any existing chunks no longer inside it.

The center chunk is normally the chunk the player is in, but apart from the implications on chunk loading, the (Notchian) client takes no issue with this not being the case. Indeed, as long as chunks are sent only within the default loading area centered on the world origin, it is not necessary to send this packet at all. This may be useful for servers with small bounded worlds, such as minigames, since it ensures chunks never need to be resent after the client has joined, saving on bandwidth.

The Notchian server sends this packet whenever the player moves across a chunk border horizontally, and also (according to testing) for any integer change in the vertical axis, even if it doesn't go across a chunk section border.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x54 | Play | Client | Chunk X | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Chunk X coordinate of the loading area center. |
| Chunk Z | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Chunk Z coordinate of the loading area center. |

#### Set Render Distance

Sent by the integrated singleplayer server when changing render distance. This packet is sent by the server when the client reappears in the overworld after leaving the end.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x55 | Play | Client | View Distance | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Render distance (2-32). |

#### Set Default Spawn Position

Sent by the server after login to specify the coordinates of the spawn point (the point at which players spawn at, and which the compass points to). It can be sent at any time to update the point compasses point at.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x56 | Play | Client | Location | [Position](https://wiki.vg/Protocol#Type:Position) | Spawn location. |
| Angle | [Float](https://wiki.vg/Protocol#Type:Float) | The angle at which to respawn at. |

#### Display Objective

This is sent to the client when it should display a scoreboard.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x57 | Play | Client | Position | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The position of the scoreboard. 0: list, 1: sidebar, 2: below name, 3 - 18: team specific sidebar, indexed as 3 + team color. |
| Score Name | [String](https://wiki.vg/Protocol#Type:String) (32767) | The unique name for the scoreboard to be displayed. |

#### Set Entity Metadata

Updates one or more [metadata](https://wiki.vg/Entity_metadata#Entity_Metadata_Format "Entity metadata") properties for an existing entity. Any properties not included in the Metadata field are left unchanged.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x58 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Metadata | [Entity Metadata](https://wiki.vg/Entity_metadata#Entity_Metadata_Format "Entity metadata") |  |

#### Link Entities

This packet is sent when an entity has been [leashed](https://minecraft.wiki/w/Lead) to another entity.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x59 | Play | Client | Attached Entity ID | [Int](https://wiki.vg/Protocol#Type:Int) | Attached entity's EID. |
| Holding Entity ID | [Int](https://wiki.vg/Protocol#Type:Int) | ID of the entity holding the lead. Set to -1 to detach. |

#### Set Entity Velocity

Velocity is in units of 1/8000 of a block per server tick (50ms); for example, -1343 would move (-1343 / 8000) = −0.167875 blocks per tick (or −3.3575 blocks per second).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x5A | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Velocity X | [Short](https://wiki.vg/Protocol#Type:Short) | Velocity on the X axis. |
| Velocity Y | [Short](https://wiki.vg/Protocol#Type:Short) | Velocity on the Y axis. |
| Velocity Z | [Short](https://wiki.vg/Protocol#Type:Short) | Velocity on the Z axis. |

#### Set Equipment

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x5B | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Entity's ID. |
| Equipment | Slot | [Array](https://wiki.vg/Protocol#Type:Array) | [Byte](https://wiki.vg/Protocol#Type:Byte) [Enum](https://wiki.vg/Protocol#Type:Enum) | Equipment slot (see below). Also has the top bit set if another entry follows, and otherwise unset if this is the last item in the array. |
| Item | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |

Equipment slot can be one of the following:

| ID | Equipment slot |
| --- | --- |
| 0 | Main hand |
| 1 | Off hand |
| 2 | Boots |
| 3 | Leggings |
| 4 | Chestplate |
| 5 | Helmet |
| 6 | Body |

#### Set Experience

Sent by the server when the client should change experience levels.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x5C | Play | Client | Experience bar | [Float](https://wiki.vg/Protocol#Type:Float) | Between 0 and 1. |
| Level | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Total Experience | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | See [Experience#Leveling up](https://minecraft.wiki/w/Experience%23Leveling_up) on the Minecraft Wiki for Total Experience to Level conversion. |

#### Set Health

Sent by the server to set the health of the player it is sent to.

Food [saturation](https://minecraft.wiki/w/Food%23Hunger_and_saturation) acts as a food “overcharge”. Food values will not decrease while the saturation is over zero. New players logging in or respawning automatically get a saturation of 5.0. Eating food increases the saturation as well as the food bar.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x5D | Play | Client | Health | [Float](https://wiki.vg/Protocol#Type:Float) | 0 or less = dead, 20 = full HP. |
| Food | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | 0–20. |
| Food Saturation | [Float](https://wiki.vg/Protocol#Type:Float) | Seems to vary from 0.0 to 5.0 in integer increments. |

#### Update Objectives

This is sent to the client when it should create a new [scoreboard](https://minecraft.wiki/w/Scoreboard) objective or remove one.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x5E | Play | Client | Objective Name | [String](https://wiki.vg/Protocol#Type:String) (32767) | A unique name for the objective. |
| Mode | [Byte](https://wiki.vg/Protocol#Type:Byte) | 0 to create the scoreboard. 1 to remove the scoreboard. 2 to update the display text. |
| Objective Value | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Only if mode is 0 or 2.The text to be displayed for the score. |
| Type | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Only if mode is 0 or 2. 0 = "integer", 1 = "hearts". |
| Has Number Format | [Optional](https://wiki.vg/Protocol#Type:Optional) [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Only if mode is 0 or 2. Whether this objective has a set number format for the scores. |
| Number Format | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Only if mode is 0 or 2 and the previous boolean is true. Determines how the score number should be formatted. |
| Number Format | Field Name |  |  |
| 0: blank | _no fields_ | Show nothing. |
| 1: styled | Styling | [Compound Tag](https://wiki.vg/NBT#Specification:compound_tag "NBT") | The styling to be used when formatting the score number. Contains the [text component styling fields](https://wiki.vg/Text_formatting#Styling_fields "Text formatting"). |
| 2: fixed | Content | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The text to be used as placeholder. |

#### Set Passengers

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x5F | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Vehicle's EID. |
| Passenger Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Passengers | [Array](https://wiki.vg/Protocol#Type:Array) of [VarInt](https://wiki.vg/Protocol#Type:VarInt) | EIDs of entity's passengers. |

#### Update Teams

Creates and updates teams.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x60 | Play | Client | Team Name | [String](https://wiki.vg/Protocol#Type:String) (32767) | A unique name for the team. (Shared with scoreboard). |
| Method | [Byte](https://wiki.vg/Protocol#Type:Byte) | Determines the layout of the remaining packet. |
| 0: create team | Team Display Name | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| Friendly Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | Bit mask. 0x01: Allow friendly fire, 0x02: can see invisible players on same team. |
| Name Tag Visibility | [String](https://wiki.vg/Protocol#Type:String) [Enum](https://wiki.vg/Protocol#Type:Enum) (40) | `always`, `hideForOtherTeams`, `hideForOwnTeam`, `never`. |
| Collision Rule | [String](https://wiki.vg/Protocol#Type:String) [Enum](https://wiki.vg/Protocol#Type:Enum) (40) | `always`, `pushOtherTeams`, `pushOwnTeam`, `never`. |
| Team Color | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Used to color the name of players on the team; see below. |
| Team Prefix | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Displayed before the names of players that are part of this team. |
| Team Suffix | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Displayed after the names of players that are part of this team. |
| Entity Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Entities | [Array](https://wiki.vg/Protocol#Type:Array) of [String](https://wiki.vg/Protocol#Type:String) (32767) | Identifiers for the entities in this team. For players, this is their username; for other entities, it is their UUID. |
| 1: remove team | _no fields_ | _no fields_ |  |
| 2: update team info | Team Display Name | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| Friendly Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | Bit mask. 0x01: Allow friendly fire, 0x02: can see invisible entities on same team. |
| Name Tag Visibility | [String](https://wiki.vg/Protocol#Type:String) [Enum](https://wiki.vg/Protocol#Type:Enum) (40) | `always`, `hideForOtherTeams`, `hideForOwnTeam`, `never` |
| Collision Rule | [String](https://wiki.vg/Protocol#Type:String) [Enum](https://wiki.vg/Protocol#Type:Enum) (40) | `always`, `pushOtherTeams`, `pushOwnTeam`, `never` |
| Team Color | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Used to color the name of players on the team; see below. |
| Team Prefix | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Displayed before the names of players that are part of this team. |
| Team Suffix | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Displayed after the names of players that are part of this team. |
| 3: add entities to team | Entity Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Entities | [Array](https://wiki.vg/Protocol#Type:Array) of [String](https://wiki.vg/Protocol#Type:String) (32767) | Identifiers for the added entities. For players, this is their username; for other entities, it is their UUID. |
| 4: remove entities from team | Entity Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Entities | [Array](https://wiki.vg/Protocol#Type:Array) of [String](https://wiki.vg/Protocol#Type:String) (32767) | Identifiers for the removed entities. For players, this is their username; for other entities, it is their UUID. |

Team Color: The color of a team defines how the names of the team members are visualized; any formatting code can be used. The following table lists all the possible values.

| ID | Formatting |
| --- | --- |
| 0-15 | Color formatting, same values as in [Text formatting#Colors](https://wiki.vg/Text_formatting#Colors "Text formatting"). |
| 16 | Obfuscated |
| 17 | Bold |
| 18 | Strikethrough |
| 19 | Underlined |
| 20 | Italic |
| 21 | Reset |

#### Update Score

This is sent to the client when it should update a scoreboard item.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x61 | Play | Client | Entity Name | [String](https://wiki.vg/Protocol#Type:String) (32767) | The entity whose score this is. For players, this is their username; for other entities, it is their UUID. |
| Objective Name | [String](https://wiki.vg/Protocol#Type:String) (32767) | The name of the objective the score belongs to. |
| Value | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The score to be displayed next to the entry. |
| Has Display Name | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether this score has a custom display name. |
| Display Name | [Optional](https://wiki.vg/Protocol#Type:Optional) [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The custom display name. Only present if the previous boolean is true. |
| Has Number Format | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether this score has a set number format. This overrides the number format set on the objective, if any. |
| Number Format | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Determines how the score number should be formatted. Only present if the previous boolean is true. |
| Number Format | Field Name |  |  |
| 0: blank | _no fields_ | Show nothing. |
| 1: styled | Styling | [Compound Tag](https://wiki.vg/NBT#Specification:compound_tag "NBT") | The styling to be used when formatting the score number. Contains the [text component styling fields](https://wiki.vg/Text_formatting#Styling_fields "Text formatting"). |
| 2: fixed | Content | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | The text to be used as placeholder. |

#### Set Simulation Distance

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x62 | Play | Client | Simulation Distance | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The distance that the client will process specific things, such as entities. |

#### Set Subtitle Text

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x63 | Play | Client | Subtitle Text | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |

#### Update Time

Time is based on ticks, where 20 ticks happen every second. There are 24000 ticks in a day, making Minecraft days exactly 20 minutes long.

The time of day is based on the timestamp modulo 24000. 0 is sunrise, 6000 is noon, 12000 is sunset, and 18000 is midnight.

The default SMP server increments the time by `20` every second.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x64 | Play | Client | World Age | [Long](https://wiki.vg/Protocol#Type:Long) | In ticks; not changed by server commands. |
| Time of day | [Long](https://wiki.vg/Protocol#Type:Long) | The world (or region) time, in ticks. If negative the sun will stop moving at the Math.abs of the time. |

#### Set Title Text

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x65 | Play | Client | Title Text | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |

#### Set Title Animation Times

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x66 | Play | Client | Fade In | [Int](https://wiki.vg/Protocol#Type:Int) | Ticks to spend fading in. |
| Stay | [Int](https://wiki.vg/Protocol#Type:Int) | Ticks to keep the title displayed. |
| Fade Out | [Int](https://wiki.vg/Protocol#Type:Int) | Ticks to spend fading out, not when to start fading out. |

#### Entity Sound Effect

Plays a sound effect from an entity, either by hardcoded ID or Identifier. Sound IDs and names can be found [here](https://pokechu22.github.io/Burger/1.21.html#sounds).

 [![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) Numeric sound effect IDs are liable to change between versions

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x67 | Play | Client | Sound ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Represents the `Sound ID + 1`. If the value is 0, the packet contains a sound specified by Identifier. |
| Sound Name | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Only present if Sound ID is 0 |
| Has Fixed Range | [Optional](https://wiki.vg/Protocol#Type:Optional) [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Only present if Sound ID is 0. |
| Range | [Optional](https://wiki.vg/Protocol#Type:Optional) [Float](https://wiki.vg/Protocol#Type:Float) | The fixed range of the sound. Only present if previous boolean is true and Sound ID is 0. |
| Sound Category | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The category that this sound will be played from ([current categories](https://gist.github.com/konwboj/7c0c380d3923443e9d55)). |
| Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Volume | [Float](https://wiki.vg/Protocol#Type:Float) | 1.0 is 100%, capped between 0.0 and 1.0 by Notchian clients. |
| Pitch | [Float](https://wiki.vg/Protocol#Type:Float) | Float between 0.5 and 2.0 by Notchian clients. |
| Seed | [Long](https://wiki.vg/Protocol#Type:Long) | Seed used to pick sound variant. |

#### Sound Effect

Plays a sound effect at the given location, either by hardcoded ID or Identifier. Sound IDs and names can be found [here](https://pokechu22.github.io/Burger/1.21.html#sounds).

 [![Warning.png](https://wiki.vg/images/c/cb/Warning.png)](https://wiki.vg/File:Warning.png) Numeric sound effect IDs are liable to change between versions

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x68 | Play | Client | Sound ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Represents the `Sound ID + 1`. If the value is 0, the packet contains a sound specified by Identifier. |
| Sound Name | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Only present if Sound ID is 0 |
| Has Fixed Range | [Optional](https://wiki.vg/Protocol#Type:Optional) [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Only present if Sound ID is 0. |
| Range | [Optional](https://wiki.vg/Protocol#Type:Optional) [Float](https://wiki.vg/Protocol#Type:Float) | The fixed range of the sound. Only present if previous boolean is true and Sound ID is 0. |
| Sound Category | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The category that this sound will be played from ([current categories](https://gist.github.com/konwboj/7c0c380d3923443e9d55)). |
| Effect Position X | [Int](https://wiki.vg/Protocol#Type:Int) | Effect X multiplied by 8 ([fixed-point number](https://wiki.vg/Data_types#Fixed-point_numbers "Data types") with only 3 bits dedicated to the fractional part). |
| Effect Position Y | [Int](https://wiki.vg/Protocol#Type:Int) | Effect Y multiplied by 8 ([fixed-point number](https://wiki.vg/Data_types#Fixed-point_numbers "Data types") with only 3 bits dedicated to the fractional part). |
| Effect Position Z | [Int](https://wiki.vg/Protocol#Type:Int) | Effect Z multiplied by 8 ([fixed-point number](https://wiki.vg/Data_types#Fixed-point_numbers "Data types") with only 3 bits dedicated to the fractional part). |
| Volume | [Float](https://wiki.vg/Protocol#Type:Float) | 1.0 is 100%, capped between 0.0 and 1.0 by Notchian clients. |
| Pitch | [Float](https://wiki.vg/Protocol#Type:Float) | Float between 0.5 and 2.0 by Notchian clients. |
| Seed | [Long](https://wiki.vg/Protocol#Type:Long) | Seed used to pick sound variant. |

#### Start Configuration

Sent during gameplay in order to redo the configuration process. The client must respond with [Acknowledge Configuration](https://wiki.vg/Protocol#Acknowledge_Configuration) for the process to start.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x69 | Play | Client | _no fields_ |

This packet switches the connection state to [configuration](https://wiki.vg/Protocol#Configuration).

#### Stop Sound

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x6A | Play | Client | Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | Controls which fields are present. |
| Source | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Only if flags is 3 or 1 (bit mask 0x1). See below. If not present, then sounds from all sources are cleared. |
| Sound | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Only if flags is 2 or 3 (bit mask 0x2). A sound effect name, see [Custom Sound Effect](https://wiki.vg/Protocol#Custom_Sound_Effect). If not present, then all sounds are cleared. |

Categories:

| Name | Value |
| --- | --- |
| master | 0 |
| music | 1 |
| record | 2 |
| weather | 3 |
| block | 4 |
| hostile | 5 |
| neutral | 6 |
| player | 7 |
| ambient | 8 |
| voice | 9 |

#### Store Cookie (play)

Stores some arbitrary data on the client, which persists between server transfers. The Notchian client only accepts cookies of up to 5 kiB in size.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x6B | Play | Client | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |
| Payload Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following byte array. |
| Payload | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (5120) | The data of the cookie. |

#### System Chat Message

Sends the client a raw system message.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x6C | Play | Client | Content | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | Limited to 262144 bytes. |
| Overlay | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the message is an actionbar or chat message. See also [#Set Action Bar Text](https://wiki.vg/Protocol#Set_Action_Bar_Text). |

#### Set Tab List Header And Footer

This packet may be used by custom servers to display additional information above/below the player list. It is never sent by the Notchian server.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x6D | Play | Client | Header | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | To remove the header, send a empty text component: `{"text":""}`. |
| Footer | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | To remove the footer, send a empty text component: `{"text":""}`. |

#### Tag Query Response

Sent in response to [Query Block Entity Tag](https://wiki.vg/Protocol#Query_Block_Entity_Tag) or [Query Entity Tag](https://wiki.vg/Protocol#Query_Entity_Tag).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x6E | Play | Client | Transaction ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Can be compared to the one sent in the original query packet. |
| NBT | [NBT](https://wiki.vg/Protocol#Type:NBT) | The NBT of the block or entity. May be a TAG\_END (0) in which case no NBT is present. |

#### Pickup Item

Sent by the server when someone picks up an item lying on the ground — its sole purpose appears to be the animation of the item flying towards you. It doesn't destroy the entity in the client memory, and it doesn't add it to your inventory. The server only checks for items to be picked up after each [Set Player Position](https://wiki.vg/Protocol#Set_Player_Position) (and [Set Player Position And Rotation](https://wiki.vg/Protocol#Set_Player_Position_And_Rotation)) packet sent by the client. The collector entity can be any entity; it does not have to be a player. The collected entity also can be any entity, but the Notchian server only uses this for items, experience orbs, and the different varieties of arrows.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x6F | Play | Client | Collected Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Collector Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Pickup Item Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Seems to be 1 for XP orbs, otherwise the number of items in the stack. |

#### Teleport Entity

This packet is sent by the server when an entity moves more than 8 blocks.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x70 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| X | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Yaw | [Angle](https://wiki.vg/Protocol#Type:Angle) | (Y Rot)New angle, not a delta. |
| Pitch | [Angle](https://wiki.vg/Protocol#Type:Angle) | (X Rot)New angle, not a delta. |
| On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Set Ticking State

Used to adjust the ticking rate of the client, and whether it's frozen.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x71 | Play | Client | Tick rate | [Float](https://wiki.vg/Protocol#Type:Float) |  |
| Is frozen | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Step Tick

Advances the client processing by the specified number of ticks. Has no effect unless client ticking is frozen.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x72 | Play | Client | Tick steps | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |

#### Transfer (play)

Notifies the client that it should transfer to the given server. Cookies previously stored are preserved between server transfers.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x73 | Play | Client | Host | [String](https://wiki.vg/Protocol#Type:String) | The hostname of IP of the server. |
| Port | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The port of the server. |

#### Update Advancements

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x74 | Play | Client | Reset/Clear | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether to reset/clear the current advancements. |
| Mapping size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Size of the following array. |
| Advancement mapping | Key | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the advancement. |
| Value | Advancement | See below |
| List size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Size of the following array. |
| Identifiers | [Array](https://wiki.vg/Protocol#Type:Array) of [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifiers of the advancements that should be removed. |
| Progress size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Size of the following array. |
| Progress mapping | Key | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the advancement. |
| Value | Advancement progress | See below. |

Advancement structure:

| Field Name | Field Type | Notes |
| --- | --- | --- |
| Has parent | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Indicates whether the next field exists. |
| Parent id | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the parent advancement. |
| Has display | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Indicates whether the next field exists. |
| Display data | [Optional](https://wiki.vg/Protocol#Type:Optional) Advancement display | See below. |
| Array length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of arrays in the following array. |
| Requirements | Array length 2 | [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Requirement | [Array](https://wiki.vg/Protocol#Type:Array) of [String](https://wiki.vg/Protocol#Type:String) (32767) | Array of required criteria. |
| Sends telemetry data | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the client should include this achievement in the telemetry data when it's completed.
The Notchian client only sends data for advancements on the `minecraft` namespace.

 |

Advancement display:

| Field Name | Field Type | Notes |
| --- | --- | --- |
| Title | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| Description | [Text Component](https://wiki.vg/Protocol#Type:Text_Component) |  |
| Icon | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |
| Frame type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0 = `task`, 1 = `challenge`, 2 = `goal`. |
| Flags | [Int](https://wiki.vg/Protocol#Type:Int) | 0x01: has background texture; 0x02: `show_toast`; 0x04: `hidden`. |
| Background texture | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Background texture location. Only if flags indicates it. |
| X coord | [Float](https://wiki.vg/Protocol#Type:Float) |  |
| Y coord | [Float](https://wiki.vg/Protocol#Type:Float) |  |

Advancement progress:

| Field Name | Field Type | Notes |
| --- | --- | --- |
| Size | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Size of the following array. |
| Criteria | Criterion identifier | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the criterion. |
| Criterion progress | Criterion progress |  |

Criterion progress:

| Field Name | Field Type | Notes |
| --- | --- | --- |
| Achieved | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, next field is present. |
| Date of achieving | [Optional](https://wiki.vg/Protocol#Type:Optional) [Long](https://wiki.vg/Protocol#Type:Long) | As returned by [`Date.getTime`](https://docs.oracle.com/javase/6/docs/api/java/util/Date.html#getTime()). |

#### Update Attributes

Sets [attributes](https://minecraft.wiki/w/Attribute) on the given entity.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x75 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Number Of Properties | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Property | Id | [Array](https://wiki.vg/Protocol#Type:Array) | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | See below. |
| Value | [Double](https://wiki.vg/Protocol#Type:Double) | See below. |
| Number Of Modifiers | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Modifiers | [Array](https://wiki.vg/Protocol#Type:Array) of Modifier Data | See [Attribute#Modifiers](https://minecraft.wiki/w/Attribute%23Modifiers). Modifier Data defined below. |

Known Id values (see also [Attribute#Modifiers](https://minecraft.wiki/w/Attribute%23Modifiers)):

| Id | Key | Default | Min | Max | Label |
| --- | --- | --- | --- | --- | --- |
| 0 | generic.armor | 0.0 | 0.0 | 30.0 | Armor. |
| 1 | generic.armor\_toughness | 0.0 | 0.0 | 20.0 | Armor Toughness. |
| 2 | generic.attack\_damage | 2.0 | 0.0 | 2048.0 | Attack Damage. |
| 3 | generic.attack\_knockback | 0.0 | 0.0 | 5.0 | Attack Knockback. |
| 4 | generic.attack\_speed | 4.0 | 0.0 | 1024.0 | Attack Speed. |
| 5 | generic.block\_break\_speed | 1.0 | 0.0 | 1024.0 | Block Break Speed. |
| 6 | generic.block\_interaction\_range | 4.5 | 0.0 | 64.0 | Block Interaction Range. |
| 7 | generic.entity\_interaction\_range | 3.0 | 0.0 | 64.0 | Entity Interaction Range. |
| 8 | generic.fall\_damage\_multiplier | 1.0 | 0.0 | 100.0 | Fall Damage Multiplier. |
| 9 | generic.flying\_speed | 0.4 | 0.0 | 1024.0 | Flying Speed. |
| 10 | generic.follow\_range | 32.0 | 0.0 | 2048.0 | Follow Range. |
| 11 | generic.gravity | 0.08 | \-1.0 | 1.0 | Gravity. |
| 12 | generic.jump\_strength | 0.42 | 0.0 | 32.0 | Jump Strength. |
| 13 | generic.knockback\_resistance | 0.0 | 0.0 | 1.0 | Knockback Resistance. |
| 14 | generic.luck | 0.0 | \-1024.0 | 1024.0 | Luck. |
| 15 | generic.max\_absorption | 0.0 | 0.0 | 2048.0 | Max Absorption. |
| 16 | generic.max\_health | 20.0 | 1.0 | 1024.0 | Max Health. |
| 17 | generic.movement\_speed | 0.7 | 0.0 | 1024.0 | Movement Speed. |
| 18 | generic.safe\_fall\_distance | 3.0 | \-1024.0 | 1024.0 | Safe Fall Distance. |
| 19 | generic.scale | 1.0 | 0.0625 | 16.0 | Scale. |
| 20 | zombie.spawn\_reinforcements | 0.0 | 0.0 | 1.0 | Spawn Reinforcements Chance. |
| 21 | generic.step\_height | 0.6 | 0.0 | 10.0 | Step Height. |

_Modifier Data_ structure:

| Field Name | Field Type | Notes |
| --- | --- | --- |
| UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) |  |
| Amount | [Double](https://wiki.vg/Protocol#Type:Double) | May be positive or negative. |
| Operation | [Byte](https://wiki.vg/Protocol#Type:Byte) | See below. |

The operation controls how the base value of the modifier is changed.

-   0: Add/subtract amount
-   1: Add/subtract amount percent of the current value
-   2: Multiply by amount percent

All of the 0's are applied first, and then the 1's, and then the 2's.

#### Entity Effect

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x76 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Effect ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | See [this table](https://minecraft.wiki/w/Status_effect%23Effect_list). |
| Amplifier | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Notchian client displays effect level as Amplifier + 1. |
| Duration | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Duration in ticks. (-1 for infinite) |
| Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | Bit field, see below. |

<table><tbody><tr><td rowspan="2"><a href="https://wiki.vg/File:Huh.png"><img alt="Huh.png" src="https://wiki.vg/images/4/4f/Huh.png" decoding="async" width="40" height="40"></a></td><td>The following information needs to be added to this page:</td></tr><tr><td><b>What exact effect does the blend bit flag have on the client? What happens if it is used on effects besides DARKNESS?</b></td></tr></tbody></table>

Within flags:

-   0x01: Is ambient - was the effect spawned from a beacon? All beacon-generated effects are ambient. Ambient effects use a different icon in the HUD (blue border rather than gray). If all effects on an entity are ambient, the ["Is potion effect ambient" living metadata field](https://wiki.vg/Entity_metadata#Living_Entity "Entity metadata") should be set to true. Usually should not be enabled.
-   0x02: Show particles - should all particles from this effect be hidden? Effects with particles hidden are not included in the calculation of the effect color, and are not rendered on the HUD (but are still rendered within the inventory). Usually should be enabled.
-   0x04: Show icon - should the icon be displayed on the client? Usually should be enabled.
-   0x08: Blend - should the effect's hard-coded blending be applied? Currently only used in the DARKNESS effect to apply extra void fog and adjust the gamma value for lighting.

#### Update Recipes

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x77 | Play | Client | Num Recipes | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Recipe | Recipe ID | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Type ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The recipe type, see below. |
| Data | Varies | Additional data for the recipe. |

Recipe types:

| ID | Type | Description | Data |
| --- | --- | --- | --- |
| 0 | `minecraft:crafting_shaped` | Shaped crafting recipe. All items must be present in the same pattern (which may be flipped horizontally or translated). | As follows:
| Name | Type | Description |
| --- | --- | --- |
| Group | [String](https://wiki.vg/Protocol#Type:String) (32767) | Used to group similar recipes together in the recipe book. Tag is present in recipe JSON. |
| Category | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Building = 0, Redstone = 1, Equipment = 2, Misc = 3 |
| Width | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Height | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Ingredients | [Array](https://wiki.vg/Protocol#Type:Array) of Ingredient | Length is `width * height`. Indexed by `x + (y * width)`. |
| Result | [Slot](https://wiki.vg/Protocol#Type:Slot) |
| Show notification | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Show a toast when the recipe is [added](https://wiki.vg/Protocol#Update_Recipe_Book "Protocol"). |

 |
| 1 | `minecraft:crafting_shapeless` | Shapeless crafting recipe. All items in the ingredient list must be present, but in any order/slot. | As follows:

| Name | Type | Description |
| --- | --- | --- |
| Group | [String](https://wiki.vg/Protocol#Type:String) (32767) | Used to group similar recipes together in the recipe book. Tag is present in recipe JSON. |
| Category | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Building = 0, Redstone = 1, Equipment = 2, Misc = 3 |
| Ingredient count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Ingredients | [Array](https://wiki.vg/Protocol#Type:Array) of Ingredient. |  |
| Result | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |

 |
| 2 | `minecraft:crafting_special_armordye` | Recipe for dying leather armor | As follows:

| Name | Type | Description |
| --- | --- | --- |
| Category | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Building = 0, Redstone = 1, Equipment = 2, Misc = 3 |

 |
| 3 | `minecraft:crafting_special_bookcloning` | Recipe for copying contents of written books |
| 4 | `minecraft:crafting_special_mapcloning` | Recipe for copying maps |
| 5 | `minecraft:crafting_special_mapextending` | Recipe for adding paper to maps |
| 6 | `minecraft:crafting_special_firework_rocket` | Recipe for making firework rockets |
| 7 | `minecraft:crafting_special_firework_star` | Recipe for making firework stars |
| 8 | `minecraft:crafting_special_firework_star_fade` | Recipe for making firework stars fade between multiple colors |
| 9 | `minecraft:crafting_special_tippedarrow` | Recipe for crafting tipped arrows |
| 10 | `minecraft:crafting_special_bannerduplicate` | Recipe for copying banner patterns |
| 11 | `minecraft:crafting_special_shielddecoration` | Recipe for applying a banner's pattern to a shield |
| 12 | `minecraft:crafting_special_shulkerboxcoloring` | Recipe for recoloring a shulker box |
| 13 | `minecraft:crafting_special_suspiciousstew` | Recipe for crafting suspicious stews |
| 14 | `minecraft:crafting_special_repairitem` | Recipe for repairing items via crafting |
| 22 | `minecraft:crafting_decorated_pot` | Recipe for crafting decorated pots |
| 15 | `minecraft:smelting` | Smelting recipe | As follows:

| Name | Type | Description |
| --- | --- | --- |
| Group | [String](https://wiki.vg/Protocol#Type:String) (32767) | Used to group similar recipes together in the recipe book. |
| Category | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Food = 0, Blocks = 1, Misc = 2 |
| Ingredient | Ingredient |  |
| Result | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |
| Experience | [Float](https://wiki.vg/Protocol#Type:Float) |  |
| Cooking time | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |

 |
| 16 | `minecraft:blasting` | Blast furnace recipe |
| 17 | `minecraft:smoking` | Smoker recipe |
| 18 | `minecraft:campfire_cooking` | Campfire recipe |
| 19 | `minecraft:stonecutting` | Stonecutter recipe | As follows:

| Name | Type | Description |
| --- | --- | --- |
| Group | [String](https://wiki.vg/Protocol#Type:String) (32767) | Used to group similar recipes together in the recipe book. Tag is present in recipe JSON. |
| Ingredient | Ingredient |  |
| Result | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |

 |
| 20 | `minecraft:smithing_transform` | Recipe for smithing netherite gear | As follows:

| Name | Type | Description |
| --- | --- | --- |
| Template | Ingredient | The smithing template. |
| Base | Ingredient | The base item. |
| Addition | Ingredient | The additional ingredient. |
| Result | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |

 |
| 21 | `minecraft:smithing_trim` | Recipe for applying armor trims | As follows:

| Name | Type | Description |
| --- | --- | --- |
| Template | Ingredient | The smithing template. |
| Base | Ingredient | The base item. |
| Addition | Ingredient | The additional ingredient. |

 |

Ingredient is defined as:

| Name | Type | Description |
| --- | --- | --- |
| Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. |
| Items | [Array](https://wiki.vg/Protocol#Type:Array) of [Slot](https://wiki.vg/Protocol#Type:Slot) | Any item in this array may be used for the recipe. The count of each item should be 1. |

#### Update Tags (play)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x78 | Play | Client | Length of the array | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Array of tags | Registry | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Registry identifier (Vanilla expects tags for the registries `minecraft:block`, `minecraft:item`, `minecraft:fluid`, `minecraft:entity_type`, and `minecraft:game_event`) |
| Array of Tag | (See below) |  |

Tag arrays look like:

| Field Name | Field Type | Notes |
| --- | --- | --- |
| Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array |
| Tags | Tag name | [Array](https://wiki.vg/Protocol#Type:Array) | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array |
| Entries | [Array](https://wiki.vg/Protocol#Type:Array) of [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Numeric IDs of the given type (block, item, etc.). This list replaces the previous list of IDs for the given tag. If some preexisting tags are left unmentioned, a warning is printed. |

See [Tag](https://minecraft.wiki/w/Tag) on the Minecraft Wiki for more information, including a list of vanilla tags.

#### Projectile Power

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x79 | Play | Client | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Power X | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Power Y | [Double](https://wiki.vg/Protocol#Type:Double) |  |
| Power Z | [Double](https://wiki.vg/Protocol#Type:Double) |  |

#### Custom Report Details

Contains a list of key-value text entries that are included in any crash or disconnection report generated during connection to the server.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x7A | Configuration | Client | Details Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) (32) | The number of details in the following array. |
| Details | Title | [Array](https://wiki.vg/Protocol#Type:Array) | [String](https://wiki.vg/Protocol#Type:String) (128) |  |
| Description | [String](https://wiki.vg/Protocol#Type:String) (4096) |  |

#### Server Links

This packet contains a list of links that the Notchian client will display in the menu available from the pause menu. Link labels can be built-in or custom (i.e., any text).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x7B | Configuration | Client | Links Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The number of links in the following array. |
| Links | Is built-in | [Array](https://wiki.vg/Protocol#Type:Array) | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Determines if the following label is built-in (from enum) or custom (text component). |
| Label | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) / [Text Component](https://wiki.vg/Protocol#Type:Text_Component) | See below. |
| URL | [String](https://wiki.vg/Protocol#Type:String) | Valid URL. |

| ID | Name | Notes |
| --- | --- | --- |
| 0 | Bug Report | Displayed on connection error screen; included as a comment in the disconnection report. |
| 1 | Community Guidelines |  |
| 2 | Support |  |
| 3 | Status |  |
| 4 | Feedback |  |
| 5 | Community |  |
| 6 | Website |  |
| 7 | Forums |  |
| 8 | News |  |
| 9 | Announcements |  |

### Serverbound

#### Confirm Teleportation

Sent by client as confirmation of [Synchronize Player Position](https://wiki.vg/Protocol#Synchronize_Player_Position).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x00 | Play | Server | Teleport ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID given by the [Synchronize Player Position](https://wiki.vg/Protocol#Synchronize_Player_Position) packet. |

#### Query Block Entity Tag

Used when F3+I is pressed while looking at a block.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x01 | Play | Server | Transaction ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | An incremental ID so that the client can verify that the response matches. |
| Location | [Position](https://wiki.vg/Protocol#Type:Position) | The location of the block to check. |

#### Change Difficulty

Must have at least op level 2 to use. Appears to only be used on singleplayer; the difficulty buttons are still disabled in multiplayer.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x02 | Play | Server | New difficulty | [Byte](https://wiki.vg/Protocol#Type:Byte) | 0: peaceful, 1: easy, 2: normal, 3: hard . |

#### Acknowledge Message

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x03 | Play | Server | Message Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |

#### Chat Command

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x04 | Play | Server | Command | [String](https://wiki.vg/Protocol#Type:String) (32767) | The command typed by the client. |

#### Signed Chat Command

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x05 | Play | Server | Command | [String](https://wiki.vg/Protocol#Type:String) (32767) | The command typed by the client. |
| Timestamp | [Long](https://wiki.vg/Protocol#Type:Long) | The timestamp that the command was executed. |
| Salt | [Long](https://wiki.vg/Protocol#Type:Long) | The salt for the following argument signatures. |
| Array length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of entries in the following array. The maximum length in Notchian server is 8. |
| Array of argument signatures | Argument name | [Array](https://wiki.vg/Protocol#Type:Array) (8) | [String](https://wiki.vg/Protocol#Type:String) (16) | The name of the argument that is signed by the following signature. |
| Signature | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (256) | The signature that verifies the argument. Always 256 bytes and is not length-prefixed. |
| Message Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Acknowledged | [Fixed BitSet](https://wiki.vg/Protocol#Type:Fixed_BitSet) (20) |  |

#### Chat Message

Used to send a chat message to the server. The message may not be longer than 256 characters or else the server will kick the client.

The server will broadcast a [Player Chat Message](https://wiki.vg/Protocol#Player_Chat_Message) packet with Chat Type `minecraft:chat` to all players that haven't disabled chat (including the player that sent the message). See [Chat#Processing chat](https://wiki.vg/Chat#Processing_chat "Chat") for more information.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x06 | Play | Server | Message | [String](https://wiki.vg/Protocol#Type:String) (256) |  |
| Timestamp | [Long](https://wiki.vg/Protocol#Type:Long) |  |
| Salt | [Long](https://wiki.vg/Protocol#Type:Long) | The salt used to verify the signature hash. |
| Has Signature | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the next field is present. |
| Signature | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (256) | The signature used to verify the chat message's authentication. When present, always 256 bytes and not length-prefixed. |
| Message Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Acknowledged | [Fixed BitSet](https://wiki.vg/Protocol#Type:Fixed_BitSet) (20) |  |

#### Player Session

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x07 | Play | Server | Session Id | [UUID](https://wiki.vg/Protocol#Type:UUID) |  |
| Public Key | Expires At | [Long](https://wiki.vg/Protocol#Type:Long) | The time the play session key expires in [epoch](https://en.wikipedia.org/wiki/Unix_time) milliseconds. |
| Public Key Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the proceeding public key. Maximum length in Notchian server is 512 bytes. |
| Public Key | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (512) | A byte array of an X.509-encoded public key. |
| Key Signature Length | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the proceeding key signature array. Maximum length in Notchian server is 4096 bytes. |
| Key Signature | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (4096) | The signature consists of the player UUID, the key expiration timestamp, and the public key data. These values are hashed using [SHA-1](https://en.wikipedia.org/wiki/SHA-1) and signed using Mojang's private [RSA](https://en.wikipedia.org/wiki/RSA_(cryptosystem)) key. |

#### Chunk Batch Received

Notifies the server that the chunk batch has been received by the client. The server uses the value sent in this packet to adjust the number of chunks to be sent in a batch.

The Notchian server will stop sending further chunk data until the client acknowledges the sent chunk batch. After the first acknowledgement, the server adjusts this number to allow up to 10 unacknowledged batches.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x08 | Play | Server | Chunks per tick | [Float](https://wiki.vg/Protocol#Type:Float) | Desired chunks per tick. |

#### Client Status

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x09 | Play | Server | Action ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | See below |

_Action ID_ values:

| Action ID | Action | Notes |
| --- | --- | --- |
| 0 | Perform respawn | Sent when the client is ready to complete login and when the client is ready to respawn after death. |
| 1 | Request stats | Sent when the client opens the Statistics menu. |

#### Client Information (play)

Sent when the player connects, or when settings are changed.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0A | Play | Server | Locale | [String](https://wiki.vg/Protocol#Type:String) (16) | e.g. `en_GB`. |
| View Distance | [Byte](https://wiki.vg/Protocol#Type:Byte) | Client-side render distance, in chunks. |
| Chat Mode | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: enabled, 1: commands only, 2: hidden. See [Chat#Client chat mode](https://wiki.vg/Chat#Client_chat_mode "Chat") for more information. |
| Chat Colors | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | “Colors” multiplayer setting. Can the chat be colored? |
| Displayed Skin Parts | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Bit mask, see below. |
| Main Hand | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: Left, 1: Right. |
| Enable text filtering | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Enables filtering of text on signs and written book titles. Currently always false (i.e. the filtering is disabled) |
| Allow server listings | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Servers usually list online players, this option should let you not show up in that list. |

_Displayed Skin Parts_ flags:

-   Bit 0 (0x01): Cape enabled
-   Bit 1 (0x02): Jacket enabled
-   Bit 2 (0x04): Left Sleeve enabled
-   Bit 3 (0x08): Right Sleeve enabled
-   Bit 4 (0x10): Left Pants Leg enabled
-   Bit 5 (0x20): Right Pants Leg enabled
-   Bit 6 (0x40): Hat enabled

The most significant bit (bit 7, 0x80) appears to be unused.

#### Command Suggestions Request

Sent when the client needs to tab-complete a `minecraft:ask_server` suggestion type.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0B | Play | Server | Transaction Id | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The id of the transaction that the server will send back to the client in the response of this packet. Client generates this and increments it each time it sends another tab completion that doesn't get a response. |
| Text | [String](https://wiki.vg/Protocol#Type:String) (32500) | All text behind the cursor without the `/` (e.g. to the left of the cursor in left-to-right languages like English). |

#### Acknowledge Configuration

Sent by the client upon receiving a [Start Configuration](https://wiki.vg/Protocol#Start_Configuration) packet from the server.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0C | Play | Server | _no fields_ |

This packet switches the connection state to [configuration](https://wiki.vg/Protocol#Configuration).

#### Click Container Button

Used when clicking on window buttons. Until 1.14, this was only used by enchantment tables.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0D | Play | Server | Window ID | [Byte](https://wiki.vg/Protocol#Type:Byte) | The ID of the window sent by [Open Screen](https://wiki.vg/Protocol#Open_Screen). |
| Button ID | [Byte](https://wiki.vg/Protocol#Type:Byte) | Meaning depends on window type; see below. |

| Window type | ID | Meaning |
| --- | --- | --- |
| Enchantment Table | 0 | Topmost enchantment. |
| 1 | Middle enchantment. |
| 2 | Bottom enchantment. |
| Lectern | 1 | Previous page (which does give a redstone output). |
| 2 | Next page. |
| 3 | Take Book. |
| 100+page | Opened page number - 100 + number. |
| Stonecutter | Recipe button number - 4\*row + col. Depends on the item. |
| Loom | Recipe button number - 4\*row + col. Depends on the item. |

#### Click Container

This packet is sent by the client when the player clicks on a slot in a window.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0E | Play | Server | Window ID | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | The ID of the window which was clicked. 0 for player inventory. The server ignores any packets targeting a Window ID other than the current one, including ignoring 0 when any other window is open. |
| State ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The last received State ID from either a [Set Container Slot](https://wiki.vg/Protocol#Set_Container_Slot) or a [Set Container Content](https://wiki.vg/Protocol#Set_Container_Content) packet. |
| Slot | [Short](https://wiki.vg/Protocol#Type:Short) | The clicked slot number, see below. |
| Button | [Byte](https://wiki.vg/Protocol#Type:Byte) | The button used in the click, see below. |
| Mode | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Inventory operation mode, see below. |
| Length of the array | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Maximum value for Notchian server is 128 slots. |
| Array of changed slots | Slot number | [Array](https://wiki.vg/Protocol#Type:Array) (128) | [Short](https://wiki.vg/Protocol#Type:Short) |  |
| Slot data | [Slot](https://wiki.vg/Protocol#Type:Slot) | New data for this slot, in the client's opinion; see below. |
| Carried item | [Slot](https://wiki.vg/Slot_Data "Slot Data") | Item carried by the cursor. Has to be empty (item ID = -1) for drop mode, otherwise nothing will happen. |

See [Inventory](https://wiki.vg/Inventory "Inventory") for further information about how slots are indexed.

After performing the action, the server compares the results to the slot change information included in the packet, as applied on top of the server's view of the container's state prior to the action. For any slots that do not match, it sends [Set Container Slot](https://wiki.vg/Protocol#Set_Container_Slot) packets containing the correct results. If State ID does not match the last ID sent by the server, it will instead send a full [Set Container Content](https://wiki.vg/Protocol#Set_Container_Content) to resynchronize the client.

When right-clicking on a stack of items, half the stack will be picked up and half left in the slot. If the stack is an odd number, the half left in the slot will be smaller of the amounts.

The distinct type of click performed by the client is determined by the combination of the Mode and Button fields.

| Mode | Button | Slot | Trigger |
| --- | --- | --- | --- |
| 0 | 0 | Normal | Left mouse click |
| 1 | Normal | Right mouse click |
| 0 | \-999 | Left click outside inventory (drop cursor stack) |
| 1 | \-999 | Right click outside inventory (drop cursor single item) |
| 1 | 0 | Normal | Shift + left mouse click |
| 1 | Normal | Shift + right mouse click _(identical behavior)_ |
| 2 | 0 | Normal | Number key 1 |
| 1 | Normal | Number key 2 |
| 2 | Normal | Number key 3 |
| ⋮ | ⋮ | ⋮ |
| 8 | Normal | Number key 9 |
| ⋮ | ⋮ | Button is used as the slot index (impossible in vanilla clients) |
| 40 | Normal | Offhand swap key F |
| 3 | 2 | Normal | Middle click, only defined for creative players in non-player inventories. |
| 4 | 0 | Normal\* | Drop key (Q) (\* Clicked item is always empty) |
| 1 | Normal\* | Control + Drop key (Q) (\* Clicked item is always empty) |
| 5 | 0 | \-999 | Starting left mouse drag |
| 4 | \-999 | Starting right mouse drag |
| 8 | \-999 | Starting middle mouse drag, only defined for creative players in non-player inventories. |
| 1 | Normal | Add slot for left-mouse drag |
| 5 | Normal | Add slot for right-mouse drag |
| 9 | Normal | Add slot for middle-mouse drag, only defined for creative players in non-player inventories. |
| 2 | \-999 | Ending left mouse drag |
| 6 | \-999 | Ending right mouse drag |
| 10 | \-999 | Ending middle mouse drag, only defined for creative players in non-player inventories. |
| 6 | 0 | Normal | Double click |
| 1 | Normal | Pickup all but check items in reverse order (impossible in vanilla clients) |

Starting from version 1.5, “painting mode” is available for use in inventory windows. It is done by picking up stack of something (more than 1 item), then holding mouse button (left, right or middle) and dragging held stack over empty (or same type in case of right button) slots. In that case client sends the following to server after mouse button release (omitting first pickup packet which is sent as usual):

1.  packet with mode 5, slot -999, button (0 for left | 4 for right);
2.  packet for every slot painted on, mode is still 5, button (1 | 5);
3.  packet with mode 5, slot -999, button (2 | 6);

If any of the painting packets other than the “progress” ones are sent out of order (for example, a start, some slots, then another start; or a left-click in the middle) the painting status will be reset.

#### Close Container

This packet is sent by the client when closing a window.

Notchian clients send a Close Window packet with Window ID 0 to close their inventory even though there is never an [Open Screen](https://wiki.vg/Protocol#Open_Screen) packet for the inventory.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x0F | Play | Server | Window ID | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | This is the ID of the window that was closed. 0 for player inventory. |

#### Change Container Slot State

This packet is sent by the client when toggling the state of a Crafter.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x10 | Play | Server | Slot ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | This is the ID of the slot that was changed. |
| Window ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | This is the ID of the window that was changed. |
| State | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | The new state of the slot. True for enabled, false for disabled. |

#### Cookie Response (play)

Response to a [Cookie Request (play)](https://wiki.vg/Protocol#Cookie_Request_.28play.29) from the server. The Notchian server only accepts responses of up to 5 kiB in size.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x11 | Play | Server | Key | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | The identifier of the cookie. |
| Has Payload | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | The payload is only present if the cookie exists on the client. |
| Payload Length | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Length of the following byte array. |
| Payload | [Optional](https://wiki.vg/Protocol#Type:Optional) [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (5120) | The data of the cookie, if any. |

#### Serverbound Plugin Message (play)

_Main article: [Plugin channels](https://wiki.vg/Plugin_channels "Plugin channels")_

Mods and plugins can use this to send their data. Minecraft itself uses some [plugin channels](https://wiki.vg/Plugin_channel "Plugin channel"). These internal channels are in the `minecraft` namespace.

More documentation on this: [https://dinnerbone.com/blog/2012/01/13/minecraft-plugin-channels-messaging/](https://dinnerbone.com/blog/2012/01/13/minecraft-plugin-channels-messaging/)

Note that the length of Data is known only from the packet length, since the packet has no length field of any kind.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x12 | Play | Server | Channel | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Name of the [plugin channel](https://wiki.vg/Plugin_channel "Plugin channel") used to send the data. |
| Data | [Byte Array](https://wiki.vg/Protocol#Type:Byte_Array) (32767) | Any data, depending on the channel. `minecraft:` channels are documented [here](https://wiki.vg/Plugin_channel "Plugin channel"). The length of this array must be inferred from the packet length. |

In Notchian server, the maximum data length is 32767 bytes.

#### Debug Sample Subscription

Subscribes to the specified type of debug sample data, which is then sent periodically to the client via [Debug Sample](https://wiki.vg/Protocol#Debug_Sample).

The Notchian server only allows subscriptions from players that are server operators.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x13 | Play | Server | Sample Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The type of debug sample to subscribe to. Can be one of the following:
-   0 - Tick time

 |

#### Edit Book

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x14 | Play | Server | Slot | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The hotbar slot where the written book is located |
| Count | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Number of elements in the following array. Maximum array size is 200. |
| Entries | [Array](https://wiki.vg/Protocol#Type:Array) (200) of [String](https://wiki.vg/Protocol#Type:String) (8192) | Text from each page. Maximum string length is 8192 chars. |
| Has title | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If true, the next field is present. true if book is being signed, false if book is being edited. |
| Title | [Optional](https://wiki.vg/Protocol#Type:Optional) [String](https://wiki.vg/Protocol#Type:String) (128) | Title of book. |

#### Query Entity Tag

Used when F3+I is pressed while looking at an entity.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x15 | Play | Server | Transaction ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | An incremental ID so that the client can verify that the response matches. |
| Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of the entity to query. |

#### Interact

This packet is sent from the client to the server when the client attacks or right-clicks another entity (a player, minecart, etc).

A Notchian server only accepts this packet if the entity being attacked/used is visible without obstruction and within a 4-unit radius of the player's position.

The target X, Y, and Z fields represent the difference between the vector location of the cursor at the time of the packet and the entity's position.

Note that middle-click in creative mode is interpreted by the client and sent as a [Set Creative Mode Slot](https://wiki.vg/Protocol#Set_Creative_Mode_Slot) packet instead.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x16 | Play | Server | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The ID of the entity to interact. Note the special case described below. |
| Type | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: interact, 1: attack, 2: interact at. |
| Target X | [Optional](https://wiki.vg/Protocol#Type:Optional) [Float](https://wiki.vg/Protocol#Type:Float) | Only if Type is interact at. |
| Target Y | [Optional](https://wiki.vg/Protocol#Type:Optional) [Float](https://wiki.vg/Protocol#Type:Float) | Only if Type is interact at. |
| Target Z | [Optional](https://wiki.vg/Protocol#Type:Optional) [Float](https://wiki.vg/Protocol#Type:Float) | Only if Type is interact at. |
| Hand | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Only if Type is interact or interact at; 0: main hand, 1: off hand. |
| Sneaking | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If the client is sneaking. |

Interaction with the ender dragon is an odd special case characteristic of release deadline–driven design. 8 consecutive entity IDs following the dragon's ID (id + 1, id + 2, ..., id + 8) are reserved for the 8 hitboxes that make up the dragon:

| ID offset | Description |
| --- | --- |
| 0 | The dragon itself (never used in this packet) |
| 1 | Head |
| 2 | Neck |
| 3 | Body |
| 4 | Tail 1 |
| 5 | Tail 2 |
| 6 | Tail 3 |
| 7 | Wing 1 |
| 8 | Wing 2 |

#### Jigsaw Generate

Sent when Generate is pressed on the [Jigsaw Block](https://minecraft.wiki/w/Jigsaw_Block) interface.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x17 | Play | Server | Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block entity location. |
| Levels | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Value of the levels slider/max depth to generate. |
| Keep Jigsaws | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Serverbound Keep Alive (play)

The server will frequently send out a keep-alive (see [Clientbound Keep Alive](https://wiki.vg/Protocol#Clientbound_Keep_Alive_.28play.29)), each containing a random ID. The client must respond with the same packet.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x18 | Play | Server | Keep Alive ID | [Long](https://wiki.vg/Protocol#Type:Long) |  |

#### Lock Difficulty

Must have at least op level 2 to use. Appears to only be used on singleplayer; the difficulty buttons are still disabled in multiplayer.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x19 | Play | Server | Locked | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Set Player Position

Updates the player's XYZ position on the server.

If the player is in a vehicle, the position is ignored (but in case of [Set Player Position and Rotation](https://wiki.vg/Protocol#Set_Player_Position_and_Rotation), the rotation is still used as normal). No validation steps other than value range clamping are performed in this case.

If the player is sleeping, the position (or rotation) is not changed, and a [Synchronize Player Position](https://wiki.vg/Protocol#Synchronize_Player_Position) is sent if the received position deviated from the server's view by more than a meter.

The Notchian server silently clamps the x and z coordinates between -30,000,000 and 30,000,000, and the y coordinate between -20,000,000 and 20,000,000. A similar condition has historically caused a kick for "Illegal position"; this is no longer the case. However, infinite or NaN coordinates (or angles) still result in a kick for `multiplayer.disconnect.invalid_player_movement`.

As of 1.20.6, checking for moving too fast is achieved like this (sic):

-   Each server tick, the player's current position is stored.
-   When the player moves, the offset from the stored position to the requested position is computed (Δx, Δy, Δz).
-   The requested movement distance squared is computed as Δx² + Δy² + Δz².
-   The baseline expected movement distance squared is computed based on the player's server-side velocity as Vx² + Vy² + Vz². The player's server-side velocity is a somewhat ill-defined quantity that includes among other things gravity, jump velocity and knockback, but _not_ regular horizontal movement. A proper description would bring much of Minecraft's physics engine with it. It is accessible as the `Motion` NBT tag on the player entity.
-   The maximum permitted movement distance squared is computed as 100 (300 if the player is using an elytra), multiplied by the number of movement packets received since the last tick, including this one, unless that value is greater than 5, in which case no multiplier is applied.
-   If the requested movement distance squared minus the baseline distance squared is more than the maximum squared, the player is moving too fast.

If the player is moving too fast, it is logged that "<player> moved too quickly! " followed by the change in x, y, and z, and the player is teleported back to their current (before this packet) server-side position.

Checking for block collisions is achieved like this:

-   A temporary collision-checked move of the player is attempted from its current position to the requested one.
-   The offset from the resulting position to the requested position is computed. If the absolute value of the offset on the y axis is less than 0.5, it (only the y component) is rounded down to 0.
-   If the magnitude of the offset is greater than 0.25 and the player isn't in creative or spectator mode, it is logged that "<player> moved wrongly!", and the player is teleported back to their current (before this packet) server-side position.
-   In addition, if the player's hitbox stationary at the requested position would intersect with a block, and they aren't in spectator mode, they are teleported back without a log message.

Checking for illegal flight is achieved like this:

-   When a movement packet is received, a flag indicating whether or not the player is floating mid-air is updated. The flag is set if the move test described above detected no collision below the player _and_ the y component of the offset from the player's current position to the requested one is greater than -0.5, unless any of various conditions permitting flight (creative mode, elytra, levitation effect, etc., but not jumping) are met.
-   Each server tick, it is checked if the flag has been set for more than 80 consecutive ticks. If so, and the player isn't currently sleeping, dead or riding a vehicle, they are kicked for `multiplayer.disconnect.flying`.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1A | Play | Server | X | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position. |
| Feet Y | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute feet position, normally Head Y - 1.62. |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position. |
| On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the client is on the ground, false otherwise. |

#### Set Player Position and Rotation

A combination of [Move Player Rotation](https://wiki.vg/Protocol#Set_Player_Rotation) and [Move Player Position](https://wiki.vg/Protocol#Set_Player_Position).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1B | Play | Server | X | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position. |
| Feet Y | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute feet position, normally Head Y - 1.62. |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position. |
| Yaw | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the X Axis, in degrees. |
| Pitch | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the Y Axis, in degrees. |
| On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the client is on the ground, false otherwise. |

#### Set Player Rotation

[![](https://wiki.vg/images/1/1e/Minecraft-trig-yaw.png)](https://wiki.vg/File:Minecraft-trig-yaw.png)

[![](https://wiki.vg/images/a/ad/Yaw.png)](https://wiki.vg/File:Yaw.png)

The unit circle of yaw, redrawn

Updates the direction the player is looking in.

Yaw is measured in degrees, and does not follow classical trigonometry rules. The unit circle of yaw on the XZ-plane starts at (0, 1) and turns counterclockwise, with 90 at (-1, 0), 180 at (0,-1) and 270 at (1, 0). Additionally, yaw is not clamped to between 0 and 360 degrees; any number is valid, including negative numbers and numbers greater than 360.

Pitch is measured in degrees, where 0 is looking straight ahead, -90 is looking straight up, and 90 is looking straight down.

The yaw and pitch of player (in degrees), standing at point (x0, y0, z0) and looking towards point (x, y, z) can be calculated with:

```
dx = x-x0
dy = y-y0
dz = z-z0
r = sqrt( dx*dx + dy*dy + dz*dz )
yaw = -atan2(dx,dz)/PI*180
if yaw &lt; 0 then
    yaw = 360 + yaw
pitch = -arcsin(dy/r)/PI*180
```

You can get a unit vector from a given yaw/pitch via:

```
x = -cos(pitch) * sin(yaw)
y = -sin(pitch)
z =  cos(pitch) * cos(yaw)
```

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1C | Play | Server | Yaw | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the X Axis, in degrees. |
| Pitch | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the Y Axis, in degrees. |
| On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the client is on the ground, false otherwise. |

#### Set Player On Ground

This packet as well as [Set Player Position](https://wiki.vg/Protocol#Set_Player_Position), [Set Player Rotation](https://wiki.vg/Protocol#Set_Player_Rotation), and [Set Player Position and Rotation](https://wiki.vg/Protocol#Set_Player_Position_and_Rotation) are called the “serverbound movement packets”. Vanilla clients will send Move Player Position once every 20 ticks even for a stationary player.

This packet is used to indicate whether the player is on ground (walking/swimming), or airborne (jumping/falling).

When dropping from sufficient height, fall damage is applied when this state goes from false to true. The amount of damage applied is based on the point where it last changed from true to false. Note that there are several movement related packets containing this state.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1D | Play | Server | On Ground | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True if the client is on the ground, false otherwise. |

#### Move Vehicle

Sent when a player moves in a vehicle. Fields are the same as in [Set Player Position and Rotation](https://wiki.vg/Protocol#Set_Player_Position_and_Rotation). Note that all fields use absolute positioning and do not allow for relative positioning.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1E | Play | Server | X | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position (X coordinate). |
| Y | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position (Y coordinate). |
| Z | [Double](https://wiki.vg/Protocol#Type:Double) | Absolute position (Z coordinate). |
| Yaw | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the vertical axis, in degrees. |
| Pitch | [Float](https://wiki.vg/Protocol#Type:Float) | Absolute rotation on the horizontal axis, in degrees. |

#### Paddle Boat

Used to _visually_ update whether boat paddles are turning. The server will update the [Boat entity metadata](https://wiki.vg/Entity_metadata#Boat "Entity metadata") to match the values here.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x1F | Play | Server | Left paddle turning | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Right paddle turning | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

Right paddle turning is set to true when the left button or forward button is held, left paddle turning is set to true when the right button or forward button is held.

#### Pick Item

Used to swap out an empty space on the hotbar with the item in the given inventory slot. The Notchian client uses this for pick block functionality (middle click) to retrieve items from the inventory.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x20 | Play | Server | Slot to use | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | See [Inventory](https://wiki.vg/Inventory "Inventory"). |

The server first searches the player's hotbar for an empty slot, starting from the current slot and looping around to the slot before it. If there are no empty slots, it starts a second search from the current slot and finds the first slot that does not contain an enchanted item. If there still are no slots that meet that criteria, then the server uses the currently selected slot.

After finding the appropriate slot, the server swaps the items and sends 3 packets:

-   [Set Container Slot](https://wiki.vg/Protocol#Set_Container_Slot) with window ID set to -2, updating the chosen hotbar slot.
-   [Set Container Slot](https://wiki.vg/Protocol#Set_Container_Slot) with window ID set to -2, updating the slot where the picked item used to be.
-   [Set Held Item](https://wiki.vg/Protocol#Set_Held_Item_.28clientbound.29), switching to the newly chosen slot.

#### Ping Request (play)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x21 | Play | Server | Payload | [Long](https://wiki.vg/Protocol#Type:Long) | May be any number. Notchian clients use a system-dependent time value which is counted in milliseconds. |

#### Place Recipe

This packet is sent when a player clicks a recipe in the crafting book that is craftable (white border).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x22 | Play | Server | Window ID | [Byte](https://wiki.vg/Protocol#Type:Byte) |  |
| Recipe | [Identifier](https://wiki.vg/Protocol#Type:Identifier) | A recipe ID. |
| Make all | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Affects the amount of items processed; true if shift is down when clicked. |

#### Player Abilities (serverbound)

The vanilla client sends this packet when the player starts/stops flying with the Flags parameter changed accordingly.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x23 | Play | Server | Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | Bit mask. 0x02: is flying. |

#### Player Action

Sent when the player mines a block. A Notchian server only accepts digging packets with coordinates within a 6-unit radius between the center of the block and the player's eyes.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x24 | Play | Server | Status | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The action the player is taking against the block (see below). |
| Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block position. |
| Face | [Byte](https://wiki.vg/Protocol#Type:Byte) [Enum](https://wiki.vg/Protocol#Type:Enum) | The face being hit (see below). |
| Sequence | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Block change sequence number (see [#Acknowledge Block Change](https://wiki.vg/Protocol#Acknowledge_Block_Change)). |

Status can be one of seven values:

| Value | Meaning | Notes |
| --- | --- | --- |
| 0 | Started digging | Sent when the player starts digging a block. If the block was instamined or the player is in creative mode, the client will _not_ send Status = Finished digging, and will assume the server completed the destruction. To detect this, it is necessary to [calculate the block destruction speed](https://minecraft.wiki/w/Breaking%23Speed) server-side. |
| 1 | Cancelled digging | Sent when the player lets go of the Mine Block key (default: left click). Face is always set to -Y. |
| 2 | Finished digging | Sent when the client thinks it is finished. |
| 3 | Drop item stack | Triggered by using the Drop Item key (default: Q) with the modifier to drop the entire selected stack (default: Control or Command, depending on OS). Location is always set to 0/0/0, Face is always set to -Y. Sequence is always set to 0. |
| 4 | Drop item | Triggered by using the Drop Item key (default: Q). Location is always set to 0/0/0, Face is always set to -Y. Sequence is always set to 0. |
| 5 | Shoot arrow / finish eating | Indicates that the currently held item should have its state updated such as eating food, pulling back bows, using buckets, etc. Location is always set to 0/0/0, Face is always set to -Y. Sequence is always set to 0. |
| 6 | Swap item in hand | Used to swap or assign an item to the second hand. Location is always set to 0/0/0, Face is always set to -Y. Sequence is always set to 0. |

The Face field can be one of the following values, representing the face being hit:

| Value | Offset | Face |
| --- | --- | --- |
| 0 | \-Y | Bottom |
| 1 | +Y | Top |
| 2 | \-Z | North |
| 3 | +Z | South |
| 4 | \-X | West |
| 5 | +X | East |

#### Player Command

Sent by the client to indicate that it has performed certain actions: sneaking (crouching), sprinting, exiting a bed, jumping with a horse, and opening a horse's inventory while riding it.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x25 | Play | Server | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Player ID |
| Action ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The ID of the action, see below. |
| Jump Boost | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Only used by the “start jump with horse” action, in which case it ranges from 0 to 100. In all other cases it is 0. |

Action ID can be one of the following values:

| ID | Action |
| --- | --- |
| 0 | Start sneaking |
| 1 | Stop sneaking |
| 2 | Leave bed |
| 3 | Start sprinting |
| 4 | Stop sprinting |
| 5 | Start jump with horse |
| 6 | Stop jump with horse |
| 7 | Open vehicle inventory |
| 8 | Start flying with elytra |

Leave bed is only sent when the “Leave Bed” button is clicked on the sleep GUI, not when waking up in the morning.

Open vehicle inventory is only sent when pressing the inventory key (default: E) while on a horse or chest boat — all other methods of opening such an inventory (involving right-clicking or shift-right-clicking it) do not use this packet.

#### Player Input

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x26 | Play | Server | Sideways | [Float](https://wiki.vg/Protocol#Type:Float) | Positive to the left of the player. |
| Forward | [Float](https://wiki.vg/Protocol#Type:Float) | Positive forward. |
| Flags | [Unsigned Byte](https://wiki.vg/Protocol#Type:Unsigned_Byte) | Bit mask. 0x1: jump, 0x2: unmount. |

Also known as 'Input' packet.

#### Pong (play)

Response to the clientbound packet ([Ping](https://wiki.vg/Protocol#Ping_.28play.29)) with the same id.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x27 | Play | Server | ID | [Int](https://wiki.vg/Protocol#Type:Int) | id is the same as the ping packet |

#### Change Recipe Book Settings

Replaces Recipe Book Data, type 1.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x28 | Play | Server | Book ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: crafting, 1: furnace, 2: blast furnace, 3: smoker. |
| Book Open | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Filter Active | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |

#### Set Seen Recipe

Sent when recipe is first seen in recipe book. Replaces Recipe Book Data, type 0.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x29 | Play | Server | Recipe ID | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |

#### Rename Item

Sent as a player is renaming an item in an anvil (each keypress in the anvil UI sends a new Rename Item packet). If the new name is empty, then the item loses its custom name (this is different from setting the custom name to the normal name of the item). The item name may be no longer than 50 characters long, and if it is longer than that, then the rename is silently ignored.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2A | Play | Server | Item name | [String](https://wiki.vg/Protocol#Type:String) (32767) | The new name of the item. |

#### Resource Pack Response (play)

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2B | Play | Server |
| UUID | [UUID](https://wiki.vg/Protocol#Type:UUID) | The unique identifier of the resource pack received in the [Add Resource Pack (play)](https://wiki.vg/Protocol#Add_Resource_Pack_.28play.29) request. |
| Result | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Result ID (see below). |

Result can be one of the following values:

| ID | Result |
| --- | --- |
| 0 | Successfully downloaded |
| 1 | Declined |
| 2 | Failed to download |
| 3 | Accepted |
| 4 | Invalid URL |
| 5 | Failed to reload |
| 6 | Discarded |

#### Seen Advancements

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2C | Play | Server | Action | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | 0: Opened tab, 1: Closed screen. |
| Tab ID | [Optional](https://wiki.vg/Protocol#Type:Optional) [Identifier](https://wiki.vg/Protocol#Type:Identifier) | Only present if action is Opened tab. |

#### Select Trade

When a player selects a specific trade offered by a villager NPC.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2D | Play | Server | Selected slot | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | The selected slot in the players current (trading) inventory. |

#### Set Beacon Effect

Changes the effect of the current beacon.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2E | Play | Server |
| Has Primary Effect | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |
| Primary Effect | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | A [Potion ID](https://minecraft.wiki/w/Potion#ID). |
| Has Secondary Effect | [Boolean](https://wiki.vg/Protocol#Type:Boolean) |  |
| Secondary Effect | [Optional](https://wiki.vg/Protocol#Type:Optional) [VarInt](https://wiki.vg/Protocol#Type:VarInt) | A [Potion ID](https://minecraft.wiki/w/Potion#ID). |

#### Set Held Item (serverbound)

Sent when the player changes the slot selection.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x2F | Play | Server | Slot | [Short](https://wiki.vg/Protocol#Type:Short) | The slot which the player has selected (0–8). |

#### Program Command Block

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x30 | Play | Server |
| Location | [Position](https://wiki.vg/Protocol#Type:Position) |  |
| Command | [String](https://wiki.vg/Protocol#Type:String) (32767) |  |
| Mode | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | One of SEQUENCE (0), AUTO (1), or REDSTONE (2). |
| Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | 0x01: Track Output (if false, the output of the previous command will not be stored within the command block); 0x02: Is conditional; 0x04: Automatic. |

#### Program Command Block Minecart

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x31 | Play | Server | Entity ID | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Command | [String](https://wiki.vg/Protocol#Type:String) (32767) |  |
| Track Output | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | If false, the output of the previous command will not be stored within the command block. |

#### Set Creative Mode Slot

While the user is in the standard inventory (i.e., not a crafting bench) in Creative mode, the player will send this packet.

Clicking in the creative inventory menu is quite different from non-creative inventory management. Picking up an item with the mouse actually deletes the item from the server, and placing an item into a slot or dropping it out of the inventory actually tells the server to create the item from scratch. (This can be verified by clicking an item that you don't mind deleting, then severing the connection to the server; the item will be nowhere to be found when you log back in.) As a result of this implementation strategy, the "Destroy Item" slot is just a client-side implementation detail that means "I don't intend to recreate this item.". Additionally, the long listings of items (by category, etc.) are a client-side interface for choosing which item to create. Picking up an item from such listings sends no packets to the server; only when you put it somewhere does it tell the server to create the item in that location.

This action can be described as "set inventory slot". Picking up an item sets the slot to item ID -1. Placing an item into an inventory slot sets the slot to the specified item. Dropping an item (by clicking outside the window) effectively sets slot -1 to the specified item, which causes the server to spawn the item entity, etc.. All other inventory slots are numbered the same as the non-creative inventory (including slots for the 2x2 crafting menu, even though they aren't visible in the vanilla client).

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x32 | Play | Server | Slot | [Short](https://wiki.vg/Protocol#Type:Short) | Inventory slot. |
| Clicked Item | [Slot](https://wiki.vg/Protocol#Type:Slot) |  |

#### Program Jigsaw Block

Sent when Done is pressed on the [Jigsaw Block](https://minecraft.wiki/w/Jigsaw_Block) interface.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x33 | Play | Server | Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block entity location |
| Name | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Target | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Pool | [Identifier](https://wiki.vg/Protocol#Type:Identifier) |  |
| Final state | [String](https://wiki.vg/Protocol#Type:String) (32767) | "Turns into" on the GUI, `final_state` in NBT. |
| Joint type | [String](https://wiki.vg/Protocol#Type:String) (32767) | `rollable` if the attached piece can be rotated, else `aligned`. |
| Selection priority | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |
| Placement priority | [VarInt](https://wiki.vg/Protocol#Type:VarInt) |  |

#### Program Structure Block

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x34 | Play | Server |
| Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block entity location. |
| Action | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | An additional action to perform beyond simply saving the given data; see below. |
| Mode | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | One of SAVE (0), LOAD (1), CORNER (2), DATA (3). |
| Name | [String](https://wiki.vg/Protocol#Type:String) (32767) |  |
| Offset X | [Byte](https://wiki.vg/Protocol#Type:Byte) | Between -48 and 48. |
| Offset Y | [Byte](https://wiki.vg/Protocol#Type:Byte) | Between -48 and 48. |
| Offset Z | [Byte](https://wiki.vg/Protocol#Type:Byte) | Between -48 and 48. |
| Size X | [Byte](https://wiki.vg/Protocol#Type:Byte) | Between 0 and 48. |
| Size Y | [Byte](https://wiki.vg/Protocol#Type:Byte) | Between 0 and 48. |
| Size Z | [Byte](https://wiki.vg/Protocol#Type:Byte) | Between 0 and 48. |
| Mirror | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | One of NONE (0), LEFT\_RIGHT (1), FRONT\_BACK (2). |
| Rotation | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | One of NONE (0), CLOCKWISE\_90 (1), CLOCKWISE\_180 (2), COUNTERCLOCKWISE\_90 (3). |
| Metadata | [String](https://wiki.vg/Protocol#Type:String) (128) |  |
| Integrity | [Float](https://wiki.vg/Protocol#Type:Float) | Between 0 and 1. |
| Seed | [VarLong](https://wiki.vg/Protocol#Type:VarLong) |  |
| Flags | [Byte](https://wiki.vg/Protocol#Type:Byte) | 0x01: Ignore entities; 0x02: Show air; 0x04: Show bounding box. |

Possible actions:

-   0 - Update data
-   1 - Save the structure
-   2 - Load the structure
-   3 - Detect size

The Notchian client uses update data to indicate no special action should be taken (i.e. the done button).

#### Update Sign

This message is sent from the client to the server when the “Done” button is pushed after placing a sign.

The server only accepts this packet after [Open Sign Editor](https://wiki.vg/Protocol#Open_Sign_Editor), otherwise this packet is silently ignored.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x35 | Play | Server | Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block Coordinates. |
| Is Front Text | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | Whether the updated text is in front or on the back of the sign |
| Line 1 | [String](https://wiki.vg/Protocol#Type:String) (384) | First line of text in the sign. |
| Line 2 | [String](https://wiki.vg/Protocol#Type:String) (384) | Second line of text in the sign. |
| Line 3 | [String](https://wiki.vg/Protocol#Type:String) (384) | Third line of text in the sign. |
| Line 4 | [String](https://wiki.vg/Protocol#Type:String) (384) | Fourth line of text in the sign. |

#### Swing Arm

Sent when the player's arm swings.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x36 | Play | Server | Hand | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Hand used for the animation. 0: main hand, 1: off hand. |

#### Teleport To Entity

Teleports the player to the given entity. The player must be in spectator mode.

The Notchian client only uses this to teleport to players, but it appears to accept any type of entity. The entity does not need to be in the same dimension as the player; if necessary, the player will be respawned in the right world. If the given entity cannot be found (or isn't loaded), this packet will be ignored. It will also be ignored if the player attempts to teleport to themselves.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x37 | Play | Server | Target Player | [UUID](https://wiki.vg/Protocol#Type:UUID) | UUID of the player to teleport to (can also be an entity UUID). |

#### Use Item On

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x38 | Play | Server | Hand | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The hand from which the block is placed; 0: main hand, 1: off hand. |
| Location | [Position](https://wiki.vg/Protocol#Type:Position) | Block position. |
| Face | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | The face on which the block is placed (as documented at [Player Action](https://wiki.vg/Protocol#Player_Action)). |
| Cursor Position X | [Float](https://wiki.vg/Protocol#Type:Float) | The position of the crosshair on the block, from 0 to 1 increasing from west to east. |
| Cursor Position Y | [Float](https://wiki.vg/Protocol#Type:Float) | The position of the crosshair on the block, from 0 to 1 increasing from bottom to top. |
| Cursor Position Z | [Float](https://wiki.vg/Protocol#Type:Float) | The position of the crosshair on the block, from 0 to 1 increasing from north to south. |
| Inside block | [Boolean](https://wiki.vg/Protocol#Type:Boolean) | True when the player's head is inside of a block. |
| Sequence | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Block change sequence number (see [#Acknowledge Block Change](https://wiki.vg/Protocol#Acknowledge_Block_Change)). |

Upon placing a block, this packet is sent once.

The Cursor Position X/Y/Z fields (also known as in-block coordinates) are calculated using raytracing. The unit corresponds to sixteen pixels in the default resource pack. For example, let's say a slab is being placed against the south face of a full block. The Cursor Position X will be higher if the player was pointing near the right (east) edge of the face, lower if pointing near the left. The Cursor Position Y will be used to determine whether it will appear as a bottom slab (values 0.0–0.5) or as a top slab (values 0.5-1.0). The Cursor Position Z should be 1.0 since the player was looking at the southernmost part of the block.

Inside block is true when a player's head (specifically eyes) are inside of a block's collision. In 1.13 and later versions, collision is rather complicated and individual blocks can have multiple collision boxes. For instance, a ring of vines has a non-colliding hole in the middle. This value is only true when the player is directly in the box. In practice, though, this value is only used by scaffolding to place in front of the player when sneaking inside of it (other blocks will place behind when you intersect with them -- try with glass for instance).

#### Use Item

Sent when pressing the Use Item key (default: right click) with an item in hand.

| Packet ID | State | Bound To | Field Name | Field Type | Notes |
| --- | --- | --- | --- | --- | --- |
| 0x39 | Play | Server | Hand | [VarInt](https://wiki.vg/Protocol#Type:VarInt) [Enum](https://wiki.vg/Protocol#Type:Enum) | Hand used for the animation. 0: main hand, 1: off hand. |
| Sequence | [VarInt](https://wiki.vg/Protocol#Type:VarInt) | Block change sequence number (see [#Acknowledge Block Change](https://wiki.vg/Protocol#Acknowledge_Block_Change)). |