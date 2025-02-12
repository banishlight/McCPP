# Documentation of McCPP

## TODO
- Finish readData functions to handle all data types
- Extend connection processing to send packets while they are being processed.
- Complete all packet handling to send recieve the required packets for the player to recieve the server information
    - may need to import a json library [like this](https://github.com/nlohmann/json)
- Structure a general commands system to allow for externally made commands and for new commands to be easily appended when needed.  (Some sort of lookup table?)
- the Console class is not thread safe.  It currently relies on cout buffering.



## CUDA Documentation
- Found post about linking Cuda code with C++ code [here](https://stackoverflow.com/questions/9421108/how-can-i-compile-cuda-code-then-link-it-to-a-c-project)

## G++ Flags
- This page explains the possible optimization flags for g++ [here](https://clang.llvm.org/docs/CommandGuide/clang.html#code-generation-options)

## Minecraft Documentation
Much of the work done deconstructing minecraft and its network protocol has been done and documented on wiki.vg.  Sadly wiki.vg has been taken down by the owner and the pages have been moved [here](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge).  Included in this repo I have a markdown copy of the protocol documentation as a contingency.

## Networking
This server handles networking by creating a thread that will focus on accepting new connections and adding them to the network queue(Round robin scheduler).  A thread pool then pops these connections off the queue, sends and recieves any packets they need and then add them to the back of the queue.  When the connection is closed, it is processed and then it is not added back into to the queue.  This is all orchestrated in the NetworkHandler class.

## JSON Parsing
Using [this](https://github.com/nlohmann/json) library to parse json's.  Its a header library in the ```include/lin``` directory.  