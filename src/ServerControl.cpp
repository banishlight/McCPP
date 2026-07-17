#include <ServerControl.hpp>
#include <atomic>

namespace {
    std::atomic<bool> shutdownRequested{false};
}

namespace ServerControl {
    void requestShutdown() {
        shutdownRequested = true;
    }

    bool isShutdownRequested() {
        return shutdownRequested;
    }
}