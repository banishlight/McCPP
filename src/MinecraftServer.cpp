#include <Standards.hpp>
#include <Properties.hpp>
#include <network/CubaSocket.hpp>
#include <Console.hpp>
#include <network/NetworkHandler.hpp>

int main() {
    Console::getConsole().Entry("A C++ Minecraft Server");
    
    NetworkHandler::getHandler(); // Init Network

    Console::getConsole().Entry("Ready!");
    while(true) {
        #ifdef DEBUG
            Console::getConsole().Entry("Waiting on Command!");
        #endif
        if (Console::getConsole().Command() == 1) { break; }
    }
    // Clean up memory and threads here
    NetworkHandler::getHandler().close();
    return 0;
}
