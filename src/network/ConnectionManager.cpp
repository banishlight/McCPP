#include <Standards.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>
#include <thread>
#include <ThreadPool.hpp>


ConnectionManager::ConnectionManager() {

}

ConnectionManager::~ConnectionManager() {

}

ConnectionManager& ConnectionManager::getInstance() {
    static ConnectionManager instance;
    return instance;
}

void ConnectionManager::initialize() {
    // Initialize the server socket
    _serverSocket = ServerSocket();
    // Create Connection Threadpool
    _conThreads = ThreadPool(THREAD_COUNT);
    // Start ServerSocket Thread
    _serverConThread = std::thread([this]() { serverThreadLoop(); });
}

void ConnectionManager::processConnection(Connection conn) {
    if (!conn.isValid()) return;
    conn.receivePackets();
    // TODO, Send pending packets here
    _conThreads->enqueue([this,conn]() mutable {
            processConnection(conn);
    });
}

void ConnectionManager::serverThreadLoop() {
    Socket newConn = _serverSocket.Accept();
    if (newConn.isValid()) {
        // Add to connection pool
        _conThreads->enqueue([this, newConn]() mutable {
                processConnection(newConn);
        });
    }
}