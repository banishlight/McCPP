#include <Standards.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/Socket.hpp>
#include <network/ServerSocket.hpp>
#include <Console.hpp>
#include <Properties.hpp>
#include <ThreadPool.hpp>
#include <network/Crypto.hpp>
#include <network/packets/Play.hpp>
#include <entities/PlayerVisibilityManager.hpp>
#include <World.hpp>
#include <PlayerDataPersistence.hpp>
#include <thread>
#include <algorithm>


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
    // Generate the server's RSA keypair once, eagerly, so no connection can
    // race generatePublicKey()'s first-call memoization once real traffic starts.
    generatePublicKey();
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
    running = false; // Stop the server thread loop
    // reset() (not a manual destructor call) so these aren't destroyed a second
    // time when the unique_ptrs themselves are torn down with ConnectionManager.
    _serverSocket.reset(); // shuts down + closes the listening socket, unblocking Accept()
    if (_serverConThread.joinable()) _serverConThread.join();
    _conThreads.reset();
    _initialized = false;
}

void ConnectionManager::processConnection(std::shared_ptr<Connection> conn) {
    // Computed before the try block below (isValid() itself can't throw) --
    // used to tell apart "processing threw" from "the disconnect cleanup
    // itself threw", since those need different recovery (see the catch
    // blocks below).
    bool wasDisconnecting = !conn->isValid();
    try {
        if (wasDisconnecting) {
            // Only Play-state connections were ever announced to anyone (tab
            // list + in-world visibility), so only those need an announced exit.
            if (conn->getState() == ConnectionState::Play) {
                std::vector<std::vector<long>> leavingUuid{conn->getPlayer().getUUID()};
                for (auto& other : getActiveConnections()) {
                    if (!other || other.get() == conn.get()) continue;
                    if (other->getState() != ConnectionState::Play) continue;
                    other->addPacket(std::make_shared<Player_Info_Remove_p>(other->getCompressionThreshold(), leavingUuid));
                }
                PlayerVisibilityManager::getInstance().handleDisconnect(conn);
                // Nothing else decrements this player's view of their loaded
                // chunks on disconnect -- without this, every chunk they ever saw
                // would stay "viewed" forever as far as ChunkUnloadSystem is concerned.
                World& world = World::getInstance();
                for (auto& [x, z] : conn->getPlayer().getLoadedChunks()) {
                    world.chunkViewerRemoved(x, z);
                }
                // Persist this player's final position/rotation/gamemode/hotbar so
                // they resume here next join -- Player is a plain value member of
                // Connection, so it's still fully populated at this point.
                PlayerDataPersistence::save(world.getWorldDir(), conn->getPlayer());
            }
            return;
        }
        conn->receivePacket();
        conn->deliverGeneratedChunks();
        conn->sendPackets();
    } catch (const std::exception& e) {
        // A single malformed/unexpected packet used to be able to crash the
        // whole server for every connected player (found via a real crash --
        // an uncaught exception during packet processing propagated all the
        // way up through this recursive task and called std::terminate).
        // Dropping just this one connection instead is the fix; giving up
        // outright (rather than retrying) if the *disconnect cleanup itself*
        // is what threw avoids an infinite retry loop against a cleanup that
        // will just keep failing the same way.
        Console::getConsole().Error(string("ConnectionManager::processConnection(): Unhandled exception")
            + (wasDisconnecting ? " during disconnect cleanup -- giving up on this connection: " : " -- closing this connection rather than crashing the server: ")
            + e.what());
        if (wasDisconnecting) return;
        conn->forceClose();
    } catch (...) {
        Console::getConsole().Error(string("ConnectionManager::processConnection(): Unknown unhandled exception")
            + (wasDisconnecting ? " during disconnect cleanup -- giving up on this connection." : " -- closing this connection rather than crashing the server."));
        if (wasDisconnecting) return;
        conn->forceClose();
    }
    // An idle connection re-enqueues itself forever; without this check the task
    // queue never empties, so ThreadPool::~ThreadPool() can never join its workers
    // while any client is still connected.
    if (!running) return;
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
            {
                std::lock_guard<std::mutex> lock(_connectionsMutex);
                _connections.push_back(newConn);
            }
            _conThreads->enqueue([this, newConn]() mutable {
                    processConnection(newConn);
            });
        }
    }
}

std::vector<std::shared_ptr<Connection>> ConnectionManager::getActiveConnections() {
    std::lock_guard<std::mutex> lock(_connectionsMutex);
    _connections.erase(
        std::remove_if(_connections.begin(), _connections.end(),
            [](const std::weak_ptr<Connection>& conn) { return conn.expired(); }),
        _connections.end()
    );
    std::vector<std::shared_ptr<Connection>> active;
    active.reserve(_connections.size());
    for (const auto& conn : _connections) {
        active.push_back(conn.lock());
    }
    return active;
}