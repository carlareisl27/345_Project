#ifndef TESTHELPER_H
#define TESTHELPER_H

#include <iostream>

// Minimal test helper shared by MapTests, TowerTests and CritterTests (no testing library needed).
inline int passedTests = 0;
inline int failedTests = 0;

inline void check(bool condition, const char* description) {
    if (condition) {
        passedTests++;
        std::cout << "  [PASS] " << description << '\n';
    } else {
        failedTests++;
        std::cout << "  [FAIL] " << description << '\n';
    }
}

// prints the totals and returns the exit code of main: 0 when every check passed.
inline int testSummary() {
    std::cout << "\nPassed: " << passedTests << "   Failed: " << failedTests << '\n';
    return failedTests == 0 ? 0 : 1;
}

#endif
