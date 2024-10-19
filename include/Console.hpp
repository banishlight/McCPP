#pragma once
#include <Standards.hpp>
#include <iostream>
#include <fstream>

class Console {
    public:
        static Console& getConsole();
        int Entry(string text);
        int Post();
        int Error(string text);
        int Command();
    private:
        Console();
        ~Console();
        int CommandDecode(string command);

        bool opened = false;
        std::ofstream myLog;
};