#pragma once

#include <string>
#include <iostream>
#include "Transaction.h"

// ============================================================================
// TRANSACTION REVERSAL (For Undo Stack Simulation)
// Demonstrating DSA: LIFO Stack Element
// ============================================================================

struct TransactionReversal {
    std::string reversalId;
    std::string originalTxnId;
    std::string sourceAccount;
    std::string targetAccount;
    double amount;
    TransactionType originalType;
    std::string reason;

    void display() const {
        std::cout << "Reversal ID: " << reversalId
                  << " | Ref Txn: " << originalTxnId
                  << " | Source: " << sourceAccount
                  << " | Target: " << targetAccount
                  << " | Amount: $" << amount
                  << " | Type: " << Transaction::typeToString(originalType)
                  << " | Reason: " << reason << "\n";
    }
};
