#include <iostream>

#include <pthread.h>
#include <Standards.hpp>
#include <Properties.hpp>
#include <network/Connection.hpp>

int main()
{
    std::cout << "A C++ Minecraft Server" << std::endl;
    //const char* ip = "15.235.13.44";
    // ProcessHandshake(764,25565,ip);

    // Initialize Configs
    Properties& myProperties = Properties::getProperties();

    // Begin Connection handler
    ConnectionList& myList = ConnectionList::getList();

    // Initialize World

    // Create listening thread

    // Create worker threads

    
    return 0;
}

