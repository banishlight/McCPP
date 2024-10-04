#include <iostream>
#include <thread>
#include <Standards.hpp>
#include <Properties.hpp>
#include <network/Connection.hpp>
#include <network/AcceptChild.hpp>

int main()
{
    std::cout << "A C++ Minecraft Server" << std::endl;

    // Initialize Configs
    Properties& myProperties = Properties::getProperties();

    // Init Connection handler
    ConnectionList& myList = ConnectionList::getList();

    // Initialize World

    // Init listen() here, accept connections in child thread

    // Create RecievingThread
    AcceptChild myAccept = AcceptChild();
    //std::thread childThread(myAccept.begin);
    
    // Create worker threads

    
    return 0;
}

