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
    ConnectionList::getList().setListenfd(listen_fd);
    
    while(true) {
        if (Console::getConsole().Command() == 1) { break; }
    }
    // Clean up memory and threads here
    return 0;
}