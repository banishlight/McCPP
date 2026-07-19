#include <Console.hpp>
#include <Standards.hpp>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <csignal>

#ifdef LINUX
#include <unistd.h>
#include <sys/ioctl.h>
#endif

// Needs to be OS specific

static std::atomic<bool> g_resizePending{false};

Console::Console() {
    createLog();
    updateTerminalSize();
    setupScrollRegion();
    enableRawMode();
    #ifdef LINUX
        std::signal(SIGWINCH, Console::onResizeSignal);
    #endif
    startThread();
    startInputThread();
}

Console::~Console() {
    running = false;
    if (workerThread.joinable()) {
        workerThread.join();
    }
    // inputThread is detached (it blocks in a raw read() with no clean way to
    // interrupt it), so just restore the terminal here; the OS reclaims it on exit.
    disableRawMode();
    resetScrollRegion();
    logFile.close();
}

void Console::createLog() {
    const std::string LOG_DIR = "Logs";
    std::filesystem::create_directories(LOG_DIR);

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
    std::filesystem::path filename = std::filesystem::path(LOG_DIR) / filenameStream.str();
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

void Console::startInputThread() {
    std::thread(&Console::processInput, this).detach();
}

void Console::onResizeSignal(int) {
    g_resizePending.store(true);
}

void Console::updateTerminalSize() {
    #ifdef LINUX
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 2 && w.ws_col > 0) {
            termRows = w.ws_row;
            termCols = w.ws_col;
        }
    #endif
}

void Console::setupScrollRegion() {
    updateTerminalSize();
    std::lock_guard<std::mutex> lock(screenMutex);
    // Scrollable log region is every row except the last, which stays
    // reserved for the static input line.
    std::cout << "\033[1;" << (termRows - 1) << "r";
    redrawInputLine();
}

void Console::resetScrollRegion() {
    std::cout << "\033[r"; // full-screen scrolling again
    std::cout << "\033[" << termRows << ";1H\033[K\n";
    std::cout.flush();
}

void Console::enableRawMode() {
    #ifdef LINUX
        tcgetattr(STDIN_FILENO, &_originalTermios);
        struct termios raw = _originalTermios;
        // Disable line buffering/echo/signal-generating keys so we can render
        // the input line ourselves and treat Ctrl+C as an ordinary keystroke.
        raw.c_lflag &= ~(ICANON | ECHO | ISIG);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    #endif
}

void Console::disableRawMode() {
    #ifdef LINUX
        tcsetattr(STDIN_FILENO, TCSANOW, &_originalTermios);
    #endif
}

void Console::redrawInputLine() {
    // Caller must hold screenMutex.
    std::cout << "\033[" << termRows << ";1H" << "\033[K" << prompt << inputBuffer;
    std::cout.flush();
}

void Console::processBuffers() {
    while(running) {
        if (g_resizePending.exchange(false)) {
            setupScrollRegion();
        }

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
                std::lock_guard<std::mutex> screenLock(screenMutex);
                for (const auto& msg : toProcess) {
                    // Park the cursor on the bottom row of the scroll region so the
                    // trailing newline scrolls the log instead of touching the input row.
                    std::cout << "\033[" << (termRows - 1) << ";1H";
                    std::cout.flush();

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
                redrawInputLine();
            }
        }

        if (!processedAny) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

void Console::processInput() {
    #ifdef LINUX
        char c;
        while (running) {
            ssize_t n = read(STDIN_FILENO, &c, 1);
            if (n <= 0) {
                continue;
            }

            std::lock_guard<std::mutex> screenLock(screenMutex);
            if (c == '\r' || c == '\n') {
                string command = inputBuffer;
                inputBuffer.clear();
                redrawInputLine();
                if (!command.empty()) {
                    std::lock_guard<std::mutex> cmdLock(commandMutex);
                    commandQueue.push(std::move(command));
                }
            } else if (c == 3) { // Ctrl+C: ISIG is off, so treat it like typing "stop"
                inputBuffer.clear();
                redrawInputLine();
                std::lock_guard<std::mutex> cmdLock(commandMutex);
                commandQueue.push("stop");
            } else if (c == 127 || c == 8) { // Backspace/DEL
                if (!inputBuffer.empty()) {
                    inputBuffer.pop_back();
                    redrawInputLine();
                }
            } else if (c == 27) { // ESC sequence (arrow keys, etc.) - swallow, no editing yet
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) > 0 && seq[0] == '[') {
                    read(STDIN_FILENO, &seq[1], 1);
                }
            } else if (std::isprint(static_cast<unsigned char>(c)) &&
                       static_cast<int>(prompt.size() + inputBuffer.size()) < termCols - 1) {
                inputBuffer.push_back(c);
                redrawInputLine();
            }
        }
    #endif
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

bool Console::Post(string& command) {
    std::lock_guard<std::mutex> lock(commandMutex);
    if (commandQueue.empty()) {
        return false;
    }
    command = std::move(commandQueue.front());
    commandQueue.pop();
    return true;
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