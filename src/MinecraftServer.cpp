#include <Standards.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ConnectionManager.hpp>

int main() {
    Console::getConsole().Entry("A C++ Minecraft Server");
    Properties::getProperties().initialize();
    Console::getConsole().Entry("Properties loaded successfully.");
    ConnectionManager::getInstance().initialize();
    while (true) {
        // Main server loop
        // This could be replaced with a more sophisticated event loop
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}


#ifdef TEST
// Unit test here.
int main() {



}
#endif