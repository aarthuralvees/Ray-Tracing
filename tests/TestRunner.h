#pragma once
#include <iostream>

namespace tests {
    inline int _passed = 0, _failed = 0;
    inline const char* _current = "";
}

#define CHECK(expr) do { \
    if (!(expr)) { \
        std::cerr << "  FAIL [" << tests::_current << "]: " #expr \
                  << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        tests::_failed++; \
    } else { tests::_passed++; } \
} while(0)

inline void run_test(const char* name, void(*fn)()) {
    tests::_current = name;
    fn();
    tests::_current = "";
}

// Write to stderr so test output does not mix with the PPM image on stdout.
inline void report_tests() {
    int total = tests::_passed + tests::_failed;
    std::cerr << "\n--- " << total << " checks: "
              << tests::_passed << " passed, "
              << tests::_failed << " failed ---\n";
}
