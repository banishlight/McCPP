#include <Standards.hpp>
#include <iostream>
#include <fstream>

class Console {
    public:
        static Console& getConsole();
        int Entry(string text);
        int Post();
    private:
        Console();
        ~Console();
        bool opened = false;
        std::ofstream myLog;
};