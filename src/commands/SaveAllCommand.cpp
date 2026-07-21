#include <commands/SaveAllCommand.hpp>
#include <CommandSender.hpp>
#include <systems/AutosaveSystem.hpp>

void SaveAllCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    AutosaveSystem::saveNow();
    sender.sendMessage("Saved the world.");
}

string SaveAllCommand::getName() const {
    return "save-all";
}

string SaveAllCommand::getDescription() const {
    return "Saves every changed chunk and level.dat immediately";
}
