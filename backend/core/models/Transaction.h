#pragma once

#include <string>
#include <iostream>
#include "BankingDTOs.h"

// ============================================================================
// TRANSACTION DOMAIN MODEL & ENUMS
// Demonstrating: Enums, Structuring Data, Immutable Audit Records
// ============================================================================

enum class TransactionType {
    DEPOSIT,
    WITHDRAWAL,
    TRANSFER_IN,
    TRANSFER_OUT
};

enum class TransactionStatus {
    SUCCESS,
    FAILED
};

class Transaction {
private:
    std::string m_transactionId;
    std::string m_fromAccount;
    std::string m_toAccount;
    double m_amount;
    std::string m_timestamp;
    TransactionType m_type;
    TransactionStatus m_status;
    std::string m_description;

public:
    // Parameterized Constructor
    Transaction(const std::string& id,
                const std::string& fromAcc,
                const std::string& toAcc,
                double amount,
                TransactionType type,
                TransactionStatus status,
                const std::string& desc,
                const std::string& timestamp = "");

    // Getters
    std::string getTransactionId() const;
    std::string getFromAccount() const;
    std::string getToAccount() const;
    double getAmount() const;
    std::string getTimestamp() const;
    TransactionType getType() const;
    TransactionStatus getStatus() const;
    std::string getDescription() const;

    // Formatting & Display
    void displayTransaction() const;
    TransactionDTO toDTO() const;

    // Static Utility Helpers
    static std::string generateTransactionId();
    static std::string getCurrentTimestamp();
    static std::string typeToString(TransactionType type);
    static std::string statusToString(TransactionStatus status);
};
