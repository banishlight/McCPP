#include <Standards.hpp>
#include <Properties.hpp>
#include <Console.hpp>
#include <network/ConnectionManager.hpp>

int main() {
    // Console::getConsole().Entry("A C++ Minecraft Server");
    ConnectionManager::getInstance().initialize();
    return 0;
}


#ifdef TEST
// Unit test here.
int main() {



}
#endif