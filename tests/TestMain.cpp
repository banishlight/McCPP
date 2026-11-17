#include <tests/TestFramework.hpp>
#include <exception>
#include <iostream>
#include <string>

// Runs every registered test. An optional argument filters to tests whose
// name contains that substring, e.g. `./TestSuite VarInt`.
// Exit code is 0 only if every selected test passed.
int main(int argc, char** argv) {
    std::string filter = argc > 1 ? argv[1] : "";

    int ran = 0, failed = 0;
    for (const TestFramework::TestCase& test : TestFramework::registry()) {
        if (!filter.empty() && std::string(test.name).find(filter) == std::string::npos) {
            continue;
        }
        ran++;
        TestFramework::currentFailures() = 0;
        std::cerr << "[ RUN  ] " << test.name << "\n";
        try {
            test.body();
        } catch (const std::exception& e) {
            TestFramework::reportFailure(test.file, 0, std::string("uncaught exception: ") + e.what());
        } catch (...) {
            TestFramework::reportFailure(test.file, 0, "uncaught non-standard exception");
        }
        if (TestFramework::currentFailures() > 0) {
            failed++;
            std::cerr << "[ FAIL ] " << test.name << "\n";
        } else {
            std::cerr << "[  OK  ] " << test.name << "\n";
        }
    }

    std::cerr << "\n" << ran - failed << "/" << ran << " tests passed";
    if (failed > 0) std::cerr << ", " << failed << " failed";
    std::cerr << "\n";
    return failed == 0 ? 0 : 1;
}
