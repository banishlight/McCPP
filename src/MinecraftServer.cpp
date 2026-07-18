#include <Standards.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ConnectionManager.hpp>
#include <vanilla/VanillaDataManager.hpp>
#include <CommandRegistry.hpp>
#include <ServerControl.hpp>
#include <TickLoop.hpp>
#include <WorldWorkerPool.hpp>
#include <World.hpp>
#include <sstream>

int main() {
    Console::getConsole().Entry("A C++ Minecraft Server");
    Properties::getProperties().initialize();
    Console::getConsole().Entry("Properties loaded successfully.");
    VanillaDataManager::getInstance().initialize();
    // Construction order matters here (reverse-destruction hazard for pool
    // tasks capturing raw pointers) -- see docs/general-documentation.md,
    // "Singleton construction/destruction order".
    World::getInstance();
    WorldWorkerPool::getInstance().initialize();
    ConnectionManager::getInstance().initialize();
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