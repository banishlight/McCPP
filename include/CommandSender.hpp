#pragma once
#include <Standards.hpp>

// Who is executing a command -- lets Command::execute check identity/permission
// without every command reaching into Connection/Player details directly.
// Two implementations: ConsoleCommandSender (always max permission) and
// PlayerCommandSender (permission from OpsList), one per command dispatch.
class CommandSender {
    public:
        virtual ~CommandSender() = default;
        virtual int getPermissionLevel() const = 0;
        virtual string getName() const = 0;
        virtual void sendMessage(const string& message) = 0;
};
