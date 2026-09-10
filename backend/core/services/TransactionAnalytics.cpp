#include "TransactionAnalytics.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

// Sorting using STL std::sort with lambda comparators (O(N log N))
void TransactionAnalytics::sortByAmountDescending(std::vector<Transaction>& txns) {
    std::sort(txns.begin(), txns.end(), [](const Transaction& a, const Transaction& b) {
        return a.getAmount() > b.getAmount();
    });
}

void TransactionAnalytics::sortByTimestamp(std::vector<Transaction>& txns, bool ascending) {
    std::sort(txns.begin(), txns.end(), [ascending](const Transaction& a, const Transaction& b) {
        return ascending ? (a.getTimestamp() < b.getTimestamp()) 
                         : (a.getTimestamp() > b.getTimestamp());
    });
}

// Searching: Linear scan filter O(N)
std::vector<Transaction> TransactionAnalytics::filterByType(const std::vector<Transaction>& txns, TransactionType type) {
    std::vector<Transaction> results;
    for (const auto& t : txns) {
        if (t.getType() == type) {
            results.push_back(t);
        }
    }
    return results;
}

std::vector<Transaction> TransactionAnalytics::filterByMinAmount(const std::vector<Transaction>& txns, double minAmount) {
    std::vector<Transaction> results;
    for (const auto& t : txns) {
        if (t.getAmount() >= minAmount) {
            results.push_back(t);
        }
    }
    return results;
}

// Binary Search: O(log N) - Assumes vector is sorted by Transaction ID
const Transaction* TransactionAnalytics::binarySearchById(const std::vector<Transaction>& sortedTxns, const std::string& txnId) {
    auto it = std::lower_bound(sortedTxns.begin(), sortedTxns.end(), txnId, 
        [](const Transaction& t, const std::string& id) {
            return t.getTransactionId() < id;
        });

    if (it != sortedTxns.end() && it->getTransactionId() == txnId) {
        return &(*it);
    }
    return nullptr;
}

// Ordered Report: std::map (Red-Black Tree: O(log K) insertion, keys stored sorted)
std::map<std::string, double> TransactionAnalytics::generateCategorySpendingSummary(const std::vector<Transaction>& txns) {
    std::map<std::string, double> categoryTotals;

    for (const auto& t : txns) {
        // Categorize based on transaction metadata & description
        std::string desc = t.getDescription();
        std::string category = "General / Miscellaneous";

        if (desc.find("Salary") != std::string::npos) {
            category = "Income: Payroll & Salary";
        } else if (desc.find("Interest") != std::string::npos) {
            category = "Income: Interest";
        } else if (desc.find("Grocery") != std::string::npos || desc.find("Utilities") != std::string::npos) {
            category = "Living Expenses: Utilities & Food";
        } else if (desc.find("Equipment") != std::string::npos || desc.find("Business") != std::string::npos) {
            category = "Business: Equipment & Capital";
        } else if (desc.find("Invoice") != std::string::npos || desc.find("Contractor") != std::string::npos) {
            category = "Business: Vendor & Contractor";
        } else if (desc.find("Loan") != std::string::npos) {
            category = "Financing: Loan Disbursal";
        } else if (desc.find("ATM") != std::string::npos) {
            category = "Cash: ATM Withdrawals";
        }

        categoryTotals[category] += t.getAmount();
    }

    return categoryTotals;
}

void TransactionAnalytics::printCategoryReport(const std::map<std::string, double>& report) {
    std::cout << "\n==============================================================================\n";
    std::cout << "          ORDERED CATEGORY BREAKDOWN REPORT (STL std::map - Red-Black Tree)    \n";
    std::cout << "==============================================================================\n";
    std::cout << std::left 
              << std::setw(38) << "Category (Automatically Sorted)" << " | "
              << "Aggregated Total\n";
    std::cout << "------------------------------------------------------------------------------\n";

    double grandTotal = 0.0;
    for (const auto& pair : report) {
        std::cout << std::left 
                  << std::setw(38) << pair.first << " | $"
                  << std::right << std::setw(10) << std::fixed << std::setprecision(2) << pair.second << "\n";
        grandTotal += pair.second;
    }
    std::cout << "------------------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(38) << "TOTAL AGGREGATE VOLUME" << " | $"
              << std::right << std::setw(10) << grandTotal << "\n";
    std::cout << "==============================================================================\n\n";
}

// Unique Counterparties: std::set ensures log(U) uniqueness & lexicographical order
std::set<std::string> TransactionAnalytics::getUniqueTransactionCounterparties(const std::vector<Transaction>& txns, const std::string& myAccount) {
    std::set<std::string> counterparties;
    for (const auto& t : txns) {
        if (!t.getFromAccount().empty() && t.getFromAccount() != "N/A" && t.getFromAccount() != myAccount) {
            counterparties.insert(t.getFromAccount());
        }
        if (!t.getToAccount().empty() && t.getToAccount() != "N/A" && t.getToAccount() != myAccount) {
            counterparties.insert(t.getToAccount());
        }
    }
    return counterparties;
}

void TransactionAnalytics::printUniqueCounterparties(const std::set<std::string>& parties) {
    std::cout << "Unique Counterparty Accounts (std::set ordered): ";
    if (parties.empty()) {
        std::cout << "None\n";
    } else {
        for (auto it = parties.begin(); it != parties.end(); ++it) {
            std::cout << *it << (std::next(it) != parties.end() ? ", " : "\n");
        }
    }
}
