#include <Console.hpp>
#include <Standards.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip> 

Console::Console() {
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);
    // Format the time into a string: "Log_YYYYMMDD_HHMMSS.txt"
    std::ostringstream filenameStream;
    filenameStream << "Log_"
                   << (now->tm_year + 1900)  // Year
                   << std::setfill('0') << std::setw(2) << (now->tm_mon + 1)  // Month
                   << std::setw(2) << now->tm_mday  // Day
                   << "_"
                   << std::setw(2) << now->tm_hour  // Hour
                   << std::setw(2) << now->tm_min   // Minute
                   << std::setw(2) << now->tm_sec   // Second
                   << ".txt";
    std::string filename = filenameStream.str();
    this->myLog.open(filename);
    if (!this->myLog) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
    initList();
}

Console::~Console() {
    this->myLog.close();
}

Console& Console::getConsole() {
    static Console singleton;
    return singleton;
}

int Console::Entry(string text) {
    text = text + "\n";
    setColour(32, OUT);
    std::cout << text;
    resetColour(OUT);
    myLog << text;
    return 0;
}

int Console::Post() {
    string text;
    setColour(36, IN);
    std::cin >> text;
    resetColour(IN);
    myLog << text;
    myLog << "\n";
    return 0;
}

int Console::Error(string text) {
    text = "Error: " + text + "\n";
    setColour(31, ERR);
    std::cerr << text;
    resetColour(ERR);
    myLog << text;
    return 0;
}

int Console::Command() {
    string command;
    std::cin >> command;
    myLog << command;
    myLog << "\n";
    return CommandDecode(command);
}

int Console::CommandDecode(string command) {
    if (command.compare("help") == 0) {
        Entry("List of available commands: ");
        // Entry(commandList);
    }
    else if (command.compare("stop") == 0) {
        Entry("Exiting server.");
        return 1;
    }
    else {
        Entry("Unkown command");
    }
    return 0;
}

void Console::resetColour(Stream stream) { 
    switch (stream) {
        case OUT:
            std::cout << "\033[0m";
        break;
        case ERR:
            std::cerr << "\033[0m";
        break;
        case IN:
            // a hack since cin can't be given escape characters
            std::cout << "\033[0m";
        break;
    }
}

void Console::setColour(int colour, Stream stream) {
    switch (stream) {
        case OUT:
            std::cout << "\033[" << colour << "m";
        break;
        case ERR:
            std::cerr << "\033[" << colour << "m";
        break;
        case IN:
            // a hack since cin can't be given escape characters
            std::cout << "\033[" << colour << "m";
        break;
    } 
}

int Console::initList() {
    return 0;
}
