#pragma once

// Lets commands (e.g. "stop") request server shutdown without reaching back
// into main()'s loop directly.
namespace ServerControl {
    void requestShutdown();
    bool isShutdownRequested();
}