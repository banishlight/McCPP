#include <Standards.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>
#include <Console.hpp>
#include <Properties.hpp>
#include <ThreadPool.hpp>
#include <network/Crypto.hpp>
#include <thread>


ConnectionManager::ConnectionManager() {
    
}

ConnectionManager::~ConnectionManager() {
    close(); // Ensure resources are cleaned up
}

ConnectionManager& ConnectionManager::getInstance() {
    static ConnectionManager instance;
    return instance;
}

void ConnectionManager::initialize() {
    if (_initialized) return; // Already initialized
    // Initialize Crypto key
    initCrypto();
    // Fetch server address and make socket
    auto& props = Properties::getProperties();
    _serverSocket = std::make_unique<ServerSocket>(props.getIP(), props.getPort());
    if (!_serverSocket->isValid()) {
        // TODO: Handle error, server socket creation failed
        return;
    }
    // Create Connection Threadpool
    _conThreads = std::make_unique<ThreadPool>(THREAD_COUNT);
    // Start ServerSocket Thread
    _serverConThread = std::thread([this]() { serverThreadLoop(); });
    _initialized = true;
}

void ConnectionManager::close() {
    if (!_initialized) return; // Not initialized
    cleanupCrypto();
    if (_serverSocket) _serverSocket->~ServerSocket();
    running = false; // Stop the server thread loop
    if (_serverConThread.joinable()) _serverConThread.join();
    _conThreads->~ThreadPool();
    _initialized = false;
}

void ConnectionManager::processConnection(std::shared_ptr<Connection> conn) {
    if (!conn->isValid()) return;
    #ifdef DEBUG
        Console::getConsole().Entry("ConnectionManager::processConnection(): valid connection!");
    #endif
    conn->receivePacket();
    conn->sendPackets();
    _conThreads->enqueue([this,conn]() mutable {
            processConnection(conn);
    });
}

void ConnectionManager::serverThreadLoop() {
    while (running) {
        Socket newSock = _serverSocket->Accept();
        if (newSock.isValid()) {
            // This is kind of gross...
            std::shared_ptr<Connection> newConn = std::make_shared<Connection>(std::make_shared<Socket>(std::move(newSock)));
            _conThreads->enqueue([this, newConn]() mutable {
                    processConnection(newConn);
            });
        }
    }
}