#include <commands/KickCommand.hpp>
#include <CommandSender.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <sstream>

void KickCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    if (args.empty()) {
        sender.sendMessage("Usage: /kick <player> [reason]");
        return;
    }
    const string& targetName = args[0];

    string reason = "Kicked by an operator.";
    if (args.size() > 1) {
        std::ostringstream oss;
        for (size_t i = 1; i < args.size(); i++) {
            if (i > 1) oss << " ";
            oss << args[i];
        }
        reason = oss.str();
    }

    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        if (conn->getPlayer().getUsername() != targetName) continue;

        conn->disconnect(reason);
        sender.sendMessage("Kicked " + targetName + ": " + reason);
        return;
    }
    sender.sendMessage("No such player online: " + targetName);
}

string KickCommand::getName() const {
    return "kick";
}

string KickCommand::getDescription() const {
    return "Disconnects a connected player, optionally with a reason";
}
