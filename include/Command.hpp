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
        // Optional fixed-choice suggestions for this command's first argument
        // (e.g. gamemode names), surfaced as chat autocomplete via Commands_p.
        // Deliberately literal-only, not a full Brigadier argument-type/parser-ID
        // system (see docs/general-documentation.md, "Command autocomplete", for
        // why that's out of scope). Empty by default -- a command that returns
        // suggestions here has its own node marked non-executable (a bare
        // "/name" alone isn't a complete command), and each suggestion becomes
        // an executable literal child instead.
        virtual std::vector<string> getArgumentSuggestions() const { return {}; }
};
