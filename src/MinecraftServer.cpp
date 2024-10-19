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

    int listen_fd = Listen(myProperties.getIP(), myProperties.getPort());
    if (listen_fd) {
        Console::getConsole().Error("Failed to bind port on: " + myProperties.getIP() + ":" + myProperties.getPort());
        return -1;
    }
    Connection myConnection = Connection(listen_fd);
    
    while(true) {
        if (Console::getConsole().Command() == 1) { return 0; }
    }
    
    return 0;
}

