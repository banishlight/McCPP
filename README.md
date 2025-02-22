# MC++
By Cuba Giesbrecht and Aiden Waring

## Summary
The goal of this project is to write a minecraft server client focused around being high performance and multithreaded.  The server will be written in C++ to give the performance of a compiled language while also utilizing the performance of explicit memory management (no garbage collection).  The server client will allow vanilla java edition player clients to connect to the server seamlessly.

## Feature Goals
(We will be targeting Minecraft Java version 1.20.2 as our working version to start.)
1. Handle Java edition TCP handshakes.
2. Recreate all Vanilla gameplay mechanics.
3. Multithread common performance issues in the vanilla Minecraft server client (Multithreading certain tasks may cause vanilla gameplay interactions to not perform as they usually do, we will decide which interactions can be preserved versus performance).
    1. Chunk loading/generation
    2. Lighting
    3. AI pathing
    4. Entity handling
4. Create Java wrapper functions to allow for plugins to be written in Java just as traditional Java edition server clients do (i.e. [Bukkit](https://dev.bukkit.org) and [Spigot](https://www.spigotmc.org)).
5. Create Java wrapper functions for mods to be written with an API similar enough to [Fabric](https://github.com/FabricMC/fabric) that it would allow existing mods to be easily or seamlessly ported to our server.
6. Offload work to a GPU array (see [CUDA Programming](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)).

## Technical Goals & Requirements
- Works across platforms Linux and Windows (Only supports Linux right now)
- Simple build process.  The project must use near zero external libraries.  A user should be able to download the code and build it easily.  External libraries may be acceptable for non-release version such as [Google Tests](https://github.com/google/googletest) for testing the program.

## Supported Platforms
This project is aimed at being a Linux(x86-64) server client as this is the primary operating system for server hosting.  It will support Windows in the future.

## Build Guide
To build this project you just have to run:
```
make
```
and it will create the ```bin``` folder containing the binary.

To remove the build files and executable run:
```
make clean
```

Note: This is being designed for the x64 Linux platform

To run:
```
cd bin/
./MinecraftServer
```

### Extended build options
Note: You may attempt to build with these other options if you so wish

This option has not been thouroughly tested at the moment, but is available:
```
make fast
```

If you are having issues I may ask for you to build with:
```
make debug
```
So that I can help diagnose the issue.


## Task List
- [ ] Networking Handles all Packets up to the Play State
- [ ] Networking Handles all Play State packets
- [ ] Player can load into void world
- [ ] Player can load into flat world
