#pragma once
#include <CommandSender.hpp>
#include <memory>

class Connection;

// Wraps a connected player issuing a command via chat (a "/..." message).
// Permission comes from OpsList at lookup time rather than anything cached on
// Player, since op status can change between commands (e.g. hand-edited
// ops.json, or a /deop mid-session).
class PlayerCommandSender : public CommandSender {
    public:
        explicit PlayerCommandSender(std::shared_ptr<Connection> connection);
        int getPermissionLevel() const override;
        string getName() const override;
        void sendMessage(const string& message) override;
    private:
        std::shared_ptr<Connection> _connection;
};
