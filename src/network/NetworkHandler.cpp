#include <network/NetworkHandler.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/CubSock.hpp>
#include <network/Connection.hpp>
#include <ThreadPool.hpp>

NetworkHandler::NetworkHandler() {
    // Begin listening on IP
    Properties& myProperties = Properties::getProperties();
    int listen_fd = Listen(myProperties.getIP(), myProperties.getPort());
    if (listen_fd < 0) {
        Console::getConsole().Error("Failed to bind port on: " + myProperties.getIP() + ":" + myProperties.getPort());
        return;
    }
    ConnectionList::getList().setListenfd(listen_fd);

    // Create Thread pool
    ThreadPool netThreads(4);
    Console::getConsole().Entry("Thread Pool Created");
    // Create Thread to manage pool
}

NetworkHandler::~NetworkHandler() {

}

NetworkHandler& NetworkHandler::getHandler() {
    static NetworkHandler singleton;
    return singleton;
}

void NetworkHandler::close() {

}