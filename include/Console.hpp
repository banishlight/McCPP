#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <Command.hpp>
#include <Standards.hpp>


class Console {
    enum Stream {
            OUT,
            IN,
            ERR
        };
    public:
        static Console& getConsole();
        int Entry(string text);
        string Post();
        int Error(string text);
        int Command();
    private:
        Console();
        ~Console();
        int CommandDecode(string command);
        void setColour(int colour, Stream stream);
        int initList();
        bool opened = false;
        std::ofstream myLog;
        std::unordered_map<string, string> commandList;
        enum Colours {
            black=30,
            red=31,
            green=32,
            brown=33,
            blue=34,
            magenta=35,
            cyan=36,
            lightgray=37
        };
        const int IN_COLOUR = cyan;
        const int OUT_COLOUR = green;
        const int ERR_COLOUR = red;
};