#include <CommandRegistry.hpp>
#include <commands/StopCommand.hpp>
#include <Console.hpp>

CommandRegistry& CommandRegistry::getInstance() {
    static CommandRegistry instance;
    return instance;
}

void CommandRegistry::initialize() {
    if (_initialized) return;
    registerCommand(std::make_shared<StopCommand>());
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

bool CommandRegistry::dispatch(const string& name, const std::vector<string>& args) {
    auto it = _commands.find(name);
    if (it == _commands.end()) {
        return false;
    }
    it->second->execute(args);
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