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
    setColour(OUT_COLOUR, OUT);
    std::cout << text;
    setColour(0, OUT);
    myLog << text;
    return 0;
}

string Console::Post() {
    string text;
    setColour(IN_COLOUR, IN);
    std::cin >> text;
    setColour(0, IN);
    myLog << text;
    myLog << "\n";
    return text;
}

int Console::Error(string text) {
    text = "Error: " + text + "\n";
    setColour(ERR_COLOUR, ERR);
    std::cerr << text;
    setColour(0, ERR);
    myLog << text;
    return 0;
}

int Console::Command() {
    string command;
    setColour(IN_COLOUR, IN);
    std::cin >> command;
    setColour(0, IN);
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

void Console::setColour(int colour, Stream stream) {
    // 0 resets the colour to system default
    switch (stream) {
        case OUT:
            if(colour == 0) {
                std::cout << "\033[0m";
            }
            else { std::cout << "\033[" << colour << "m"; }
            
        break;
        case ERR:
            if(colour == 0) {
                std::cerr << "\033[0m";
            }
            else { std::cerr << "\033[" << colour << "m"; }
        break;
        case IN:
        // use cout, a hack since cin can't be given escape characters
            if(colour == 0) {
                std::cout << "\033[0m";
            }
            else { std::cout << "\033[" << colour << "m"; }
        break;
    } 
}

int Console::initList() {
    return 0;
}
