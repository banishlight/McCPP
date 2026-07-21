#include <commands/OpCommand.hpp>
#include <CommandSender.hpp>
#include <OpsList.hpp>
#include <Properties.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/packets/Play.hpp>
#include <network/PacketUtils.hpp>
#include <memory>

void OpCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    if (args.empty()) {
        sender.sendMessage("Usage: /op <player>");
        return;
    }
    const string& targetName = args[0];

    // Only currently-connected players can be opped -- there's no offline
    // username-to-UUID cache to resolve anyone else by name.
    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        Player& player = conn->getPlayer();
        if (player.getUsername() != targetName) continue;

        string uuidHex = uuidToHexString(player.getUUID());
        int newLevel = Properties::getProperties().op_permission_level;
        OpsList::getInstance().setOpLevel(uuidHex, targetName, newLevel);
        // Commands_p is only ever otherwise sent once, at join -- resend it
        // now so the target's chat autocomplete reflects newly-granted
        // commands immediately instead of only after a reconnect.
        conn->addPacket(std::make_shared<Commands_p>(conn->getCompressionThreshold(), newLevel));
        sender.sendMessage("Made " + targetName + " a server operator.");
        return;
    }
    sender.sendMessage("No such player online: " + targetName);
}

string OpCommand::getName() const {
    return "op";
}

string OpCommand::getDescription() const {
    return "Grants a connected player operator permissions";
}
