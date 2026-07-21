#include <commands/DeopCommand.hpp>
#include <CommandSender.hpp>
#include <OpsList.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/PacketUtils.hpp>

void DeopCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    if (args.empty()) {
        sender.sendMessage("Usage: /deop <player>");
        return;
    }
    const string& targetName = args[0];

    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        Player& player = conn->getPlayer();
        if (player.getUsername() != targetName) continue;

        string uuidHex = uuidToHexString(player.getUUID());
        OpsList::getInstance().setOpLevel(uuidHex, targetName, 0);
        sender.sendMessage("Made " + targetName + " no longer a server operator.");
        return;
    }
    sender.sendMessage("No such player online: " + targetName);
}

string DeopCommand::getName() const {
    return "deop";
}

string DeopCommand::getDescription() const {
    return "Revokes a connected player's operator permissions";
}
