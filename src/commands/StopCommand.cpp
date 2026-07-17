#include <commands/StopCommand.hpp>
#include <ServerControl.hpp>
#include <Console.hpp>

void StopCommand::execute(const std::vector<string>& args) {
    Console::getConsole().Entry("Stopping server...");
    ServerControl::requestShutdown();
}

string StopCommand::getName() const {
    return "stop";
}

string StopCommand::getDescription() const {
    return "Shuts down the server";
}
