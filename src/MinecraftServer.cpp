#include <Standards.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ConnectionManager.hpp>
#include <vanilla/VanillaDataManager.hpp>
#include <CommandRegistry.hpp>
#include <ServerControl.hpp>
#include <TickLoop.hpp>
#include <WorldWorkerPool.hpp>
#include <sstream>

int main() {
    Console::getConsole().Entry("A C++ Minecraft Server");
    Properties::getProperties().initialize();
    Console::getConsole().Entry("Properties loaded successfully.");
    VanillaDataManager::getInstance().initialize();
    // Must be initialized before ConnectionManager: singletons are torn down
    // in reverse construction order at exit, and WorldWorkerPool's queued
    // chunk-generation tasks hold shared_ptr<Connection> captures that must
    // stay safe to run even after ConnectionManager itself is gone (they only
    // touch the Connection object directly, kept alive by that shared_ptr).
    WorldWorkerPool::getInstance().initialize();
    ConnectionManager::getInstance().initialize();
    // Must be initialized after ConnectionManager: singletons are torn down in
    // reverse construction order at exit, so TickLoop's thread (which reaches
    // into ConnectionManager every tick) needs to stop before ConnectionManager does.
    TickLoop::getInstance().initialize();
    CommandRegistry::getInstance().initialize();
    while (!ServerControl::isShutdownRequested()) {
        // Main server loop
        // This could be replaced with a more sophisticated event loop
        string line;
        if (Console::getConsole().Post(line)) {
            std::istringstream iss(line);
            string name;
            iss >> name;
            std::vector<string> args;
            string arg;
            while (iss >> arg) {
                args.push_back(arg);
            }
            if (!CommandRegistry::getInstance().dispatch(name, args)) {
                Console::getConsole().Entry("Unknown command: " + name);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}