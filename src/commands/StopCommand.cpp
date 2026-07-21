#include <commands/StopCommand.hpp>
#include <ServerControl.hpp>
#include <CommandSender.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <Console.hpp>

void StopCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    Console::getConsole().Entry("Stopping server (requested by " + sender.getName() + ")...");
    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        conn->disconnect("Server closed");
    }
    ServerControl::requestShutdown();
}

string StopCommand::getName() const {
    return "stop";
}

string StopCommand::getDescription() const {
    return "Shuts down the server";
}
