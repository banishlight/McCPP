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
}

Console::~Console() {
    this->myLog.close();
}

Console& Console::getConsole() {
    static Console singleton;
    return singleton;
}

int Console::Entry(string text) {
    std::cout << text << "\n";
    myLog << text;
    myLog << "\n";
    return 0;
}

int Console::Post() {
    string text;
    std::cin >> text;
    myLog << text;
    myLog << "\n";
    return 0;
}

