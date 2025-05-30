#pragma once
#include <Standards.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>

class Console {
    public:
        static Console& getConsole();
        int Entry(string text);
        int Error(string text);
        int Post();
    private:

        enum Stream {
            OUT,
            IN,
            ERR
        };
        struct Message {
            string text;
            Stream stream;
        };
        struct LogBuffer {
            std::vector<Message> enteries;
            std::mutex mutex;
        };
        enum Colours {
            none=0,
            black=30,
            red=31,
            green=32,
            brown=33,
            blue=34,
            magenta=35,
            cyan=36,
            lightgray=37
        };
        Colours IN_COLOUR = none;
        Colours OUT_COLOUR = green;
        Colours ERR_COLOUR = red;
        Colours myColour = none;
        std::array<LogBuffer, 4> buffers;
        std::thread workerThread;
        std::atomic<bool> running{true};
        std::ofstream logFile;

        Console();
        ~Console();
        void createLog();
        void processBuffers(); // THREAD
        void startThread();
        int addToBuff(Message msg);
        void setColour(int colour, Stream stream);
};