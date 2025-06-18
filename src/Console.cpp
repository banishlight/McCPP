#include <Console.hpp>
#include <Standards.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <iomanip> 

Console::Console() {
    createLog();    
    startThread();
}

Console::~Console() {
    running = false;
    if (workerThread.joinable()) {
        workerThread.join();
    }
    logFile.close();
}

void Console::createLog() {
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
    this->logFile.open(filename);
    if (!this->logFile) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

Console& Console::getConsole() {
    static Console singleton;
    return singleton;
}

void Console::startThread() {
    workerThread = std::thread(&Console::processBuffers, this);
}

void Console::processBuffers() {
    while(running) {
        bool processedAny = false;
        // Process all buffers
        for (auto& buffer : buffers) {
            std::vector<Message> toProcess;
            
            {
                std::lock_guard<std::mutex> lock(buffer.mutex);
                if (!buffer.enteries.empty()) {
                    toProcess.swap(buffer.enteries);
                    processedAny = true;
                }
            }

            if (!toProcess.empty()) {
                // Non-Empty buffer
                for (const auto& msg : toProcess) {
                    // Actual output happens here
                    
                    if (msg.stream == OUT) {
                        setColour(OUT_COLOUR, msg.stream);
                        std::cout << msg.text;
                    } else if (msg.stream == ERR) {
                        setColour(ERR_COLOUR, msg.stream);
                        std::cerr << msg.text;
                    }
                    setColour(0, msg.stream);
                    logFile.write(msg.text.c_str(), msg.text.size());
                }
                logFile.flush();
            }
        }

        if (!processedAny) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

int Console::Entry(string text) {
    Message msg;
    msg.text = text + "\n";
    msg.stream = OUT;
    return addToBuff(msg);
}

int Console::Error(string text) {
    Message msg;
    msg.text = text + "\n";
    msg.stream = ERR;
    return addToBuff(msg);
}

int Console::Post() {
    // TODO: Something special for input
    return 0;
}

int Console::addToBuff(Message msg) {
    // Simple hash of thread ID to select a buffer, distributing threads across buffers
    size_t bufferIndex = std::hash<std::thread::id>{}(std::this_thread::get_id()) % buffers.size();
    
    // Add timestamp in future?
    // string timestampedMessage = addTimestamp(text);
    
    {
        std::lock_guard<std::mutex> lock(buffers[bufferIndex].mutex);
        buffers[bufferIndex].enteries.push_back(std::move(msg));
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
