#include <network/NetworkHandler.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/CubSock.hpp>
#include <network/Connection.hpp>
#include <ThreadPool.hpp>

NetworkHandler::NetworkHandler() {
    this->initNetwork();
}

NetworkHandler::~NetworkHandler() {
    
}

NetworkHandler& NetworkHandler::getHandler() {
    static NetworkHandler singleton;
    return singleton;
}

void NetworkHandler::close() {
    if (needInit) return;
    #ifdef DEBUG
        Console::getConsole().Entry("Network Handler closing");
    #endif
    acceptConnections = false;
    while (!acceptThread.joinable()) {
        #ifdef DEBUG
            Console::getConsole().Entry("Spin waiting for Accept thread");
        #endif
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    acceptThread.join();
    #ifdef DEBUG
        Console::getConsole().Entry("Accept Thread joined");
    #endif
    delete netThreads;
}

int NetworkHandler::initNetwork() {
    if(!needInit) {
        Console::getConsole().Entry("Network Already Initialized!");
        return 0;
    }
    Properties& myProperties = Properties::getProperties();
    int listen_fd = Listen(myProperties.getIP(), myProperties.getPort());
    if (listen_fd < 0) {
        Console::getConsole().Error("Failed to get Listen FD while initializing network.");
        return -1;
    }
    ConnectionList::getList().setListenfd(listen_fd);
    netThreads = new ThreadPool(4);
    Console::getConsole().Entry("Network Thread Pool Created");
    // could potentially move this loop onto the Network Thread pool (for smaller servers?)
    acceptConnections = true;
    acceptThread = std::thread([this]() { acceptConnectionsLoop(); });
    needInit = false;
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
            // Avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
        }
    }
    Console::getConsole().Entry("Stopped accepting connections.  Accept thread closing.");
}

void NetworkHandler::processConnection(Connection conn) {
    if (!conn.isConnected()) {
        Console::getConsole().Entry("Connection closed.");
        ConnectionList::getList().removeConnection(conn);
        return;  
    }
    // Attempt to decode a single packet
    if (packetReady(conn.getFD())) {
        bool success = conn.processPacket();
        if (!success) {
            Console::getConsole().Error("processPacket() Failure");
        }
    }
    /*
    if (!conn.processPacket()) { 
        // Avoid busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    */
    netThreads->enqueue([this,conn]() mutable {
            processConnection(conn);
        });
}