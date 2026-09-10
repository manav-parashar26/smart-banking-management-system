#pragma once

#include "BankSystem.h"
#include "TransactionAnalytics.h"
#include "TransactionGraph.h"
#include "SmartAuditor.h"
#include "FilePersistence.h"
#include "DSABenchmark.h"
#include "HttpServer.h"
#include "DatabaseManager.h"

// ============================================================================
// BANKING APPLICATION CONTROLLER (INTERACTIVE CLI & ADMIN DASHBOARD)
// Demonstrating complete college-level software architecture and design patterns
// ============================================================================

class BankingApp {
private:
    BankSystem m_bank;
    std::string m_adminPassword;

    // Sub-menus
    void customerPortal();
    void customerDashboard(Customer& customer);
    void adminPortal();
    void adminDashboard();

    // Helper operations
    void seedInitialDemoData();

public:
    explicit BankingApp(const std::string& adminPass = "admin123");

    void run();
    void runAutomatedSystemVerification();
    void runRestServer(int port = 8080);
};
