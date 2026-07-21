#include <CommandRegistry.hpp>
#include <commands/StopCommand.hpp>
#include <commands/HelpCommand.hpp>
#include <commands/SayCommand.hpp>
#include <commands/OpCommand.hpp>
#include <commands/DeopCommand.hpp>
#include <commands/ListCommand.hpp>
#include <commands/KickCommand.hpp>
#include <commands/SaveAllCommand.hpp>
#include <Console.hpp>

CommandRegistry& CommandRegistry::getInstance() {
    static CommandRegistry instance;
    return instance;
}

void CommandRegistry::initialize() {
    if (_initialized) return;
    registerCommand(std::make_shared<StopCommand>());
    registerCommand(std::make_shared<HelpCommand>());
    registerCommand(std::make_shared<SayCommand>());
    registerCommand(std::make_shared<OpCommand>());
    registerCommand(std::make_shared<DeopCommand>());
    registerCommand(std::make_shared<ListCommand>());
    registerCommand(std::make_shared<KickCommand>());
    registerCommand(std::make_shared<SaveAllCommand>());
    _initialized = true;
}

void CommandRegistry::registerCommand(std::shared_ptr<Command> command) {
    const string& name = command->getName();
    if (_commands.find(name) != _commands.end()) {
        Console::getConsole().Error("CommandRegistry::registerCommand(): A command named '" + name + "' is already registered; ignoring.");
        return;
    }
    _commandNames.push_back(name);
    _commands[name] = std::move(command);
}

bool CommandRegistry::dispatch(CommandSender& sender, const string& name, const std::vector<string>& args) {
    auto it = _commands.find(name);
    if (it == _commands.end()) {
        sender.sendMessage("Unknown command: " + name);
        return false;
    }
    if (sender.getPermissionLevel() < it->second->getRequiredPermission()) {
        sender.sendMessage("You do not have permission to use this command.");
        return false;
    }
    it->second->execute(sender, args);
    return true;
}

const std::vector<string>& CommandRegistry::getCommandNames() const {
    return _commandNames;
}

std::shared_ptr<Command> CommandRegistry::getCommand(const string& name) const {
    auto it = _commands.find(name);
    if (it == _commands.end()) {
        return nullptr;
    }
    return it->second;
}