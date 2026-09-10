#pragma once

#include <string>
#include <vector>
#include "BankSystem.h"
#include "TransactionAnalytics.h"
#include "TransactionGraph.h"

// ============================================================================
// DATA STRUCTURES & ALGORITHMS (DSA) BENCHMARKING & VALIDATION SUITE
// ============================================================================
// Academic & Viva Proof of:
// 1. std::vector        : Contiguous memory layout, capacity geometric doubling O(1) amortized
// 2. std::unordered_map : Hash table bucket distribution, load factor, O(1) avg lookup vs O(N) linear
// 3. std::queue         : FIFO pipeline ordering for loan & request processing
// 4. std::stack         : LIFO cascading transaction rollback and balance state restoration
// 5. std::map           : Red-Black self-balancing BST with O(log N) ordered key iteration
// 6. std::set           : Balanced BST O(log N) deduplication of transaction counterparties
// 7. TransactionGraph   : Directed graph adjacency list & DFS circular wash-trading AML cycle detection
// 8. Algorithms         : O(log N) Binary Search (std::lower_bound) vs O(N) Linear Search; std::sort lambdas
// ============================================================================

class DSABenchmark {
public:
    // 1. Vector: Capacity growth & contiguous memory address verification
    static void benchmarkVectorReallocation();

    // 2. Unordered Map: Hash Table O(1) avg lookup vs O(N) linear scan over 10,000 elements
    static void benchmarkUnorderedMapVsLinear();

    // 3. Queue: Strict FIFO ordering verification for multi-request pipeline
    static void verifyQueueFIFOOrdering(BankSystem& bank);

    // 4. Stack: LIFO multi-level cascading transaction rollback and state restoration
    static void verifyStackLIFORollback(BankSystem& bank);

    // 5. Map: Self-balancing BST O(log N) ordered key verification
    static void verifyMapRedBlackTreeOrdering();

    // 6. Set: Balanced BST unique counterparty deduplication verification
    static void verifySetDeduplication();

    // 7. Graph: Adjacency List & DFS circular cycle detection (AML scheme)
    static void verifyGraphCycleDetection();

    // 8. Binary Search vs Linear Search: Algorithmic speedup proof
    static void verifyBinarySearchVsLinear();

    // Orchestrator: Runs all DSA verifications & benchmarks
    static void runAllDSABenchmarks(BankSystem& bank);
};
