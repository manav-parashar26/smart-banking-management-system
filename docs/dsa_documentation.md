# Smart Banking Management System - Data Structures & Algorithms (DSA) Technical Documentation

## Executive Summary

The **Smart Banking Management System** is engineered around a high-performance C++17 core engine that pairs real-world banking operations with rigorous computer science fundamentals. Every data structure and algorithm in the system was selected to solve a concrete financial problem: append-only ledger immutability, instant account lookup, ordered financial expenditure categorization, counterparty deduplication, regulatory request processing order, cascading transaction rollbacks, logarithmic statement searching, and forensic Anti-Money Laundering (AML) cycle detection.

All implementations referenced herein exist directly in the project codebase and have been verified against the C++ source code and the automated test suite (`SmartBankingSystem.exe --test`).

---

## 1. Master DSA Specification Table

| DSA | Implementation | Banking Use | Time | Space |
| :--- | :--- | :--- | :---: | :---: |
| **`std::vector`** | `Account::m_transactionHistory`<br>`Customer::m_linkedAccountNumbers`<br>`BankSystem::getAllTransactionsLedger` | Chronological transaction audit ledgers and customer account lists | Append: $O(1)$ amortized<br>Access: $O(1)$<br>Scan: $O(N)$ | $O(N)$ contiguous |
| **`std::unordered_map`** | `BankSystem::m_accounts`<br>`CustomerManager::m_customers`<br>`TransactionGraph::m_adjList` | Instant lookup of accounts, customers, and graph adjacency lists by string ID | Average: $O(1)$<br>Worst-Case: $O(N)$ | $O(N)$ hash buckets & nodes |
| **`std::map`** | `TransactionAnalytics::generateCategorySpendingSummary`<br>`TransactionAnalytics::printCategoryReport` | Aggregating expenditures across categories in strictly sorted alphabetical order | Search/Insert: $O(\log K)$<br>Traversal: $O(K)$ | $O(K)$ Red-Black tree nodes |
| **`std::set`** | `TransactionAnalytics::getUniqueTransactionCounterparties`<br>`TransactionGraph::m_vertices`<br>`FilePersistence::saveBankStateAtomic` | Deduplicating merchant counterparties and active network vertices | Insert/Search: $O(\log U)$<br>Traversal: $O(U)$ | $O(U)$ BST nodes |
| **`std::queue`** | `BankSystem::m_pendingRequests`<br>`BankSystem::submitBankingRequest`<br>`BankSystem::processNextBankingRequest` | Administrative FIFO approval pipeline for loans and unblock requests | Enqueue: $O(1)$<br>Dequeue: $O(1)$ | $O(R)$ sequential |
| **`std::stack`** | `BankSystem::m_reversalStack`<br>`BankSystem::undoLastTransaction`<br>`BankSystem::displayReversalStack` | Multi-step transaction rollback and cascading balance reversal | Push: $O(1)$<br>Pop: $O(1)$ | $O(S)$ sequential |
| **Binary Search** (`std::lower_bound`) | `TransactionAnalytics::binarySearchById`<br>`TransactionAnalytics::sortByTimestamp` | High-speed range queries and transaction lookups in sorted ledgers | Search: $O(\log N)$ | $O(1)$ auxiliary |
| **Directed Graph** | `TransactionGraph::m_adjList`<br>`TransactionGraph::m_vertices`<br>`TransactionGraph::addEdge` | Network modeling of inter-account fund flows with weighted edges | Add Edge: $O(1)$<br>Adj Scan: $O(\text{deg}(u))$ | $O(V + E)$ adjacency list |
| **DFS Cycle Detection** | `TransactionGraph::findCircularRoutingRings`<br>`TransactionGraph::dfsCycleHelper` | Forensic AML detection of circular wash-trading and money-laundering rings | Traversal: $O(V + E)$ | $O(V)$ recursion stack |

*Note: In the table above, $N$ denotes total transactions or accounts, $K$ denotes distinct spending categories, $U$ denotes unique counterparties/vertices, $R$ denotes pending queue requests, $S$ denotes stack reversal operations, $V$ denotes graph vertices (accounts), and $E$ denotes directed graph edges (fund transfers).*

---

## 2. In-Depth Technical Analysis by Data Structure

### 2.1. `std::vector` (Dynamic Contiguous Sequence)

#### Source Verification
- **Source Files**: `backend/core/models/Account.h`, `backend/core/models/Account.cpp`, `backend/core/models/Customer.h`, `backend/core/services/BankSystem.cpp`
- **Class / Members**:
  - `Account::m_transactionHistory` (`std::vector<Transaction>`)
  - `Customer::m_linkedAccountNumbers` (`std::vector<std::string>`)
  - `BankSystem::getAllTransactionsLedger()` (`returns std::vector<Transaction>`)

#### Banking Problem It Solves
A core requirement of financial ledgers is an immutable, append-only chronological log of all debits, credits, and transfers. Bank statements require sequential iteration over an account's history to calculate balances, generate PDF/JSON exports, and present transaction histories to customers.

#### Why This Structure Was Selected
1. **Contiguous Memory & CPU Cache Locality**: Elements in `std::vector` are laid out in a single contiguous memory buffer. When iterating through hundreds of transactions during statement generation, the CPU hardware prefetcher loads contiguous cache lines (typically 64 bytes), minimizing L1/L2 cache misses compared to node-based structures like `std::list`.
2. **$O(1)$ Amortized Append**: New transactions are appended via `.push_back()`. By doubling its internal capacity upon reaching capacity, the average cost per insertion is amortized $O(1)$, even though occasional reallocations take $O(N)$.
3. **Low Memory Overhead**: Unlike linked lists that require 16 to 24 bytes of pointer overhead per node (next and previous pointers on 64-bit platforms), `std::vector` has zero per-element memory overhead beyond object size.
4. **Fast Random Access**: Direct indexing via `operator[]` or `.at()` allows $O(1)$ constant-time retrieval of any transaction by ordinal sequence.

#### Algorithmic Complexity
- **Append (`push_back`)**: $O(1)$ amortized ($O(N)$ on geometric reallocation)
- **Random Access (`[]`)**: $O(1)$ worst-case
- **Traversal / Linear Scan**: $O(N)$
- **Space Complexity**: $O(N)$ elements plus unused reserved capacity

#### Code Example from Codebase
```cpp
// From Account.cpp: Appending transactions to the chronological audit ledger
void Account::recordTransaction(const Transaction& txn) {
    m_transactionHistory.push_back(txn); // O(1) amortized append
}
```

---

### 2.2. `std::unordered_map` (Hash Table with Bucket Chaining)

#### Source Verification
- **Source Files**: `backend/core/services/BankSystem.h`, `backend/core/services/BankSystem.cpp`, `backend/core/services/CustomerManager.h`, `backend/core/services/CustomerManager.cpp`
- **Class / Members**:
  - `BankSystem::m_accounts` (`std::unordered_map<std::string, std::shared_ptr<Account>>`)
  - `CustomerManager::m_customers` (`std::unordered_map<std::string, Customer>`)
  - `TransactionGraph::m_adjList` (`std::unordered_map<std::string, std::vector<TransferEdge>>`)

#### Banking Problem It Solves
During ATM withdrawals, point-of-sale terminal card swipes, and customer logins, the system must locate an account or customer profile in sub-millisecond time. In a bank with 100,000 accounts, a linear scan ($O(N)$) would cause unacceptable latency and thread contention under high concurrency.

#### Why This Structure Was Selected
1. **$O(1)$ Average Lookup Time**: Maps unique string identifiers (`accountNumber` like `"SAV-1001"`, `customerId` like `"CUST-101"`) directly to account pointers or customer entities in constant time using `std::hash<std::string>`.
2. **Decoupled Key Management**: Eliminates the need to maintain an integer-indexed contiguous array or continuously re-sort accounts after new registrations.

#### Average vs Worst-Case Complexity
- **Average Time Complexity**: $O(1)$ for `.find()`, `operator[]`, `.insert()`, and `.erase()`.
- **Worst-Case Time Complexity**: $O(N)$.
  - *Why Worst-Case is $O(N)$*: Hash tables map keys to integer bucket indices: `bucket = hash(key) % bucket_count`. If multiple keys hash to the identical bucket (hash collision), they are stored in a linked bucket chain. If a catastrophic hash collision occursâ€”such as a degenerate hash function or a deliberate Hash-Flooding Denial-of-Service (DoS) attack where an adversary crafts keys with identical hash valuesâ€”all $N$ elements end up in a single bucket. Searching that bucket degrades to an $O(N)$ linear linked-list traversal.
  - *Mitigation*: C++17 implementations use MurmurHash or CityHash derivatives and maintain a low default `max_load_factor` ($\le 1.0$). When `load_factor() > max_load_factor()`, the container automatically triggers a `rehash()`, expanding the bucket array and redistributing keys to preserve $O(1)$ performance.
- **Space Complexity**: $O(B + N)$, where $B$ is the number of hash buckets and $N$ is the number of stored entities.

#### Code Example from Codebase
```cpp
// From BankSystem.cpp: O(1) average account lookup
std::shared_ptr<Account> BankSystem::getAccount(const std::string& accountNumber) {
    auto it = m_accounts.find(accountNumber); // O(1) average lookup
    if (it != m_accounts.end()) {
        return it->second;
    }
    return nullptr;
}
```

---

### 2.3. `std::map` (Self-Balancing Red-Black Binary Search Tree)

#### Source Verification
- **Source Files**: `backend/core/services/TransactionAnalytics.h`, `backend/core/services/TransactionAnalytics.cpp`
- **Class / Members**:
  - `TransactionAnalytics::generateCategorySpendingSummary()`
  - `TransactionAnalytics::printCategoryReport()`

#### Banking Problem It Solves
Customers and bank managers require categorized expense breakdowns (e.g., "Business: Equipment", "Financing: Loan Disbursal", "Income: Payroll", "Living Expenses: Utilities") formatted alphabetically on account statements.

#### Why This Structure Was Selected
1. **Automatic Lexicographical Ordering**: `std::map` is implemented internally as a self-balancing Red-Black Binary Search Tree. Every key inserted is immediately placed in its strict sorted position ($O(\log K)$).
2. **Zero Secondary Sort Overhead**: Unlike `std::unordered_map` (which stores keys in pseudo-random bucket order and requires copying keys to a vector and calling `std::sort` for reporting), iterating through `std::map` performs an in-order tree traversal (`Left -> Root -> Right`) that yields sorted categories in $O(K)$ time without extra memory allocations.
3. **Strict Guaranteed Worst-Case Bounds**: Unlike hash tables whose worst case is $O(N)$, a Red-Black tree guarantees that the maximum height of the tree never exceeds $2 \log_2(K + 1)$. Therefore, every search, insertion, and update operation is guaranteed $O(\log K)$ in the worst case.

#### Algorithmic Complexity
- **Insertion / Key Update (`[]`)**: $O(\log K)$ guaranteed
- **Lookup (`find`)**: $O(\log K)$ guaranteed
- **In-Order Traversal**: $O(K)$ linear time across all categories
- **Space Complexity**: $O(K)$ tree nodes, where each node stores key, value, left pointer, right pointer, parent pointer, and 1-bit color flag.

#### Code Example from Codebase
```cpp
// From TransactionAnalytics.cpp: Automatic sorted category aggregation
std::map<std::string, double> TransactionAnalytics::generateCategorySpendingSummary(
    const std::vector<Transaction>& txns) {
    std::map<std::string, double> categoryTotals;
    for (const auto& t : txns) {
        std::string category = resolveCategory(t);
        categoryTotals[category] += t.getAmount(); // O(log K) tree insert/update
    }
    return categoryTotals; // Keys strictly sorted alphabetically
}
```

---

### 2.4. `std::set` (Balanced Binary Search Tree with Unique Keys)

#### Source Verification
- **Source Files**: `backend/core/services/TransactionAnalytics.h`, `backend/core/services/TransactionAnalytics.cpp`, `backend/core/services/TransactionGraph.h`, `backend/storage/FilePersistence.cpp`
- **Class / Members**:
  - `TransactionAnalytics::getUniqueTransactionCounterparties()`
  - `TransactionGraph::m_vertices`
  - `FilePersistence::saveBankStateAtomic()` (`savedTxnIds`)

#### Banking Problem It Solves
In transaction analysis and fraud investigation, an account may conduct thousands of transactions with the same merchants or accounts (e.g. 50 Amazon purchases, 30 Uber rides). An auditor needs an exact, deduplicated list of unique counterparties without duplicate entries. Similarly, network graph construction requires maintaining the unique set of active bank accounts.

#### Why This Structure Was Selected
1. **Mathematical Set Uniqueness**: `std::set` guarantees that every element is unique. Inserting an already existing element is silently rejected without corrupting data or requiring manual `std::find` pre-checks.
2. **$O(\log U)$ Insertion and Deduplication**: Verifying uniqueness and inserting a new counterparty takes $O(\log U)$ time in the Red-Black tree, compared to $O(U)$ for linear search in an unsorted array.
3. **Sorted Presentation**: Iterating over `std::set` automatically presents counterparties or account IDs in ascending sorted order.

#### Algorithmic Complexity
- **Insertion (`insert`)**: $O(\log U)$ where $U$ is the count of unique elements
- **Existence Check (`count` / `find`)**: $O(\log U)$
- **Traversal**: $O(U)$
- **Space Complexity**: $O(U)$ tree nodes

#### Code Example from Codebase
```cpp
// From TransactionAnalytics.cpp: Deduplicating transaction counterparties
std::set<std::string> TransactionAnalytics::getUniqueTransactionCounterparties(
    const std::vector<Transaction>& txns, const std::string& myAccount) {
    std::set<std::string> counterparties;
    for (const auto& t : txns) {
        if (!t.getToAccount().empty() && t.getToAccount() != myAccount) {
            counterparties.insert(t.getToAccount()); // O(log U) deduplication
        }
    }
    return counterparties;
}
```

---

### 2.5. `std::queue` (First-In, First-Out Container Adapter)

#### Source Verification
- **Source Files**: `backend/core/services/BankSystem.h`, `backend/core/services/BankSystem.cpp`
- **Class / Members**:
  - `BankSystem::m_pendingRequests` (`std::queue<BankingRequest>`)
  - `BankSystem::submitBankingRequest()`
  - `BankSystem::processNextBankingRequest()`
  - `BankSystem::displayPendingRequests()`

#### Banking Problem It Solves
Customers submit administrative requests asynchronously: loan applications, overdraft limit increases, and account unblock requests. Banking regulations and fairness policies require that customer requests be processed strictly in the chronological order of submission (First-In, First-Out), preventing prioritization bias or starvation.

#### Why This Structure Was Selected
1. **Enforced FIFO Semantics**: `std::queue` restricts access exclusively to `push()` at the back and `front()` / `pop()` at the front. It is impossible for an operator to service an arbitrary request out of turn.
2. **$O(1)$ Constant-Time Operations**: Enqueuing a new request via `push()` and dequeuing via `pop()` are guaranteed $O(1)$ operations, using `std::deque` as the underlying sequence container.
3. **Memory Stability**: Unlike vectors that may trigger expensive reallocation of all existing elements, the underlying deque allocates memory in fixed-size chunks, preventing large memory copies during continuous push/pop churn.

#### Algorithmic Complexity
- **Enqueue (`push`)**: $O(1)$
- **Inspect Front (`front`)**: $O(1)$
- **Dequeue (`pop`)**: $O(1)$
- **Space Complexity**: $O(R)$ where $R$ is the number of queued requests

#### Code Example from Codebase
```cpp
// From BankSystem.cpp: Enqueuing and processing requests in strict FIFO order
std::string BankSystem::submitBankingRequest(const std::string& customerId, 
                                             const std::string& accountNumber, 
                                             RequestType type, double amount, 
                                             const std::string& notes) {
    std::string reqId = "REQ-" + std::to_string(m_nextRequestId++);
    BankingRequest req{reqId, customerId, accountNumber, type, amount, RequestStatus::PENDING, notes};
    m_pendingRequests.push(req); // O(1) FIFO Enqueue
    return reqId;
}

bool BankSystem::processNextBankingRequest(bool approve, std::string& outMessage) {
    if (m_pendingRequests.empty()) return false;
    BankingRequest req = m_pendingRequests.front(); // O(1) Inspect oldest request
    m_pendingRequests.pop();                        // O(1) Dequeue
    // Execute business logic (loan disbursal / account unblock)...
    return true;
}
```

---

### 2.6. `std::stack` (Last-In, First-Out Container Adapter)

#### Source Verification
- **Source Files**: `backend/core/services/BankSystem.h`, `backend/core/services/BankSystem.cpp`
- **Class / Members**:
  - `BankSystem::m_reversalStack` (`std::stack<TransactionReversal>`)
  - `BankSystem::undoLastTransaction()`
  - `BankSystem::displayReversalStack()`

#### Banking Problem It Solves
Financial audit and transaction management engines require a safe transaction undo and compensation mechanism. If an operator or customer executes a sequence of dependent financial operationsâ€”such as:
1. `Step 1`: Deposit $300 to `SAV-1001`
2. `Step 2`: Withdraw $100 from `SAV-1001`
3. `Step 3`: Transfer $250 from `SAV-1001` to `CUR-1002`

The operations cannot be reversed in arbitrary order. Reversing `Step 1` first could cause the account balance to turn negative and trigger overdraft violations. Rollbacks must execute in strict reverse chronological order: `Step 3`, then `Step 2`, then `Step 1`.

#### Why This Structure Was Selected
1. **Enforced LIFO Semantics**: `std::stack` guarantees Last-In, First-Out execution. The most recent transaction pushed onto the stack is the first element accessed (`top()`) and removed (`pop()`).
2. **$O(1)$ Constant Time Reversal**: Pushing a compensation action upon transaction completion and popping it during rollback takes $O(1)$ time.
3. **Compensating Financial Reversals**: Instead of mutating or deleting historical database records (which violates banking auditability laws), the reversal engine pops the top action and executes an equal-and-opposite compensating financial entry (`DEPOSIT` reversal debits the account; `WITHDRAWAL` reversal re-credits the account; `TRANSFER` reversal recovers funds from recipient).

#### Algorithmic Complexity
- **Push Compensation (`push`)**: $O(1)$
- **Inspect Top (`top`)**: $O(1)$
- **Pop / Undo (`pop`)**: $O(1)$
- **Space Complexity**: $O(S)$ where $S$ is the stack depth of reversible operations

#### Code Example from Codebase
```cpp
// From BankSystem.cpp: LIFO Transaction rollback engine
bool BankSystem::undoLastTransaction(std::string& outMessage) {
    if (m_reversalStack.empty()) return false;

    // Pop the most recent transaction from LIFO stack
    TransactionReversal rev = m_reversalStack.top(); // O(1)
    m_reversalStack.pop();                           // O(1)

    // Execute compensating transaction
    if (rev.originalType == TransactionType::TRANSFER_OUT) {
        auto src = getAccount(rev.sourceAccount);
        auto dest = getAccount(rev.targetAccount);
        dest->withdraw(rev.amount, "Reversal: Transfer Refund");
        src->deposit(rev.amount, "Reversal: Transfer Recovered");
    }
    return true;
}
```

---

### 2.7. Binary Search (`std::lower_bound`)

#### Source Verification
- **Source Files**: `backend/core/services/TransactionAnalytics.h`, `backend/core/services/TransactionAnalytics.cpp`
- **Class / Members**:
  - `TransactionAnalytics::binarySearchById()`
  - `TransactionAnalytics::sortByTimestamp()`

#### Banking Problem It Solves
Enterprise banking ledgers store millions of transaction records. When an auditor or customer searches for a specific transaction ID or requests all transactions within a specific timestamp date range, scanning the ledger sequentially ($O(N)$) produces perceptible UI lag and excessive CPU cycles.

#### Why This Structure Was Selected
1. **$O(\log N)$ Divide-and-Conquer**: Because transactions are sorted (or can be sorted in $O(N \log N)$), binary search halves the search space with every comparison. On an audit ledger with $N = 1,000,000$ transactions, binary search requires at most $\lceil\log_2(1,000,000)\rceil \approx 20$ comparisons, whereas a linear scan averages $500,000$ comparisons.
2. **`std::lower_bound` Compatibility**: In C++, `std::lower_bound` on a `std::vector` uses random-access iterators to calculate midpoints in $O(1)$ time, achieving true theoretical $O(\log N)$ performance.

#### Theoretical vs Benchmark Clarification
- **Theoretical Time Complexity**: $O(\log N)$
- **Linear Scan Theoretical Complexity**: $O(N)$
- **Measured Benchmark Performance** (from `DSABenchmark::verifyBinarySearchVsLinear()` over 5,000 transactions across 1,000 repeated lookups):
  - Binary Search: **~13 comparisons** per search ($138.80\ \mu\text{s}$ total)
  - Linear Search: **~2,500 comparisons** per search ($40,683.00\ \mu\text{s}$ total)
  - Measured Benchmark Speedup: **~293x faster** on the tested dataset.
  - *Academic Distinction*: The ~293x speedup is a hardware/dataset benchmark measurement, **not** a Big-O complexity change. The algorithmic complexity remains strictly $O(\log N)$ vs $O(N)$.

#### Code Example from Codebase
```cpp
// From TransactionAnalytics.cpp: O(log N) Binary Search via std::lower_bound
const Transaction* TransactionAnalytics::binarySearchById(
    const std::vector<Transaction>& sortedTxns, const std::string& txnId) {
    auto it = std::lower_bound(sortedTxns.begin(), sortedTxns.end(), txnId, 
        [](const Transaction& t, const std::string& id) {
            return t.getTransactionId() < id;
        });

    if (it != sortedTxns.end() && it->getTransactionId() == txnId) {
        return &(*it); // Found in O(log N) comparisons
    }
    return nullptr;
}
```

---

### 2.8. Directed Graph (Adjacency List Representation)

#### Source Verification
- **Source Files**: `backend/core/services/TransactionGraph.h`, `backend/core/services/TransactionGraph.cpp`
- **Class / Members**:
  - `TransactionGraph::m_adjList` (`std::unordered_map<std::string, std::vector<TransferEdge>>`)
  - `TransactionGraph::m_vertices` (`std::set<std::string>`)
  - `TransactionGraph::addEdge()`
  - `TransactionGraph::populateFromLedger()`

#### Banking Problem It Solves
Inter-account fund transfers do not exist in isolation; they form a complex transaction network. Financial regulators require banks to map capital flows between accounts to detect money mule accounts, fraud rings, and illicit financial networks.

#### Mathematical Formulation & Mapping
- **Vertex ($V$)**: Represents an individual bank account (e.g. `"ACC-1001"`).
- **Directed Edge ($E$)**: Represents a directional fund transfer from a source account to a destination account ($u \to v$).
- **Edge Information / Weight (`TransferEdge`)**: Contains the transferred monetary amount (`amount`), transaction ID (`txnId`), and timestamp.
- **Graph Representation**: An **Adjacency List** using `std::unordered_map<std::string, std::vector<TransferEdge>>`.

#### Why Adjacency List over Adjacency Matrix?
1. **Sparsity of Financial Networks**: In a banking system with $V = 100,000$ accounts, the total possible account pairings is $V^2 = 10,000,000,000$. However, any single account transacts with only a handful of other accounts ($E \ll V^2$).
2. **Space Efficiency**: An adjacency matrix requires $O(V^2)$ memory ($~40\text{ GB}$ of RAM for 100k accounts), which is completely impractical. An adjacency list consumes only $O(V + E)$ space, allocating memory strictly for active accounts and actual transactions.
3. **Fast Neighbor Traversal**: Iterating over an account's outgoing transfers takes $O(\text{deg}(u))$ time instead of scanning an entire row of $V$ entries.

#### Algorithmic Complexity
- **Add Edge**: $O(1)$ amortized
- **Lookup Outgoing Edges of Vertex $u$**: $O(1)$ average hash table lookup
- **Graph Construction from Ledger**: $O(E)$ linear time over transfer transactions
- **Space Complexity**: $O(V + E)$

#### Code Example from Codebase
```cpp
// From TransactionGraph.cpp: Building directed adjacency list edges
void TransactionGraph::addEdge(const std::string& fromAcc, 
                               const std::string& toAcc, 
                               double amount, 
                               const std::string& txnId, 
                               const std::string& timestamp) {
    if (fromAcc.empty() || toAcc.empty()) return;
    m_adjList[fromAcc].push_back(TransferEdge{toAcc, amount, txnId, timestamp});
    m_vertices.insert(fromAcc);
    m_vertices.insert(toAcc);
}
```

---

### 2.9. Depth-First Search (DFS) Cycle Detection

#### Source Verification
- **Source Files**: `backend/core/services/TransactionGraph.h`, `backend/core/services/TransactionGraph.cpp`
- **Class / Members**:
  - `TransactionGraph::findCircularRoutingRings()`
  - `TransactionGraph::dfsCycleHelper()`

#### Banking Problem It Solves (Forensic AML)
In **Anti-Money Laundering (AML)** investigations, criminals attempt to disguise the origin of illicit capital through **wash trading**, **layering**, and **carousel fraud**. Funds are transferred through a chain of seemingly unrelated intermediary accounts and eventually funneled back to the originator:
$$\text{Account } A \xrightarrow{\$10,000} \text{Account } B \xrightarrow{\$9,900} \text{Account } C \xrightarrow{\$9,800} \text{Account } D \xrightarrow{\$9,700} \text{Account } A$$
This closed circular routing ring is a primary indicator of money laundering.

#### How the Algorithm Works
The engine uses **Depth-First Search (DFS)** with explicit recursion-stack state tracking (the 3-color graph traversal model):
1. **`visited` State Map**: Tracks whether a vertex has been examined during the global traversal to prevent redundant exploration across disconnected subgraphs.
2. **`inStack` State Map (Recursion Call Stack)**: Tracks whether a vertex is currently in the active DFS exploration path.
   - When descending into an adjacent vertex $v$, `inStack[v] = true`.
   - When backtracking out of vertex $v$, `inStack[v] = false`.
3. **Cycle Condition**: If DFS traverses an edge from $u$ to neighbor $v$, and $v == \text{start}$ (or $v$ is currently marked `inStack[v] == true`), a **back-edge** in a directed graph has been identified, proving the existence of a closed directed cycle.
4. **Depth Bounding (`maxCycleLength`)**: The helper prunes branches where path length exceeds a configurable threshold (e.g. 5 hops), targeting actionable laundering rings while preventing exponential path explosions.

#### Why DFS over BFS for Cycle Detection?
- **Path Reconstruction**: DFS naturally maintains the current exploration path in its call stack (or explicit `path` vector). When a cycle is detected, the entire circular path ($A \to B \to C \to A$) is immediately available in the current recursion frame.
- **Back-Edge Detection**: In directed graphs, cycle detection requires detecting **back-edges** to active ancestors on the recursion stack. Breadth-First Search (BFS) explores radially by levels and cannot differentiate between cross-edges, forward-edges, and back-edges without complex ancestor tracking.

#### Algorithmic Complexity
- **Time Complexity**: $O(V + E)$ per source component traversal (bounded by `maxCycleLength`).
- **Space Complexity**: $O(V)$ auxiliary space for `visited`, `inStack`, and recursion stack depth.

#### Code Example from Codebase
```cpp
// From TransactionGraph.cpp: DFS Cycle Detection with recursion stack tracking
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
                // Closed cycle detected! Loop back to start found.
                std::vector<std::string> cycle = path;
                cycle.push_back(start);
                detectedCycles.push_back(cycle);
            } else if (!visited[neighbor]) {
                dfsCycleHelper(neighbor, start, path, visited, inStack, 
                               detectedCycles, maxCycleLength);
            }
        }
    }

    // Backtrack: Remove from current exploration path
    path.pop_back();
    inStack[current] = false;
}
```

---

## 3. Complexity Rigor: Theoretical Complexity vs Benchmark Performance

In academic examination and software engineering defenses, students must never confuse **empirical benchmark results** with **asymptotic computational complexity**:

| Algorithm / Operation | Theoretical Asymptotic Complexity | Measured Project Benchmark (Hardware Run) | Technical Meaning & Academic Distinction |
| :--- | :---: | :---: | :--- |
| **Account Lookup** (`unordered_map` vs `vector`) | $O(1)\text{ avg}$ vs $O(N)$ | **~430x faster** on 10,000 accounts | The 430x speedup is an empirical measurement on a specific CPU. The Big-O complexity remains $O(1)$ vs $O(N)$ regardless of hardware. |
| **Transaction Search** (Binary Search vs Linear) | $O(\log N)$ vs $O(N)$ | **~293x faster** on 5,000 items | Binary search requires $\approx 13$ comparisons vs $\approx 2,500$ linear comparisons. Big-O is fundamentally $O(\log N)$. |
| **Vector Allocation** (`reserve()` vs dynamic growth) | $O(N)$ vs $O(N)$ amortized | **~2.1x faster** insertion | Both approaches have $O(N)$ total time complexity, but `reserve()` eliminates intermediate heap reallocations and copies, reducing the constant factor $c$. |
| **AML Cycle Detection** (DFS Traversal) | $O(V + E)$ | **&lt; 1 ms** on test networks | Traverses each vertex and edge at most once within bounded hop limits. |

---

## 4. Common Viva Questions & Model Answers

### Question 1: Why did you use `std::unordered_map` instead of `std::map` for account and customer lookup?
> **Answer**: Account and customer lookups occur on every single banking operation (deposits, card withdrawals, balance inquiries). `std::unordered_map` uses a hash table with bucket chaining, offering $O(1)$ average time complexity. In contrast, `std::map` is a self-balancing Red-Black tree with $O(\log N)$ lookup time. For 100,000 accounts, $O(1)$ requires 1 hash lookup, whereas $O(\log_2 100,000) \approx 17$ pointer dereferences and string comparisons. We do not need accounts sorted by ID during routine lookups, making `unordered_map` optimal.

---

### Question 2: Why did you use `std::map` for category analytics if `std::unordered_map` has faster $O(1)$ average lookup?
> **Answer**: `std::map` maintains its keys strictly in sorted order via an internal Red-Black Binary Search Tree. For financial category reports, statements must display categories in alphabetical order (e.g., "ATM", "Groceries", "Salary"). If we used `unordered_map`, keys would be stored in arbitrary bucket order, requiring us to allocate an auxiliary vector, copy all keys, and run `std::sort` ($O(K \log K)$). With `std::map`, in-order traversal yields the sorted statement automatically with zero extra sorting overhead.

---

### Question 3: Why is a `std::queue` suitable for pending administrative requests?
> **Answer**: `std::queue` enforces First-In, First-Out (FIFO) processing. In banking, fairness and compliance mandate that loan applications, overdraft increases, and account unblock requests be evaluated strictly in the chronological order of submission. A queue guarantees that no customer's request can be prioritized or starved out of turn, and provides $O(1)$ enqueue (`push`) and dequeue (`pop`) operations.

---

### Question 4: Why is a `std::stack` suitable for the transaction undo / reversal engine?
> **Answer**: `std::stack` enforces Last-In, First-Out (LIFO) semantics. In financial accounting, multi-step transactions can create cascading balance dependencies. To safely unwind a sequence of operations without violating intermediate balance constraints (e.g. triggering overdraft errors), the most recently executed transaction must be reversed first. Compensating transactions are popped from the top of the stack in $O(1)$ time, reversing the latest state change before touching earlier ones.

---

### Question 5: Why did you use a Graph to represent transactions instead of a standard relational table?
> **Answer**: Relational tables excel at tabular storage, but cannot efficiently model multi-hop relationship topologies. When analyzing capital flows across accounts, accounts naturally map to vertices ($V$) and fund transfers map to directed edges ($E = u \to v$) with weights representing amounts. Representing this as an adjacency list allows us to perform graph traversal algorithms - such as cycle detection, shortest-path fund tracing, and hub-and-spoke mule detection - in $O(V + E)$ time, which would require expensive recursive SQL joins.

---

### Question 6: Why did you use Depth-First Search (DFS) instead of Breadth-First Search (BFS) for AML cycle detection?
> **Answer**: In directed graphs, detecting cycles requires identifying **back-edges** to ancestor nodes currently in the active exploration path. DFS naturally tracks the current traversal branch in its recursion call stack (or explicit `inStack` map). When a back-edge is encountered, the entire cycle path ($A \to B \to C \to A$) is immediately intact in the call stack for instant retrieval. BFS explores radially by level and cannot isolate directed cycles without cumbersome parent-pointer tracking, making DFS the mathematically superior algorithm.

---

### Question 7: What is the difference between $O(1)$ average and $O(N)$ worst-case for `std::unordered_map`?
> **Answer**: In the average case, a good hash function distributes $N$ keys uniformly across $B$ buckets. When the load factor $N/B \le 1.0$, each bucket contains $\approx 1$ element, yielding $O(1)$ direct lookup. However, in the worst case where all keys produce the identical hash value (hash collisions), all $N$ elements fall into the exact same bucket linked list. Searching that bucket degrades to an $O(N)$ linear traversal. In production, this is guarded against by Rehashing when `load_factor() > 1.0` and using robust hash functions.

---

### Question 8: Why is Binary Search $O(\log N)$?
> **Answer**: Binary search operates on a sorted random-access array. At each iteration, the algorithm compares the target element with the middle element, immediately eliminating half of the remaining search space. Starting with $N$ elements, after 1 step we have $N/2$, then $N/4$, down to $N/2^k = 1$. Solving for $k$:
$$2^k = N \implies k = \log_2 N$$
Therefore, the maximum number of comparisons is strictly bounded by $\lceil\log_2 N\rceil$, giving a time complexity of $O(\log N)$.

---

### Question 9: What are $V$ and $E$ in your transaction graph?
> **Answer**: 
- **$V$ (Vertices)**: The set of unique bank accounts registered in the banking network (e.g. `"ACC-1001"`, `"CUR-2004"`).
- **$E$ (Edges)**: The directed fund transfers between accounts. Each edge $e = (u, v)$ points from the sender account $u$ to the recipient account $v$, carrying the monetary transfer amount, transaction ID, and timestamp as edge attributes.

---

### Question 10: What happens if the transaction graph contains a directed cycle?
> **Answer**: In banking compliance, a directed cycle ($A \to B \to C \to A$) indicates that funds originated from Account $A$ and returned back to Account $A$ through intermediary accounts. This pattern is characteristic of **wash trading**, **circular layering**, or **carousel fraud**, where actors attempt to artificially inflate transaction volumes or disguise the illicit origin of dirty money. When `findCircularRoutingRings()` detects a cycle, the system flags the accounts involved, displays the circular loop on the Admin AML Network visualizer, and logs a high-priority forensic audit alert.