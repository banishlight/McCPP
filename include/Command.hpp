#pragma once
#include <Standards.hpp>
#include <vector>

// Base class for console commands. Subclass this and register an instance
// with CommandRegistry to add a new command; the same interface is meant to
// serve plugin-provided commands later, not just built-in ones.
class Command {
    public:
        virtual ~Command() = default;
        virtual void execute(const std::vector<string>& args) = 0;
        virtual string getName() const = 0;
        virtual string getDescription() const = 0;
};