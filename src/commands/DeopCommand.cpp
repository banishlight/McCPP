#include <commands/DeopCommand.hpp>
#include <CommandSender.hpp>
#include <OpsList.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/packets/Play.hpp>
#include <network/PacketUtils.hpp>
#include <memory>

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
        // Commands_p is only ever otherwise sent once, at join -- resend it
        // now so the target's chat autocomplete drops the commands they just
        // lost immediately instead of only after a reconnect.
        conn->addPacket(std::make_shared<Commands_p>(conn->getCompressionThreshold(), 0));
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
