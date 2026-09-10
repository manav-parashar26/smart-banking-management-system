#pragma once

#include <string>
#include <vector>

// ============================================================================
// DATA TRANSFER OBJECTS (DTOs)
// Decouples domain entities from presentation (CLI/REST API/Web Frontend)
// ============================================================================

struct CustomerDTO {
    std::string customerId;
    std::string fullName;
    std::string email;
    std::string phone;
    std::vector<std::string> linkedAccounts;
};

struct AccountDTO {
    std::string accountNumber;
    std::string accountType;
    std::string holderName;
    double balance;
    bool isActive;
    double interestRate;     // Specific to Savings
    double minimumBalance;   // Specific to Savings
    double overdraftLimit;   // Specific to Current
    double availableCredit;  // Balance + Overdraft
};

struct TransactionDTO {
    std::string transactionId;
    std::string fromAccount;
    std::string toAccount;
    double amount;
    std::string timestamp;
    std::string type;
    std::string status;
    std::string description;
};

struct TransferRequestDTO {
    std::string fromAccount;
    std::string toAccount;
    double amount;
    std::string description;
};

struct DepositWithdrawDTO {
    std::string accountNumber;
    double amount;
    std::string description;
};

struct AuthRequestDTO {
    std::string customerId;
    std::string pin;
};

struct AuthResponseDTO {
    bool success;
    std::string customerId;
    std::string fullName;
    std::string token;
    std::string message;
};
