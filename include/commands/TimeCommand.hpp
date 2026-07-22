#pragma once
#include <Command.hpp>

// Only the "set" subcommand is implemented -- no /time add or /time query,
// matching what this feature was actually asked to help with (testing).
class TimeCommand : public Command {
    public:
        void execute(CommandSender& sender, const std::vector<string>& args) override;
        string getName() const override;
        string getDescription() const override;
        int getRequiredPermission() const override { return 2; }
};
