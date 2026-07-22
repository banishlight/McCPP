#include <commands/GamemodeCommand.hpp>
#include <CommandSender.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/packets/Play.hpp>
#include <memory>

namespace {
    // -1 = invalid/unrecognized.
    int parseGamemode(const string& text) {
        if (text == "survival" || text == "0") return 0;
        if (text == "creative" || text == "1") return 1;
        if (text == "adventure" || text == "2") return 2;
        if (text == "spectator" || text == "3") return 3;
        return -1;
    }

    string gamemodeName(int gamemode) {
        switch (gamemode) {
            case 1: return "creative";
            case 2: return "adventure";
            case 3: return "spectator";
            default: return "survival";
        }
    }
}

void GamemodeCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    if (args.empty()) {
        sender.sendMessage("Usage: /gamemode <survival|creative|adventure|spectator> [player]");
        return;
    }
    int gamemode = parseGamemode(args[0]);
    if (gamemode < 0) {
        sender.sendMessage("Invalid gamemode: " + args[0] + ". Use survival, creative, adventure, or spectator.");
        return;
    }
    // No player arg -- target self (only meaningful for a connected player
    // sender; console must specify a target, same as every other player-name-arg
    // command in this project).
    string targetName = (args.size() >= 2) ? args[1] : sender.getName();

    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        Player& player = conn->getPlayer();
        if (player.getUsername() != targetName) continue;

        player.setGamemode(gamemode);
        int threshold = conn->getCompressionThreshold();
        conn->addPacket(std::make_shared<Game_Event_p>(threshold, 3, static_cast<float>(gamemode))); // event 3: Change game mode
        conn->addPacket(std::make_shared<Player_Abilities_p>(threshold, abilitiesFlagsForGamemode(gamemode)));

        // Player_Info_Update_p is otherwise only ever sent once, at join --
        // refresh everyone's tab-list icon for this player now, same category
        // of gap the /op-/deop Commands_p resend fix closed for autocomplete.
        Player_Info_Update_p::Entry entry;
        entry.uuid = player.getUUID();
        entry.name = player.getUsername();
        entry.properties = player.getProfileProperties();
        entry.gamemode = gamemode;
        for (auto& other : ConnectionManager::getInstance().getActiveConnections()) {
            if (!other || other->getState() != ConnectionState::Play) continue;
            other->addPacket(std::make_shared<Player_Info_Update_p>(other->getCompressionThreshold(), std::vector<Player_Info_Update_p::Entry>{entry}));
        }

        sender.sendMessage("Set " + targetName + "'s gamemode to " + gamemodeName(gamemode));
        return;
    }
    sender.sendMessage("No such player online: " + targetName);
}

string GamemodeCommand::getName() const {
    return "gamemode";
}

string GamemodeCommand::getDescription() const {
    return "Changes a player's gamemode (survival, creative, adventure, spectator)";
}
