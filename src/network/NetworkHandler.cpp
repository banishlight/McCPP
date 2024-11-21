#include <network/NetworkHandler.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/CubSock.hpp>
#include <network/Connection.hpp>
#include <ThreadPool.hpp>

NetworkHandler::NetworkHandler() {
    this->initNetwork();
    acceptConnections = true;
}

NetworkHandler::~NetworkHandler() {
    acceptConnections = false;
    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    delete netThreads;
}

NetworkHandler& NetworkHandler::getHandler() {
    static NetworkHandler singleton;
    return singleton;
}

int NetworkHandler::initNetwork() {
    // Begin listening on IP
    Properties& myProperties = Properties::getProperties();
    int listen_fd = Listen(myProperties.getIP(), myProperties.getPort());
    if (listen_fd < 0) {
        Console::getConsole().Error("Failed to bind port on: " + myProperties.getIP() + ":" + myProperties.getPort());
        return -1;
    }
    ConnectionList::getList().setListenfd(listen_fd);
    netThreads = new ThreadPool(4);
    Console::getConsole().Entry("Thread Pool Created");
    // Create Thread to manage pool
    return 0;
}

void NetworkHandler::acceptConnectionsLoop() {
    Console::getConsole().Entry("Accepting connections...");
    int listen_fd = ConnectionList::getList().getListenfd();

    while (acceptConnections) {
        int clientSock = Accept(listen_fd);
        if (clientSock != -1) {
            Console::getConsole().Entry("Accepted new connection.");
            Connection newConn(clientSock);
            ConnectionList::getList().addConnection(newConn);
            netThreads->enqueue([this, newConn]() mutable {
                processConnection(newConn);
            });
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Avoid busy waiting
        }
    }
    Console::getConsole().Entry("Stopped accepting connections.");
}

void NetworkHandler::processConnection(Connection conn) {
    if (!conn.isConnected()) {
        Console::getConsole().Entry("Connection closed.");
        // Remove connection from list
        return;  
    }
    // Attempt to decode a single packet
    if (!conn.processPacket()) { 
        // Avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    netThreads->enqueue([this,conn]() mutable {
            processConnection(conn);
        });
}