#include "BankingApp.h"
#include <iostream>
#include <string>
#include <cstdio>

// ============================================================================
// SMART BANKING MANAGEMENT SYSTEM - MAIN ENTRY POINT
// Demonstrating complete college-level C++ Architecture:
// OOP, STL, DSA (vector, unordered_map, map, queue, stack, set, graph),
// Exception Handling, File Persistence, and Smart Educational Analytics
// ============================================================================

int main(int argc, char* argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);
    try {
        BankingApp app;

        // If invoked with --test or in non-interactive verification
        if (argc > 1 && std::string(argv[1]) == "--test") {
            app.runAutomatedSystemVerification();
            return 0;
        }

        // If invoked with --server [port]
        if (argc > 1 && std::string(argv[1]) == "--server") {
            int port = (argc > 2) ? std::stoi(argv[2]) : 8080;
            app.runRestServer(port);
            return 0;
        }

        // Default: Launch Interactive Console Banking Suite
        app.run();

    } catch (const std::exception& ex) {
        std::cerr << "Fatal System Exception: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
