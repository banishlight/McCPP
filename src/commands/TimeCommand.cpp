#include <commands/TimeCommand.hpp>
#include <CommandSender.hpp>
#include <World.hpp>
#include <network/ConnectionManager.hpp>
#include <network/Connection.hpp>
#include <network/packets/Play.hpp>
#include <memory>

namespace {
    // -1 = invalid. Named presets confirmed against minecraft.wiki's /time
    // page, not assumed.
    Int64 parseTimeValue(const string& text) {
        if (text == "day") return 1000;
        if (text == "noon") return 6000;
        if (text == "night") return 13000;
        if (text == "midnight") return 18000;
        try {
            size_t consumed = 0;
            Int64 value = std::stoll(text, &consumed);
            if (consumed != text.size() || value < 0) return -1;
            return value;
        } catch (...) {
            return -1;
        }
    }
}

void TimeCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    if (args.size() < 2 || args[0] != "set") {
        sender.sendMessage("Usage: /time set <day|noon|night|midnight|<ticks>>");
        return;
    }
    Int64 value = parseTimeValue(args[1]);
    if (value < 0) {
        sender.sendMessage("Invalid time value: " + args[1]);
        return;
    }

    World& world = World::getInstance();
    world.setDayTime(value);

    // Broadcast immediately rather than waiting for DayNightSystem's next
    // periodic tick -- same loop shape it already uses.
    for (auto& conn : ConnectionManager::getInstance().getActiveConnections()) {
        if (!conn || conn->getState() != ConnectionState::Play) continue;
        conn->addPacket(std::make_shared<Update_Time_p>(conn->getCompressionThreshold(), value));
    }

    sender.sendMessage("Set the time to " + std::to_string(value));
}

string TimeCommand::getName() const {
    return "time";
}

string TimeCommand::getDescription() const {
    return "Sets the world time (/time set <day|noon|night|midnight|<ticks>>)";
}
