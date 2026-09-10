#pragma once

#include <string>
#include <vector>
#include "BankSystem.h"

// ============================================================================
// RESILIENT FILE PERSISTENCE & DISASTER RECOVERY ENGINE
// Demonstrating C++ File Handling, ACID Atomic Writes, Exception-Safe Parsing,
// Checksum Validation, and Corrupted Record Recovery
// ============================================================================

struct PersistenceResult {
    bool success;
    size_t recordsLoaded;
    size_t recordsSkipped;
    size_t recordsSaved;
    std::vector<std::string> warnings;
    std::string errorMessage;

    PersistenceResult() 
        : success(true), recordsLoaded(0), recordsSkipped(0), recordsSaved(0) {}

    void addWarning(const std::string& warn) {
        warnings.push_back(warn);
        recordsSkipped++;
    }
};

class FilePersistence {
public:
    // 1. Atomic Persistence (Write-to-Temp -> Backup -> Atomic Replace)
    static bool saveBankState(const BankSystem& bank, const std::string& directory = "data");
    static bool saveBankStateAtomic(const BankSystem& bank, const std::string& directory, PersistenceResult& outResult);

    // 2. Resilient Ingestion (Gracefully handles corrupted rows, NaN/strings, bad delimiters)
    static bool loadBankState(BankSystem& bank, const std::string& directory = "data");
    static bool loadBankStateResilient(BankSystem& bank, const std::string& directory, PersistenceResult& outResult);

    // 3. Disaster Recovery (Restores from .bak if primary .txt is missing or heavily corrupted)
    static bool restoreFromBackup(BankSystem& bank, const std::string& directory = "data", PersistenceResult* outResult = nullptr);

    // 4. Storage Diagnostic & Referential Integrity Scanner
    static bool verifyStorageIntegrity(const std::string& directory, std::vector<std::string>& outIssues);

    // 5. Exception-Safe Parsers (No unhandled std::invalid_argument or std::out_of_range exceptions)
    static bool safeParseDouble(const std::string& str, double& outVal);
    static bool safeParseInt(const std::string& str, int& outVal);
};
