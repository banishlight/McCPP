#pragma once
#include <Standards.hpp>
#include <vector>

class CommandSender;

// Base class for commands, issued either from the server console or (via
// chat's Chat Command packet) a connected player. Subclass this and register
// an instance with CommandRegistry to add a new command; the same interface
// is meant to serve plugin-provided commands later, not just built-in ones.
class Command {
    public:
        virtual ~Command() = default;
        virtual void execute(CommandSender& sender, const std::vector<string>& args) = 0;
        virtual string getName() const = 0;
        virtual string getDescription() const = 0;
        // Op level (0-4, matches Properties::op_permission_level) required to
        // run this command. 0 means anyone, including un-opped players.
        virtual int getRequiredPermission() const { return 0; }
};
