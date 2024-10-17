#include <iostream>
#include <thread>
#include <Standards.hpp>
#include <Properties.hpp>
#include <network/Connection.hpp>
#include <network/CubSock.hpp>
#include <Console.hpp>

int main() {
    Console::getConsole().Entry("A C++ Minecraft Server");
    Properties& myProperties = Properties::getProperties(); // init properties first
    CubSock serverSock;

    if (serverSock.Listen(myProperties.getIP(), myProperties.getPort())) {
        Console::getConsole().Error("Failed to bind port on: " + myProperties.getIP() + ":" + myProperties.getPort());
        return -1;
    }
    
    
    return 0;
}

