#pragma once
#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Minimal self-registering test framework, kept dependency-free to match the
// project's near-zero external library goal.
//
// Usage:
//     TEST(VarIntRoundTrip) {
//         CHECK_EQ(varIntDeserialize(varIntSerialize(300)), 300);
//     }
//
// CHECK* macros record a failure and keep going, so one run reports every
// broken assertion in a test. REQUIRE* macros record a failure and return
// from the test immediately -- use them when later checks would be
// meaningless (or would crash) if this one fails.

namespace TestFramework {

    struct TestCase {
        const char* name;
        const char* file;
        std::function<void()> body;
    };

    inline std::vector<TestCase>& registry() {
        static std::vector<TestCase> tests;
        return tests;
    }

    // Failures recorded by the currently running test.
    inline int& currentFailures() {
        static int failures = 0;
        return failures;
    }

    struct Registrar {
        Registrar(const char* name, const char* file, std::function<void()> body) {
            registry().push_back({name, file, std::move(body)});
        }
    };

    inline void reportFailure(const char* file, int line, const std::string& message) {
        currentFailures()++;
        std::cerr << "    " << file << ":" << line << ": " << message << "\n";
    }

    template <typename T>
    std::string describe(const T& value) {
        std::ostringstream out;
        out << value;
        return out.str();
    }

    inline std::string describe(unsigned char value) {
        std::ostringstream out;
        out << "0x" << std::hex << static_cast<int>(value);
        return out.str();
    }

    template <typename T>
    std::string describe(const std::vector<T>& values) {
        std::ostringstream out;
        out << "{";
        for (size_t i = 0; i < values.size(); i++) {
            if (i) out << ", ";
            out << describe(values[i]);
        }
        out << "}";
        return out.str();
    }
}

#define TEST_CONCAT_INNER(a, b) a##b
#define TEST_CONCAT(a, b) TEST_CONCAT_INNER(a, b)

#define TEST(name)                                                                  \
    static void TEST_CONCAT(test_body_, name)();                                    \
    static TestFramework::Registrar TEST_CONCAT(test_registrar_, name)(             \
        #name, __FILE__, TEST_CONCAT(test_body_, name));                            \
    static void TEST_CONCAT(test_body_, name)()

#define TEST_CHECK_IMPL(cond, message, onFail)                                      \
    do {                                                                            \
        if (!(cond)) {                                                              \
            TestFramework::reportFailure(__FILE__, __LINE__, (message));            \
            onFail;                                                                 \
        }                                                                           \
    } while (0)

#define TEST_EQ_IMPL(actual, expected, onFail)                                      \
    do {                                                                            \
        const auto& test_actual_ = (actual);                                        \
        const auto& test_expected_ = (expected);                                    \
        if (!(test_actual_ == test_expected_)) {                                    \
            TestFramework::reportFailure(__FILE__, __LINE__,                        \
                std::string(#actual " == " #expected "\n      actual:   ")          \
                + TestFramework::describe(test_actual_)                             \
                + "\n      expected: " + TestFramework::describe(test_expected_));  \
            onFail;                                                                 \
        }                                                                           \
    } while (0)

#define TEST_NEAR_IMPL(actual, expected, tolerance, onFail)                         \
    do {                                                                            \
        const double test_actual_ = (actual);                                       \
        const double test_expected_ = (expected);                                   \
        if (!(std::fabs(test_actual_ - test_expected_) <= (tolerance))) {           \
            std::ostringstream test_msg_;                                           \
            test_msg_.precision(17);                                                \
            test_msg_ << #actual " ~= " #expected " (tolerance " << (tolerance)     \
                      << ")\n      actual:   " << test_actual_                      \
                      << "\n      expected: " << test_expected_;                    \
            TestFramework::reportFailure(__FILE__, __LINE__, test_msg_.str());      \
            onFail;                                                                 \
        }                                                                           \
    } while (0)

#define CHECK(cond)                        TEST_CHECK_IMPL(cond, "CHECK(" #cond ")", (void)0)
#define CHECK_EQ(actual, expected)         TEST_EQ_IMPL(actual, expected, (void)0)
#define CHECK_NEAR(actual, expected, tol)  TEST_NEAR_IMPL(actual, expected, tol, (void)0)

#define REQUIRE(cond)                      TEST_CHECK_IMPL(cond, "REQUIRE(" #cond ")", return)
#define REQUIRE_EQ(actual, expected)       TEST_EQ_IMPL(actual, expected, return)
#define REQUIRE_NEAR(actual, expected, tol) TEST_NEAR_IMPL(actual, expected, tol, return)
