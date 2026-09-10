#include "DSABenchmark.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cassert>
#include <algorithm>
#include <random>

void DSABenchmark::benchmarkVectorReallocation() {
    std::cout << "\n==============================================================================\n";
    std::cout << " [1/8] DSA BENCHMARK: std::vector Dynamic Array & Memory Geometric Doubling   \n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: Vector maintains a contiguous memory buffer. When size reaches capacity,\n"
              << "         it allocates a new buffer of ~2x capacity, moves elements, and frees old memory.\n"
              << "         Amortized push_back complexity: O(1). Worst-case reallocation: O(N).\n\n";

    std::vector<int> dynamicVec;
    size_t prevCap = dynamicVec.capacity();
    std::cout << "Tracking capacity growth for 20 sequential push_back operations:\n";
    std::cout << "Step | Size | Capacity | Reallocated? | Memory Address of First Element\n";
    std::cout << "-----+------+----------+--------------+---------------------------------\n";

    for (int i = 1; i <= 20; ++i) {
        dynamicVec.push_back(i * 100);
        bool reallocated = (dynamicVec.capacity() != prevCap);
        if (reallocated || i == 1 || i == 20) {
            std::cout << std::setw(4) << i << " | "
                      << std::setw(4) << dynamicVec.size() << " | "
                      << std::setw(8) << dynamicVec.capacity() << " | "
                      << (reallocated ? "YES (Growth) " : "NO (Reused)  ") << " | "
                      << static_cast<const void*>(dynamicVec.data()) << "\n";
            prevCap = dynamicVec.capacity();
        }
    }

    // Contiguous Memory Proof
    bool isContiguous = true;
    for (size_t i = 0; i < dynamicVec.size(); ++i) {
        if (&dynamicVec[i] != dynamicVec.data() + i) {
            isContiguous = false;
            break;
        }
    }
    std::cout << "\n[✓] Contiguous Memory Verification: " 
              << (isContiguous ? "PASSED (&vec[i] == base + i * sizeof(T))" : "FAILED") << "\n";

    // Timing with reserve() vs without reserve()
    const int N = 100000;
    auto t1 = std::chrono::high_resolution_clock::now();
    std::vector<int> unreserved;
    for (int i = 0; i < N; ++i) unreserved.push_back(i);
    auto t2 = std::chrono::high_resolution_clock::now();
    double unreservedTime = std::chrono::duration<double, std::micro>(t2 - t1).count();

    auto t3 = std::chrono::high_resolution_clock::now();
    std::vector<int> reserved;
    reserved.reserve(N);
    for (int i = 0; i < N; ++i) reserved.push_back(i);
    auto t4 = std::chrono::high_resolution_clock::now();
    double reservedTime = std::chrono::duration<double, std::micro>(t4 - t3).count();

    std::cout << "[*] Insertion of " << N << " items without reserve(): " << unreservedTime << " µs\n";
    std::cout << "[*] Insertion of " << N << " items WITH reserve()   : " << reservedTime << " µs\n";
    std::cout << "[✓] reserve() Optimization: ~" << std::fixed << std::setprecision(1) 
              << (unreservedTime / (reservedTime > 0 ? reservedTime : 1.0)) << "x faster by avoiding reallocations.\n";
}

void DSABenchmark::benchmarkUnorderedMapVsLinear() {
    std::cout << "\n==============================================================================\n";
    std::cout << " [2/8] DSA BENCHMARK: std::unordered_map (Hash Table O(1)) vs Linear Scan O(N)\n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: std::unordered_map uses hash buckets and chaining.\n"
              << "         Average search complexity: O(1). Linear array search: O(N).\n\n";

    const int TOTAL_ACCOUNTS = 10000;
    std::unordered_map<std::string, double> accountMap;
    std::vector<std::pair<std::string, double>> accountVector;

    accountMap.reserve(TOTAL_ACCOUNTS);
    accountVector.reserve(TOTAL_ACCOUNTS);

    for (int i = 0; i < TOTAL_ACCOUNTS; ++i) {
        std::string accNo = "ACC-" + std::to_string(10000 + i);
        double balance = 500.0 + (i * 1.5);
        accountMap[accNo] = balance;
        accountVector.push_back({accNo, balance});
    }

    std::cout << "Hash Table Internal Metrics for " << TOTAL_ACCOUNTS << " accounts:\n";
    std::cout << " - Bucket Count : " << accountMap.bucket_count() << "\n";
    std::cout << " - Load Factor  : " << std::setprecision(4) << accountMap.load_factor() << "\n";
    std::cout << " - Max Load Fact: " << accountMap.max_load_factor() << "\n\n";

    // Search Benchmark over 5,000 lookups
    const int NUM_LOOKUPS = 5000;
    std::vector<std::string> searchKeys;
    searchKeys.reserve(NUM_LOOKUPS);
    for (int i = 0; i < NUM_LOOKUPS; ++i) {
        searchKeys.push_back("ACC-" + std::to_string(10000 + (i * 7 % TOTAL_ACCOUNTS)));
    }

    // 1. Hash Table Lookups (O(1) average)
    auto t1 = std::chrono::high_resolution_clock::now();
    double mapSum = 0.0;
    for (const auto& key : searchKeys) {
        auto it = accountMap.find(key);
        if (it != accountMap.end()) mapSum += it->second;
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double mapTime = std::chrono::duration<double, std::micro>(t2 - t1).count();
    if (mapTime == 0.0) mapTime = 1.0; // Floor to 1 µs for resolution precision

    // 2. Linear Scan Lookups (O(N))
    auto t3 = std::chrono::high_resolution_clock::now();
    double vecSum = 0.0;
    for (const auto& key : searchKeys) {
        for (const auto& item : accountVector) {
            if (item.first == key) {
                vecSum += item.second;
                break;
            }
        }
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    double vecTime = std::chrono::duration<double, std::micro>(t4 - t3).count();

    std::cout << "[*] " << NUM_LOOKUPS << " lookups in std::unordered_map [O(1)] : " << mapTime << " µs\n";
    std::cout << "[*] " << NUM_LOOKUPS << " lookups in std::vector        [O(N)] : " << vecTime << " µs\n";
    std::cout << "[✓] Hash Table Speedup: ~" << std::fixed << std::setprecision(1) 
              << (vecTime / mapTime) << "x faster than linear scan!\n";
    assert(mapSum == vecSum);
}

void DSABenchmark::verifyQueueFIFOOrdering(BankSystem& bank) {
    std::cout << "\n==============================================================================\n";
    std::cout << " [3/8] DSA VERIFICATION: std::queue FIFO (First-In, First-Out) Pipeline        \n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: Banking requests must be serviced strictly in arrival sequence.\n"
              << "         std::queue enforces FIFO ordering: enqueue via push(), service via front() + pop().\n\n";

    // Enqueue 3 distinct requests
    std::string req1 = bank.submitBankingRequest("CUST-101", "SAV-1001", RequestType::LOAN_APPLICATION, 1000.0, "Small Business Microloan");
    std::string req2 = bank.submitBankingRequest("CUST-101", "SAV-1001", RequestType::ACCOUNT_UNBLOCK, 0.0, "Security Clear Review");
    std::string req3 = bank.submitBankingRequest("CUST-101", "SAV-1001", RequestType::LOAN_APPLICATION, 5000.0, "Equipment Expansion");

    std::cout << "Enqueued 3 requests in order: [1: " << req1 << "], [2: " << req2 << "], [3: " << req3 << "]\n";
    std::cout << "Pending requests in queue: " << bank.getPendingRequestCount() << "\n\n";

    // Process first request -> Must be req1
    std::string msg;
    bool ok1 = bank.processNextBankingRequest(true, msg);
    std::cout << "Service Step 1 -> " << msg << "\n";
    assert(ok1 && msg.find(req1) != std::string::npos);

    // Process second request -> Must be req2
    bool ok2 = bank.processNextBankingRequest(true, msg);
    std::cout << "Service Step 2 -> " << msg << "\n";
    assert(ok2 && msg.find(req2) != std::string::npos);

    // Process third request -> Must be req3
    bool ok3 = bank.processNextBankingRequest(false, msg); // Reject
    std::cout << "Service Step 3 -> " << msg << "\n";
    assert(ok3 && msg.find(req3) != std::string::npos);

    std::cout << "\n[✓] FIFO Sequence Integrity: PASSED (Exact arrival order preserved: REQ1 -> REQ2 -> REQ3)\n";
}

void DSABenchmark::verifyStackLIFORollback(BankSystem& bank) {
    std::cout << "\n==============================================================================\n";
    std::cout << " [4/8] DSA VERIFICATION: std::stack LIFO (Last-In, First-Out) Cascade Undo    \n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: Multi-step financial transactions form a compensation stack.\n"
              << "         Undo operations pop the top element (most recent) in strict reverse order.\n\n";

    auto savAcc = bank.getAccount("SAV-1001");
    auto curAcc = bank.getAccount("CUR-1002");
    if (!savAcc || !curAcc) return;

    double initialSavBal = savAcc->getBalance();
    double initialCurBal = curAcc->getBalance();

    std::cout << "Baseline Balances:\n";
    std::cout << " - SAV-1001: $" << std::fixed << std::setprecision(2) << initialSavBal << "\n";
    std::cout << " - CUR-1002: $" << initialCurBal << "\n\n";

    // Execute Sequence of 3 Operations:
    // Op 1: Deposit $300 to SAV-1001
    bank.executeDeposit("SAV-1001", 300.0, "Test Step 1: Deposit");
    // Op 2: Withdraw $100 from SAV-1001
    bank.executeWithdrawal("SAV-1001", 100.0, "Test Step 2: Withdrawal");
    // Op 3: Transfer $250 from SAV-1001 to CUR-1002
    bank.executeTransfer("SAV-1001", "CUR-1002", 250.0, "Test Step 3: Transfer");

    std::cout << "Balances after 3 operations (Deposit $300, Withdraw $100, Transfer $250):\n";
    std::cout << " - SAV-1001: $" << savAcc->getBalance() << "\n";
    std::cout << " - CUR-1002: $" << curAcc->getBalance() << "\n";
    std::cout << " - Stack depth: " << bank.getReversalStackSize() << " undoable operations\n\n";

    // Rollback 1: Must undo Op 3 (Transfer $250)
    std::string msg;
    bank.undoLastTransaction(msg);
    std::cout << "Rollback 1 [Op 3 Transfer Undo]: " << msg << "\n";
    assert(curAcc->getBalance() == initialCurBal);

    // Rollback 2: Must undo Op 2 (Withdrawal $100)
    bank.undoLastTransaction(msg);
    std::cout << "Rollback 2 [Op 2 Withdrawal Undo]: " << msg << "\n";

    // Rollback 3: Must undo Op 1 (Deposit $300)
    bank.undoLastTransaction(msg);
    std::cout << "Rollback 3 [Op 1 Deposit Undo]: " << msg << "\n";

    // Balances must be exactly back to initial values
    std::cout << "\nPost-Rollback Balances:\n";
    std::cout << " - SAV-1001: $" << savAcc->getBalance() << " (Expected: $" << initialSavBal << ")\n";
    std::cout << " - CUR-1002: $" << curAcc->getBalance() << " (Expected: $" << initialCurBal << ")\n";

    assert(savAcc->getBalance() == initialSavBal);
    assert(curAcc->getBalance() == initialCurBal);
    std::cout << "[✓] LIFO Rollback Cascade Integrity: PASSED (Exact reverse restoration: Op3 -> Op2 -> Op1)\n";
}

void DSABenchmark::verifyMapRedBlackTreeOrdering() {
    std::cout << "\n==============================================================================\n";
    std::cout << " [5/8] DSA VERIFICATION: std::map Self-Balancing Red-Black Tree O(log N)       \n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: std::map is implemented as a Red-Black Tree. Keys are ordered strictly\n"
              << "         upon insertion in O(log N) time. In-order traversal yields sorted order.\n\n";

    std::map<std::string, double> categoryBudget;
    // Insert keys in completely arbitrary, unsorted order
    categoryBudget["Utilities"] = 180.00;
    categoryBudget["Salary Income"] = 5200.00;
    categoryBudget["Dining & Cafe"] = 95.50;
    categoryBudget["Groceries"] = 340.25;
    categoryBudget["ATM Cash Outflow"] = 200.00;
    categoryBudget["Business Equipment"] = 1250.00;

    std::cout << "Inserted 6 categories in random order. Iterating std::map:\n";
    std::string previousKey = "";
    bool isSorted = true;

    for (const auto& pair : categoryBudget) {
        std::cout << " -> " << std::left << std::setw(24) << pair.first 
                  << " : $" << std::right << std::setw(8) << std::fixed << std::setprecision(2) << pair.second << "\n";
        if (!previousKey.empty() && pair.first < previousKey) {
            isSorted = false;
        }
        previousKey = pair.first;
    }

    assert(isSorted);
    std::cout << "\n[✓] Red-Black Tree In-Order Traversal: PASSED (Keys strictly sorted alphabetically without std::sort)\n";
}

void DSABenchmark::verifySetDeduplication() {
    std::cout << "\n==============================================================================\n";
    std::cout << " [6/8] DSA VERIFICATION: std::set Balanced BST Uniqueness & Deduplication     \n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: std::set maintains unique elements in a balanced binary search tree.\n"
              << "         Duplicate insertions are silently rejected in O(log N) time.\n\n";

    std::vector<std::string> rawCounterparties = {
        "Amazon.com", "Netflix", "Amazon.com", "Starbucks",
        "Netflix", "Apple Inc.", "Amazon.com", "Chevron Gas", "Starbucks"
    };

    std::cout << "Raw incoming counterparties (" << rawCounterparties.size() << " transactions with duplicates):\n";
    for (const auto& c : rawCounterparties) std::cout << c << ", ";
    std::cout << "\n\n";

    std::set<std::string> uniqueParties;
    for (const auto& c : rawCounterparties) {
        uniqueParties.insert(c);
    }

    std::cout << "Unique counterparties after std::set insertion (" << uniqueParties.size() << " unique entries):\n";
    for (const auto& u : uniqueParties) {
        std::cout << " -> " << u << "\n";
    }

    assert(uniqueParties.size() == 5);
    std::cout << "\n[✓] Balanced BST Deduplication: PASSED (9 raw transactions condensed to 5 unique entities)\n";
}

void DSABenchmark::verifyGraphCycleDetection() {
    std::cout << "\n==============================================================================\n";
    std::cout << " [7/8] DSA VERIFICATION: Directed Graph Adjacency List & DFS Cycle Detection   \n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: Accounts form vertices V, fund transfers form directed edges E (u -> v).\n"
              << "         Anti-Money Laundering (AML) wash trading is detected via DFS cycle detection.\n\n";

    // Test 1: Construct a 4-node circular wash-trading loop: ACC-A -> ACC-B -> ACC-C -> ACC-D -> ACC-A
    TransactionGraph cyclicGraph;
    cyclicGraph.addEdge("ACC-A", "ACC-B", 10000.0, "TXN-01");
    cyclicGraph.addEdge("ACC-B", "ACC-C",  9900.0, "TXN-02");
    cyclicGraph.addEdge("ACC-C", "ACC-D",  9800.0, "TXN-03");
    cyclicGraph.addEdge("ACC-D", "ACC-A",  9700.0, "TXN-04"); // Completes circular loop

    auto rings = cyclicGraph.findCircularRoutingRings(5);
    std::cout << "Test 1 (Circular Network A -> B -> C -> D -> A):\n";
    std::cout << " - Detected Closed Cycles: " << rings.size() << "\n";
    if (!rings.empty()) {
        std::cout << " - Ring Path: ";
        for (size_t i = 0; i < rings[0].size(); ++i) {
            std::cout << rings[0][i] << (i + 1 < rings[0].size() ? " ──> " : "\n");
        }
    }
    assert(!rings.empty());

    // Test 2: Acyclic tree structure (ACC-X -> ACC-Y -> ACC-Z): Should detect 0 cycles
    TransactionGraph acyclicGraph;
    acyclicGraph.addEdge("ACC-X", "ACC-Y", 500.0, "TXN-10");
    acyclicGraph.addEdge("ACC-Y", "ACC-Z", 300.0, "TXN-11");
    auto acyclicRings = acyclicGraph.findCircularRoutingRings(5);
    std::cout << "\nTest 2 (Linear Acyclic Network X -> Y -> Z):\n";
    std::cout << " - Detected Closed Cycles: " << acyclicRings.size() << " (Expected: 0)\n";
    assert(acyclicRings.empty());

    std::cout << "\n[✓] DFS Cycle Detection Integrity: PASSED (Accurately identifies loops; zero false positives)\n";
}

void DSABenchmark::verifyBinarySearchVsLinear() {
    std::cout << "\n==============================================================================\n";
    std::cout << " [8/8] ALGORITHMIC VERIFICATION: O(log N) Binary Search vs O(N) Linear Search \n";
    std::cout << "==============================================================================\n";
    std::cout << "Concept: std::lower_bound executes Binary Search in O(log N) time on sorted data.\n"
              << "         Linear search requires O(N) comparisons.\n\n";

    const int DATA_SIZE = 5000;
    std::vector<Transaction> sortedTxns;
    sortedTxns.reserve(DATA_SIZE);

    for (int i = 0; i < DATA_SIZE; ++i) {
        // Pad with leading zeros to maintain lexicographical sort order
        std::string txnId = "TXN-" + std::string(6 - std::to_string(i).length(), '0') + std::to_string(i);
        sortedTxns.emplace_back(txnId, "ACC-1001", "ACC-1002", 100.0 + i, TransactionType::TRANSFER_OUT, TransactionStatus::SUCCESS, "Transfer #" + std::to_string(i));
    }

    std::string searchTarget = "TXN-003750"; // Towards the 75% mark

    const int NUM_SEARCHES = 1000;
    // 1. Binary Search: O(log N)
    auto t1 = std::chrono::high_resolution_clock::now();
    const Transaction* bsResult = nullptr;
    for (int i = 0; i < NUM_SEARCHES; ++i) {
        bsResult = TransactionAnalytics::binarySearchById(sortedTxns, searchTarget);
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double bsTime = std::chrono::duration<double, std::micro>(t2 - t1).count();
    if (bsTime == 0.0) bsTime = 1.0;

    // 2. Linear Search: O(N)
    auto t3 = std::chrono::high_resolution_clock::now();
    const Transaction* lsResult = nullptr;
    for (int i = 0; i < NUM_SEARCHES; ++i) {
        lsResult = nullptr;
        for (const auto& t : sortedTxns) {
            if (t.getTransactionId() == searchTarget) {
                lsResult = &t;
                break;
            }
        }
    }
    auto t4 = std::chrono::high_resolution_clock::now();
    double lsTime = std::chrono::duration<double, std::micro>(t4 - t3).count();

    assert(bsResult != nullptr && lsResult != nullptr);
    assert(bsResult->getTransactionId() == lsResult->getTransactionId());

    std::cout << "Target Item: " << searchTarget << " across " << DATA_SIZE << " sorted elements (" << NUM_SEARCHES << " searches):\n";
    std::cout << " - Binary Search [O(log N)] : " << std::fixed << std::setprecision(2) << bsTime << " µs (log2(" << DATA_SIZE << ") ≈ 13 comparisons per search)\n";
    std::cout << " - Linear Search [O(N)]     : " << lsTime << " µs (average ~2,500 comparisons per search)\n";
    std::cout << "[✓] Binary Search Speedup: ~" << std::fixed << std::setprecision(1) 
              << (lsTime / bsTime) << "x faster than linear scan!\n";
}

void DSABenchmark::runAllDSABenchmarks(BankSystem& bank) {
    std::cout << "\n========================================================================================================\n";
    std::cout << "                     COMPREHENSIVE C++ DATA STRUCTURES & ALGORITHMS (DSA) SUITE                         \n";
    std::cout << "========================================================================================================\n";

    benchmarkVectorReallocation();
    benchmarkUnorderedMapVsLinear();
    verifyQueueFIFOOrdering(bank);
    verifyStackLIFORollback(bank);
    verifyMapRedBlackTreeOrdering();
    verifySetDeduplication();
    verifyGraphCycleDetection();
    verifyBinarySearchVsLinear();

    std::cout << "\n========================================================================================================\n";
    std::cout << "                  ALL 8 DSA BENCHMARKS & VERIFICATIONS COMPLETED SUCCESSFULLY!                          \n";
    std::cout << "========================================================================================================\n\n";
}
