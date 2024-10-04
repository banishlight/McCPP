#include <iostream>
#include <thread>
#include <Standards.hpp>
#include <Properties.hpp>
#include <network/Connection.hpp>
#include <network/ListenChild.hpp>

int main()
{
    std::cout << "A C++ Minecraft Server" << std::endl;

    // Initialize Configs
    Properties& myProperties = Properties::getProperties();

    // Init Connection handler
    ConnectionList& myList = ConnectionList::getList();

    // Initialize World

    // Create RecievingThread
    // Init listen() here, accept connections in child thread
    TCPSocket::Bind();
    AcceptChild myAccept = new AcceptChild();
    std::thread childThread(myAccept.begin());
    
    // Create worker threads

    
    return 0;
}

