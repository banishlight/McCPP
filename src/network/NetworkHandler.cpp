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

            // Add connection to the ConnectionList
            ConnectionList::getList().addConnection(newConn);

            // Enqueue a task to process this connection
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
    // Check if the connection is still valid
    if (!conn.isConnected()) {
        Console::getConsole().Entry("Connection closed.");
        return;  // Exit this task
    }

    // Attempt to decode a single packet
    if (conn.processPacket()) { 
        // Successfully processed a packet, requeue for further processing
        netThreads->enqueue([this,conn]() mutable {
            processConnection(conn);
        });
    } else {
        // No packet available; requeue after a short delay
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        netThreads->enqueue([this,conn]() mutable {
            processConnection(conn);
        });
    }
}