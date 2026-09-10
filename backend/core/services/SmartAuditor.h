#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "Transaction.h"
#include "Account.h"

// ============================================================================
// SMART AUDITOR & EDUCATIONAL ANOMALY DETECTION ENGINE
// Implements explainable rule-based algorithms for educational analysis.
// NOTE: Educational simulation, not a real banking-grade fraud detection system.
// ============================================================================

enum class AlertSeverity {
    INFO,
    MEDIUM,
    HIGH,
    CRITICAL
};

struct AnomalyAlert {
    std::string alertId;
    std::string accountNumber;
    std::string txnId;
    std::string ruleName;
    AlertSeverity severity;
    double triggeredAmount;
    std::string rationale;

    static std::string severityToString(AlertSeverity s) {
        switch (s) {
            case AlertSeverity::INFO: return "INFO";
            case AlertSeverity::MEDIUM: return "MEDIUM";
            case AlertSeverity::HIGH: return "HIGH";
            case AlertSeverity::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    void display() const;
};

struct FinancialHealthReport {
    double totalIncome;
    double totalExpenses;
    double netSavings;
    double savingsRate; // Percentage
    std::string statusRating;
    std::string recommendation;

    void display() const;
};

class SmartAuditor {
public:
    // Rule 1: Statistical Outlier Amount Spike (> 3x historical average)
    static std::vector<AnomalyAlert> detectOutlierSpikes(const std::vector<Transaction>& history, 
                                                        const std::string& accountNo,
                                                        double multiplier = 3.0);

    // Rule 2: High-Velocity Rapid Transactions (Burst activity)
    static std::vector<AnomalyAlert> detectVelocityBursts(const std::vector<Transaction>& history, 
                                                         const std::string& accountNo,
                                                         size_t maxTxnsThreshold = 4);

    // Rule 3: Rapid Drainage Pattern (Withdrawing > 85% of balance shortly after large deposit)
    static std::vector<AnomalyAlert> detectRapidDrainage(const std::vector<Transaction>& history,
                                                        const std::string& accountNo);

    // Combined Audit
    static std::vector<AnomalyAlert> runFullAudit(const Account& account);

    // Educational Financial Insights & Expense Calculator
    static FinancialHealthReport evaluateFinancialHealth(const std::vector<Transaction>& history);
};
