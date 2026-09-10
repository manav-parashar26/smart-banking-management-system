#pragma once

#include <string>
#include <vector>
#include "BankSystem.h"
#include "BankingDTOs.h"

// ============================================================================
// RELATIONAL DATABASE MANAGER & SQL ABSTRACTION LAYER (DAL)
// Demonstrating Relational Algebra (SELECT, FILTER, JOIN),
// Schema Generation, Referential Integrity Validation, and SQL Dump Serialization
// ============================================================================

struct CustomerAccountJoinRecord {
    std::string customerId;
    std::string customerName;
    std::string email;
    std::string accountNumber;
    std::string accountType;
    double balance;
    bool isActive;

    void display() const;
};

class DatabaseManager {
public:
    // 1. DDL & SQL Generation (Ready for MySQL Workbench / phpMyAdmin)
    static bool generateMySQLSchemaScript(const std::string& outputPath = "data/schema.sql");
    static bool exportToMySQLDump(const BankSystem& bank, const std::string& outputPath = "data/banking_system_dump.sql");
    static bool importFromMySQLDump(BankSystem& bank, const std::string& sqlDumpPath = "data/banking_system_dump.sql");

    // 2. Relational Query Engine (Simulating SQL INNER JOIN & Filter Predicates)
    static std::vector<CustomerAccountJoinRecord> executeCustomerAccountJoin(const BankSystem& bank);
    static std::vector<AccountDTO> queryAccountsByBalanceThreshold(const BankSystem& bank, double minBalance);

    // 3. Referential Integrity Check (Validating Foreign Key constraints: accounts.customer_id -> customers.customer_id)
    static bool verifyReferentialIntegrity(const BankSystem& bank, std::vector<std::string>& outViolations);
};
