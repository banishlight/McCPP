#pragma once
#include <CommandSender.hpp>

// The server console always has max permission and bypasses op-level checks
// entirely, matching vanilla.
class ConsoleCommandSender : public CommandSender {
    public:
        int getPermissionLevel() const override { return 4; }
        string getName() const override { return "Server"; }
        void sendMessage(const string& message) override;
};
