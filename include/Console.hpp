#pragma once
#include <Standards.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <array>
#include <queue>

#ifdef LINUX
#include <termios.h>
#endif

class Console {
    public:
        static Console& getConsole();
        int Entry(string text);
        int Error(string text);
        bool Post(string& command); // pops next submitted console command, if any
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

        // Bottom row of the terminal is reserved as a static input line; the
        // scroll region above it (rows 1..termRows-1) is where log output lands.
        const string prompt = "> ";
        string inputBuffer;
        int termRows = 24;
        int termCols = 80;
        std::mutex screenMutex;
        std::queue<string> commandQueue;
        std::mutex commandMutex;
        #ifdef LINUX
            struct termios _originalTermios;
        #endif

        Console();
        ~Console();
        void createLog();
        void processBuffers(); // THREAD
        void processInput();   // THREAD
        void startThread();
        void startInputThread();
        int addToBuff(Message msg);
        void setColour(int colour, Stream stream);

        void enableRawMode();
        void disableRawMode();
        void updateTerminalSize();
        void setupScrollRegion();
        void resetScrollRegion();
        void redrawInputLine(); // caller must hold screenMutex
        static void onResizeSignal(int);
};