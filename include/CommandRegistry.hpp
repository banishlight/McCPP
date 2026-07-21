#pragma once
#include <Standards.hpp>
#include <Command.hpp>
#include <CommandSender.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

// Requires initialization
class CommandRegistry {
    public:
        static CommandRegistry& getInstance();
        void initialize();
        // Lets built-in code and (later) plugins add commands without touching this class.
        void registerCommand(std::shared_ptr<Command> command);
        // Returns false if no command with that name is registered, or if
        // sender doesn't have the required permission -- either case sends
        // sender feedback itself, so callers don't need to.
        bool dispatch(CommandSender& sender, const string& name, const std::vector<string>& args);
        const std::vector<string>& getCommandNames() const;
        std::shared_ptr<Command> getCommand(const string& name) const;
    private:
        CommandRegistry() = default;
        ~CommandRegistry() = default;
        std::unordered_map<string, std::shared_ptr<Command>> _commands;
        std::vector<string> _commandNames;
        bool _initialized = false;
};