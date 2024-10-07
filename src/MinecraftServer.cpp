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
    Console::getConsole().Entry("A C++ Minecraft Server");
    Properties::getProperties(); // init properties first
    ConnectionList& myList = ConnectionList::getList();

    // Init listen() here, accept connections in child thread
    CubSock serverSock;
    serverSock.Listen(Properties::getProperties().getIP(), Properties::getProperties().query_port);
    // Create RecievingThread
    AcceptChild myAccept = AcceptChild();
    //std::thread childThread(myAccept.begin);
    
    // Create worker threads

    // Initialize World
    
    return 0;
}

