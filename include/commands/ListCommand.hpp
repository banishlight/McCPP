#pragma once
#include <Command.hpp>

class ListCommand : public Command {
    public:
        void execute(CommandSender& sender, const std::vector<string>& args) override;
        string getName() const override;
        string getDescription() const override;
};
