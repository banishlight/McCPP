#include <commands/HelpCommand.hpp>
#include <CommandRegistry.hpp>
#include <CommandSender.hpp>

void HelpCommand::execute(CommandSender& sender, const std::vector<string>& args) {
    CommandRegistry& registry = CommandRegistry::getInstance();
    for (const string& name : registry.getCommandNames()) {
        std::shared_ptr<Command> command = registry.getCommand(name);
        if (!command) continue;
        if (sender.getPermissionLevel() < command->getRequiredPermission()) continue;
        sender.sendMessage("/" + command->getName() + " - " + command->getDescription());
    }
}

string HelpCommand::getName() const {
    return "help";
}

string HelpCommand::getDescription() const {
    return "Lists available commands";
}
