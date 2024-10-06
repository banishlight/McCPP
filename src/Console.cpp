#include <Console.hpp>
#include <Standards.hpp>
#include <iostream>
#include <fstream>

Console::Console() {
    this->myLog.open("ExampleLog.txt");
}

Console::~Console() {
    this->myLog.close();
}

Console& Console::GetConsole() {
    static Console singleton;
    return singleton;
}

int Console::Entry(string text) {
    // Write text to file and terminal
    std::cout << text << "\n";
    myLog << text;
    return 0;
}

int Console::Post() {
    // Read text from user
    // Write to file and terminal
    string text;
    std::cin >> text;
    myLog << text;
    return 0;
}

