#pragma once

#include <vector>
#include <map>
#include <set>
#include <string>
#include "Transaction.h"

// ============================================================================
// TRANSACTION ANALYTICS & DSA ALGORITHMS ENGINE
// Demonstrating: std::sort (lambdas), binary_search, std::map (ordered report),
//                std::set (unique categories/partners)
// ============================================================================

class TransactionAnalytics {
public:
    // 1. Sorting Algorithms: O(N log N)
    static void sortByAmountDescending(std::vector<Transaction>& txns);
    static void sortByTimestamp(std::vector<Transaction>& txns, bool ascending = true);

    // 2. Searching Algorithms:
    // Linear predicate search: O(N)
    static std::vector<Transaction> filterByType(const std::vector<Transaction>& txns, TransactionType type);
    static std::vector<Transaction> filterByMinAmount(const std::vector<Transaction>& txns, double minAmount);

    // Binary Search: O(log N) - Requires sorted array
    static const Transaction* binarySearchById(const std::vector<Transaction>& sortedTxns, const std::string& txnId);

    // 3. Ordered Reports using STL std::map (Red-Black Tree: O(log K) per insert, keys sorted)
    static std::map<std::string, double> generateCategorySpendingSummary(const std::vector<Transaction>& txns);
    static void printCategoryReport(const std::map<std::string, double>& report);

    // 4. Unique Set Collection using STL std::set (Balanced BST: O(log U) uniqueness guarantee)
    static std::set<std::string> getUniqueTransactionCounterparties(const std::vector<Transaction>& txns, const std::string& myAccount);
    static void printUniqueCounterparties(const std::set<std::string>& parties);
};
