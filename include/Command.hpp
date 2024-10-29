#pragma once
#include <Standards.hpp>

class Command {
    public:
        Command();
        ~Command();
    private:
        string name;
        string desc;
};