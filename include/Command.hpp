#pragma once
#include <Standards.hpp>

class Command {
    public:
        Command();
        ~Command();
        int execute();
        string getName();
        string getDesc();
    private:
        string name;
        string desc;
};