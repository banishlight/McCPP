#include <ConsoleCommandSender.hpp>
#include <Console.hpp>

void ConsoleCommandSender::sendMessage(const string& message) {
    Console::getConsole().Entry(message);
}
