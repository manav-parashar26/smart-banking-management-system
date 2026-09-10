#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <iostream>
#include "Transaction.h"

// ============================================================================
// TRANSACTION RELATIONSHIP NETWORK (DIRECTED GRAPH)
// Demonstrating DSA: Graph using Adjacency List & Cycle Detection (DFS)
// Why Appropriate: Models inter-account fund flows as directed edges u -> v.
// Space Complexity: O(V + E) where V = accounts, E = transfers.
// Time Complexity : O(V + E) for Graph Traversal / Cycle Detection.
// ============================================================================

struct TransferEdge {
    std::string toAccount;
    double amount;
    std::string txnId;
    std::string timestamp;
};

class TransactionGraph {
private:
    // Adjacency List: Account Number -> List of outgoing transfer edges
    std::unordered_map<std::string, std::vector<TransferEdge>> m_adjList;
    std::set<std::string> m_vertices; // Set of all accounts in the network

    // DFS Cycle Detection Helper
    void dfsCycleHelper(const std::string& current, 
                        const std::string& start,
                        std::vector<std::string>& path, 
                        std::unordered_map<std::string, bool>& visited, 
                        std::unordered_map<std::string, bool>& inStack,
                        std::vector<std::vector<std::string>>& detectedCycles,
                        int maxCycleLength);

public:
    TransactionGraph() = default;

    // Graph Construction
    void addEdge(const std::string& fromAcc, 
                 const std::string& toAcc, 
                 double amount, 
                 const std::string& txnId, 
                 const std::string& timestamp = "");

    void populateFromLedger(const std::vector<Transaction>& transactions);

    // Topological Exploration
    void displayNetworkTopology() const;
    double getTotalFlow(const std::string& fromAcc, const std::string& toAcc) const;

    // Forensic AML Algorithm: Circular Routing Detection
    // Detects if funds cycle in a closed loop (e.g., A -> B -> C -> A)
    std::vector<std::vector<std::string>> findCircularRoutingRings(int maxCycleLength = 5);

    // Pure Data Getters for REST API & UI Visualization
    const std::unordered_map<std::string, std::vector<TransferEdge>>& getAdjacencyList() const { return m_adjList; }
    const std::set<std::string>& getVertices() const { return m_vertices; }
};
