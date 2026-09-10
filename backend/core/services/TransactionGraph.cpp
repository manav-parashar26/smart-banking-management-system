#include "TransactionGraph.h"
#include <iomanip>

void TransactionGraph::addEdge(const std::string& fromAcc, 
                               const std::string& toAcc, 
                               double amount, 
                               const std::string& txnId, 
                               const std::string& timestamp) {
    if (fromAcc.empty() || toAcc.empty() || fromAcc == "N/A" || toAcc == "N/A") {
        return;
    }
    m_adjList[fromAcc].push_back(TransferEdge{toAcc, amount, txnId, timestamp});
    m_vertices.insert(fromAcc);
    m_vertices.insert(toAcc);
}

void TransactionGraph::populateFromLedger(const std::vector<Transaction>& transactions) {
    for (const auto& t : transactions) {
        if (t.getType() == TransactionType::TRANSFER_OUT) {
            addEdge(t.getFromAccount(), t.getToAccount(), t.getAmount(), t.getTransactionId(), t.getTimestamp());
        }
    }
}

void TransactionGraph::displayNetworkTopology() const {
    std::cout << "\n==============================================================================\n";
    std::cout << "          TRANSACTION RELATIONSHIP NETWORK (Adjacency List Directed Graph)     \n";
    std::cout << "==============================================================================\n";
    std::cout << "Network Statistics: " << m_vertices.size() << " Active Vertices (Accounts)\n\n";

    for (const auto& vertex : m_vertices) {
        std::cout << "Account [" << vertex << "]";
        auto it = m_adjList.find(vertex);
        if (it == m_adjList.end() || it->second.empty()) {
            std::cout << " -> (No outgoing fund transfers)\n";
        } else {
            std::cout << "\n";
            for (const auto& edge : it->second) {
                std::cout << "   └──> [$" << std::fixed << std::setprecision(2) << edge.amount 
                          << " via " << edge.txnId << "] ──> Account [" << edge.toAccount << "]\n";
            }
        }
    }
    std::cout << "==============================================================================\n\n";
}

double TransactionGraph::getTotalFlow(const std::string& fromAcc, const std::string& toAcc) const {
    auto it = m_adjList.find(fromAcc);
    if (it == m_adjList.end()) return 0.0;

    double total = 0.0;
    for (const auto& edge : it->second) {
        if (edge.toAccount == toAcc) {
            total += edge.amount;
        }
    }
    return total;
}

// DFS Cycle Detection: O(V + E)
void TransactionGraph::dfsCycleHelper(const std::string& current, 
                                     const std::string& start,
                                     std::vector<std::string>& path, 
                                     std::unordered_map<std::string, bool>& visited, 
                                     std::unordered_map<std::string, bool>& inStack,
                                     std::vector<std::vector<std::string>>& detectedCycles,
                                     int maxCycleLength) {
    visited[current] = true;
    inStack[current] = true;
    path.push_back(current);

    auto it = m_adjList.find(current);
    if (it != m_adjList.end() && (int)path.size() <= maxCycleLength) {
        for (const auto& edge : it->second) {
            const std::string& neighbor = edge.toAccount;

            if (neighbor == start && path.size() >= 2) {
                // Found a closed circular loop back to start!
                std::vector<std::string> cycle = path;
                cycle.push_back(start);
                detectedCycles.push_back(cycle);
            } else if (!visited[neighbor]) {
                dfsCycleHelper(neighbor, start, path, visited, inStack, detectedCycles, maxCycleLength);
            }
        }
    }

    // Backtrack
    path.pop_back();
    inStack[current] = false;
}

std::vector<std::vector<std::string>> TransactionGraph::findCircularRoutingRings(int maxCycleLength) {
    std::vector<std::vector<std::string>> detectedCycles;

    for (const auto& vertex : m_vertices) {
        std::unordered_map<std::string, bool> visited;
        std::unordered_map<std::string, bool> inStack;
        std::vector<std::string> path;

        dfsCycleHelper(vertex, vertex, path, visited, inStack, detectedCycles, maxCycleLength);
    }

    return detectedCycles;
}
