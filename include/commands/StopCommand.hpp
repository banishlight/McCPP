#pragma once
#include <Command.hpp>

class StopCommand : public Command {
    public:
        void execute(const std::vector<string>& args) override;
        string getName() const override;
        string getDescription() const override;
};