# MC++
By Cuba Giesbrecht and Aiden Waring

## Summary
The goal of this project is to write a minecraft server client focused around being high performance and multithreaded.  The server will be written in C++ to give the performance of a compiled language while also utilizing the performance of explicit memory management (no garbage collection).  The server client will allow vanilla java edition player clients to connect to the server seamlessly.

## Feature goals
1. Handle Java edition TCP handshakes.
2. Recreate all Vanilla gameplay mechanics.
3. Multithread common performance issues in the vanilla Minecraft server client (Multithreading certain tasks may cause vanilla gameplay interactions to not perform as they usually do, we will decide which interactions can be preserved versus performance).
    1. Chunk loading/generation
    2. Lighting
    3. AI pathing
    4. Entity handling
4. Create Java wrapper functions to allow for plugins to be written in Java just as traditional Java edition server clients do (i.e. [Bukkit](https://dev.bukkit.org) and [Spigot](https://www.spigotmc.org)).
5. Offload work to a GPU array (see [CUDA Programming](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)).