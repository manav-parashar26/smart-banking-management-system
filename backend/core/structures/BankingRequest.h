#pragma once

#include <string>
#include <iostream>
#include <iomanip>

// ============================================================================
// BANKING REQUEST (For Admin Approval Queue)
// Demonstrating DSA: FIFO Queue Element
// ============================================================================

enum class RequestType {
    OVERDRAFT_LIMIT_INCREASE,
    LOAN_APPLICATION,
    ACCOUNT_UNBLOCK
};

enum class RequestStatus {
    PENDING,
    APPROVED,
    REJECTED
};

struct BankingRequest {
    std::string requestId;
    std::string customerId;
    std::string accountNumber;
    RequestType type;
    double requestedAmount;
    RequestStatus status;
    std::string notes;

    static std::string typeToString(RequestType t) {
        switch (t) {
            case RequestType::OVERDRAFT_LIMIT_INCREASE: return "OVERDRAFT_INCREASE";
            case RequestType::LOAN_APPLICATION: return "LOAN_APPLICATION";
            case RequestType::ACCOUNT_UNBLOCK: return "ACCOUNT_UNBLOCK";
            default: return "UNKNOWN";
        }
    }

    static std::string statusToString(RequestStatus s) {
        switch (s) {
            case RequestStatus::PENDING: return "PENDING";
            case RequestStatus::APPROVED: return "APPROVED";
            case RequestStatus::REJECTED: return "REJECTED";
            default: return "UNKNOWN";
        }
    }

    void display() const {
        std::cout << "Request ID: " << requestId 
                  << " | Customer: " << customerId 
                  << " | Account: " << accountNumber
                  << " | Type: " << typeToString(type)
                  << " | Amount: $" << std::fixed << std::setprecision(2) << requestedAmount
                  << " | Status: " << statusToString(status)
                  << " | Notes: " << notes << "\n";
    }
};
