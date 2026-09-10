#include "SmartAuditor.h"
#include <iomanip>
#include <sstream>
#include <cmath>

static uint64_t s_alertCounter = 7001;

void AnomalyAlert::display() const {
    std::cout << "  [" << severityToString(severity) << " ALERT #" << alertId << "] "
              << "Account: " << accountNumber 
              << " | Txn: " << txnId 
              << " | Rule: " << ruleName << "\n"
              << "     Trigger Amount: $" << std::fixed << std::setprecision(2) << triggeredAmount << "\n"
              << "     Rationale     : " << rationale << "\n";
}

void FinancialHealthReport::display() const {
    std::cout << "\n==============================================================================\n";
    std::cout << "                 EDUCATIONAL FINANCIAL WELLNESS & EXPENSE REPORT              \n";
    std::cout << "==============================================================================\n";
    std::cout << "Total Recorded Inflows (Income)   : $" << std::fixed << std::setprecision(2) << totalIncome << "\n";
    std::cout << "Total Recorded Outflows (Expenses): $" << totalExpenses << "\n";
    std::cout << "Net Savings Surplus / Deficit     : $" << netSavings << "\n";
    std::cout << "Savings Rate Percentage           : " << savingsRate << "%\n";
    std::cout << "Financial Health Status           : [" << statusRating << "]\n";
    std::cout << "Smart Financial Advisory Tip      : " << recommendation << "\n";
    std::cout << "==============================================================================\n\n";
}

std::vector<AnomalyAlert> SmartAuditor::detectOutlierSpikes(const std::vector<Transaction>& history, 
                                                          const std::string& accountNo,
                                                          double multiplier) {
    std::vector<AnomalyAlert> alerts;
    if (history.size() < 3) return alerts;

    // Calculate baseline average of prior transactions
    double sum = 0.0;
    int count = 0;
    for (const auto& t : history) {
        if (t.getStatus() == TransactionStatus::SUCCESS) {
            sum += t.getAmount();
            count++;
        }
    }
    if (count == 0) return alerts;
    double average = sum / count;

    for (const auto& t : history) {
        if (t.getStatus() == TransactionStatus::SUCCESS && t.getAmount() > (average * multiplier) && t.getAmount() > 1000.0) {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2);
            oss << "Educational Alert: Transaction of $" << t.getAmount() 
                << " is " << (t.getAmount() / average) << "x above the historical account mean ($" << average << ").";

            alerts.push_back(AnomalyAlert{
                "ALT-" + std::to_string(s_alertCounter++),
                accountNo,
                t.getTransactionId(),
                "Statistical Outlier Spike",
                AlertSeverity::HIGH,
                t.getAmount(),
                oss.str()
            });
        }
    }
    return alerts;
}

std::vector<AnomalyAlert> SmartAuditor::detectVelocityBursts(const std::vector<Transaction>& history, 
                                                           const std::string& accountNo,
                                                           size_t maxTxnsThreshold) {
    std::vector<AnomalyAlert> alerts;
    if (history.size() >= maxTxnsThreshold) {
        std::ostringstream oss;
        oss << "Educational Alert: High-frequency transaction velocity (" 
            << history.size() << " transactions logged in active window).";

        alerts.push_back(AnomalyAlert{
            "ALT-" + std::to_string(s_alertCounter++),
            accountNo,
            history.back().getTransactionId(),
            "High Velocity Activity",
            AlertSeverity::MEDIUM,
            0.0,
            oss.str()
        });
    }
    return alerts;
}

std::vector<AnomalyAlert> SmartAuditor::detectRapidDrainage(const std::vector<Transaction>& history,
                                                          const std::string& accountNo) {
    std::vector<AnomalyAlert> alerts;
    if (history.size() < 2) return alerts;

    for (size_t i = 1; i < history.size(); ++i) {
        const auto& prev = history[i - 1];
        const auto& curr = history[i];

        // If a large inflow was followed immediately by a heavy outflow (> 80% of inflow)
        if ((prev.getType() == TransactionType::DEPOSIT || prev.getType() == TransactionType::TRANSFER_IN) &&
            (curr.getType() == TransactionType::WITHDRAWAL || curr.getType() == TransactionType::TRANSFER_OUT)) {
            
            if (prev.getAmount() >= 2000.0 && curr.getAmount() >= (prev.getAmount() * 0.80)) {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2);
                oss << "Educational Alert: Rapid drainage pattern. Outflow of $" 
                    << curr.getAmount() << " (" << ((curr.getAmount() / prev.getAmount()) * 100.0)
                    << "%) immediately followed deposit of $" << prev.getAmount() << ".";

                alerts.push_back(AnomalyAlert{
                    "ALT-" + std::to_string(s_alertCounter++),
                    accountNo,
                    curr.getTransactionId(),
                    "Rapid Balance Drainage",
                    AlertSeverity::CRITICAL,
                    curr.getAmount(),
                    oss.str()
                });
            }
        }
    }
    return alerts;
}

std::vector<AnomalyAlert> SmartAuditor::runFullAudit(const Account& account) {
    const auto& history = account.getTransactionHistory();
    const std::string& accNo = account.getAccountNumber();

    std::vector<AnomalyAlert> allAlerts;
    auto outliers = detectOutlierSpikes(history, accNo);
    auto velocity = detectVelocityBursts(history, accNo, 5);
    auto drainage = detectRapidDrainage(history, accNo);

    allAlerts.insert(allAlerts.end(), outliers.begin(), outliers.end());
    allAlerts.insert(allAlerts.end(), velocity.begin(), velocity.end());
    allAlerts.insert(allAlerts.end(), drainage.begin(), drainage.end());

    return allAlerts;
}

FinancialHealthReport SmartAuditor::evaluateFinancialHealth(const std::vector<Transaction>& history) {
    double income = 0.0;
    double expenses = 0.0;

    for (const auto& t : history) {
        if (t.getStatus() == TransactionStatus::SUCCESS) {
            if (t.getType() == TransactionType::DEPOSIT || t.getType() == TransactionType::TRANSFER_IN) {
                income += t.getAmount();
            } else if (t.getType() == TransactionType::WITHDRAWAL || t.getType() == TransactionType::TRANSFER_OUT) {
                expenses += t.getAmount();
            }
        }
    }

    double net = income - expenses;
    double rate = income > 0.0 ? ((net / income) * 100.0) : 0.0;

    std::string rating;
    std::string tip;

    if (rate >= 30.0) {
        rating = "EXCELLENT - Prime Financial Solvency";
        tip = "Consistently maintains high savings rate; ideal candidate for premium savings or high-yield fixed deposits.";
    } else if (rate >= 10.0) {
        rating = "STABLE - Balanced Cash Flow";
        tip = "Healthy reserve margin; consider setting up automated recurring transfers to savings to grow cushion.";
    } else if (rate >= 0.0) {
        rating = "MODERATE - Narrow Surplus Buffer";
        tip = "Spending is close to monthly inflow; minimize non-essential discretionary expenses.";
    } else {
        rating = "DEFICIT WARNING - Overspending Alert";
        tip = "Expenditure exceeds deposits; rely less on overdrafts to prevent compounding debt.";
    }

    return FinancialHealthReport{income, expenses, net, rate, rating, tip};
}
