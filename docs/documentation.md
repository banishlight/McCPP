# Documentation of McCPP

## TODO




## CUDA Documentation
- Found post about linking Cuda code with C++ code [here](https://stackoverflow.com/questions/9421108/how-can-i-compile-cuda-code-then-link-it-to-a-c-project)

## G++ Flags
- This page explains the possible optimization flags for g++ [here](https://clang.llvm.org/docs/CommandGuide/clang.html#code-generation-options)

## Minecraft Documentation
Much of the work done deconstructing minecraft and its network protocol has been done and documented on wiki.vg.  Sadly wiki.vg has been taken down by the owner and the pages have been moved [here](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge).  Included in this repo I have a markdown copy of the protocol documentation as a contingency.

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