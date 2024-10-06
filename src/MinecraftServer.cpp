#include <iostream>
#include <thread>
#include <Standards.hpp>
#include <Properties.hpp>
#include <network/Connection.hpp>
#include <network/AcceptChild.hpp>
#include <network/CubSock.hpp>
#include <Console.hpp>

int main()
{
    Console& myConsole = Console::getConsole();
    myConsole.Entry("A C++ Minecraft Server");
    Properties& myProperties = Properties::getProperties();
    ConnectionList& myList = ConnectionList::getList();

    // Init listen() here, accept connections in child thread
    CubSock serverSock;
    serverSock.Listen(myProperties.getIP(), myProperties.query_port);
    // Create RecievingThread
    AcceptChild myAccept = AcceptChild();
    //std::thread childThread(myAccept.begin);
    
    // Create worker threads

    // Initialize World
    
    return 0;
}

