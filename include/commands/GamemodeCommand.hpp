#pragma once
#include <Command.hpp>

class GamemodeCommand : public Command {
    public:
        void execute(CommandSender& sender, const std::vector<string>& args) override;
        string getName() const override;
        string getDescription() const override;
        int getRequiredPermission() const override { return 2; }
        std::vector<string> getArgumentSuggestions() const override {
            return {"survival", "creative", "adventure", "spectator"};
        }
};
