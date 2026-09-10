# Smart Banking Management System — Architecture Diagrams

This document contains the official architectural diagrams for the **Smart Banking Management System**, designed for project documentation, technical reports, and college viva examinations.

---

## 1. Main System Architecture

The system is organized into a 5-tier architecture connecting the zero-dependency React frontend to the pure C++17 core engine, native Winsock2 networking layer, and persistent storage abstractions.

```mermaid
graph TD
    subgraph PresentationTier["Client Presentation Layer (Browser)"]
        UI["React 18 Single-Page Application (SPA)<br/>(Babel Standalone / Zero npm/Node Dependency)"]
        CP["Customer Portal<br/>(Accounts, Transfers, Health)"]
        AP["Admin Dashboard<br/>(Queue, Undo, AML Visualizer)"]
        UI --- CP
        UI --- AP
    end

    subgraph NetworkTier["Network & Transport Layer"]
        HTTP["HTTP/1.1 REST Protocol (TCP Port 8080)<br/>CORS Preflight (204) | JSON Payloads"]
    end

    subgraph ServerTier["C++17 Embedded Networking Layer"]
        WINSOCK["Winsock2 HTTP Server (HttpServer)<br/>Socket Lifecycle | Non-Blocking TCP Stream"]
        PARSER["HTTP Parser & JSON Serializer (JsonHelper)"]
        AUTH_MW["Centralized Admin Authorization Middleware<br/>(X-Admin-Key Inspection)"]
        STATIC_SRV["Static Asset Delivery Service<br/>(index.html, style.css, api.js, App.js)"]
        WINSOCK --> PARSER
        PARSER --> AUTH_MW
        PARSER --> STATIC_SRV
    end

    subgraph EngineTier["C++ Core Banking Engine (Domain & Services)"]
        BS["BankSystem (Facade Coordinator)"]
        CM["CustomerManager (O(1) Customer Registry)"]
        ACC["Account Hierarchy (Polymorphic ABC)<br/>SavingsAccount | CurrentAccount"]
        SEC["SecurityHelper (PBKDF2-HMAC-SHA256, CSPRNG)"]
        ANALYTICS["TransactionAnalytics (Red-Black Tree STL map)"]
        AUDITOR["SmartAuditor (Financial Health Advisory)"]
        GRAPH["TransactionGraph (Directed Adjacency List)"]
        
        BS --> CM
        BS --> ACC
        BS --> SEC
        BS --> ANALYTICS
        BS --> AUDITOR
        BS --> GRAPH
    end

    subgraph StorageTier["Persistence & Relational Database Layer"]
        FP["FilePersistence (ACID Atomic Swaps)<br/>Staging (.tmp) | Backup (.bak) | State (.txt)"]
        DB["DatabaseManager (MySQL 3NF Engine)<br/>DDL Schema (schema.sql) | Dump (dump.sql)"]
        AML_ENG["Forensic AML Detection Engine<br/>DFS Cycle Detection (findCircularRoutingRings)"]
    end

    PresentationTier ==>|HTTP GET / POST / OPTIONS| NetworkTier
    NetworkTier ==>|Winsock TCP Streams| ServerTier
    AUTH_MW ==>|Direct C++ Method Calls / DTOs| EngineTier
    EngineTier ==>|Atomic State Serialization| FP
    EngineTier ==>|Relational 3NF Export| DB
    GRAPH ==>|Recursive Cycle Detection| AML_ENG
```

---

## 2. Authentication & Authorization Flow

The system strictly decouples public customer authentication from guarded administrative operations:

```mermaid
sequenceDiagram
    autonumber
    actor Customer as Retail Customer
    actor Admin as System Administrator
    participant Frontend as React 18 Web UI
    participant Server as C++ Winsock Server (HttpServer)
    participant AuthMW as Admin Middleware
    participant Bank as Core Banking Engine (BankSystem)
    participant Sec as SecurityHelper (PBKDF2)
    participant Storage as File Persistence / Storage

    %% CUSTOMER AUTHENTICATION FLOW
    Note over Customer, Storage: 1. Customer Authentication Flow (Public Endpoint)
    Customer->>Frontend: Enters Customer ID & 4-Digit PIN
    Frontend->>Server: POST /api/auth/login {"customerId": "CUST-101", "pin": "1234"}
    Server->>Bank: authenticateCustomer(AuthRequestDTO)
    Bank->>Storage: Retrieve Stored Salted PIN Hash
    Storage-->>Bank: "$pbkdf2-sha256$10000$<salt>$<hash>"
    Bank->>Sec: verifyPin(candidatePin, storedHash)
    Note over Sec: Extracts 16-byte salt & 10,000 iterations<br/>Computes PBKDF2-HMAC-SHA256<br/>constantTimeEquals() byte comparison
    alt Valid PIN
        Sec-->>Bank: true
        Bank-->>Server: AuthResponseDTO (success=true, sessionToken)
        Server-->>Frontend: HTTP 200 OK {"success": true, "token": "tok_..."}
        Frontend->>Customer: Unlocks Customer Dashboard (Scoped to CUST-101)
    else Invalid PIN
        Sec-->>Bank: false
        Bank-->>Server: Throws AuthenticationException
        Server-->>Frontend: HTTP 401 Unauthorized {"error": "UNAUTHORIZED"}
        Frontend->>Customer: Displays "Invalid Credentials" Toast
    end

    %% ADMIN AUTHORIZATION FLOW
    Note over Admin, Storage: 2. Administrative Operations Flow (Guarded by X-Admin-Key)
    Admin->>Frontend: Initiates Admin Action (e.g. Rollback / Undo)
    Frontend->>Server: POST /api/admin/undo (Header: X-Admin-Key: admin123)
    Server->>AuthMW: Intercept /api/admin/* Request
    Note over AuthMW: Validates Header: req.getHeader("X-Admin-Key")
    alt Valid Admin Key
        AuthMW->>Bank: undoLastTransaction(outMessage)
        Bank->>Bank: Pop LIFO m_reversalStack (Compensate Balance)
        Bank->>Storage: Atomic Flush (saveBankStateAtomic)
        Bank-->>Server: Rollback Successful
        Server-->>Frontend: HTTP 200 OK {"message": "Rolled Back TXN-XXXXXX"}
        Frontend->>Admin: Restores Balance & Shows Success Toast
    else Missing or Invalid Admin Key
        AuthMW-->>Server: Reject Request
        Server-->>Frontend: HTTP 401 Unauthorized {"error": "Missing/Invalid X-Admin-Key"}
        Frontend->>Admin: Displays "Access Denied: Admin Key Invalid"
    end
```

---

## 3. Data Structures & Algorithms (DSA) Architecture

Every major data structure in the standard C++ library and domain models is mapped to a specific banking requirement:

```mermaid
graph LR
    subgraph Operations["Banking Functional Requirement"]
        OP1["Transaction History & Ledger"]
        OP2["Customer & Account Directory Lookup"]
        OP3["Spending Category Breakdown"]
        OP4["Counterparty Deduplication"]
        OP5["Pending Loan / Unblock Requests"]
        OP6["Multi-Step Transaction Rollback (Undo)"]
        OP7["Chronological Date Search"]
        OP8["Inter-Account Fund Flow Modeling"]
        OP9["AML Forensic Wash Trading Detection"]
    end

    subgraph DSA["Chosen Data Structure & Algorithm"]
        DS1["std::vector&lt;Transaction&gt;<br/><b>Dynamic Array</b>"]
        DS2["std::unordered_map&lt;string, shared_ptr&lt;Account&gt;&gt;<br/><b>Hash Table with Chaining</b>"]
        DS3["std::map&lt;string, double&gt;<br/><b>Self-Balancing Red-Black Tree</b>"]
        DS4["std::set&lt;string&gt;<br/><b>Balanced Binary Search Tree</b>"]
        DS5["std::queue&lt;BankingRequest&gt;<br/><b>FIFO Pipeline</b>"]
        DS6["std::stack&lt;TransactionReversal&gt;<br/><b>LIFO Compensation Stack</b>"]
        DS7["std::lower_bound<br/><b>Binary Search O(log N)</b>"]
        DS8["TransactionGraph<br/><b>Directed Adjacency List</b>"]
        DS9["Depth-First Search (DFS)<br/><b>Recursive Three-Color Cycle Detection</b>"]
    end

    subgraph Complexity["Algorithmic Complexity & Role"]
        C1["<b>O(1) Amortized Append</b><br/>Cache-friendly contiguous memory"]
        C2["<b>O(1) Average Lookup</b><br/>Direct primary key hashing"]
        C3["<b>O(log N) Search / Insert</b><br/>Strict alphabetical in-order sorting"]
        C4["<b>O(log N) Insert</b><br/>Automatic duplicate rejection"]
        C5["<b>O(1) Push / Pop</b><br/>First-In, First-Out request fairness"]
        C6["<b>O(1) Push / Pop</b><br/>Last-In, First-Out reverse compensation"]
        C7["<b>O(log N) Comparisons</b><br/>~265x-375x speedup over linear scan"]
        C8["<b>O(V + E) Space</b><br/>V = Accounts, E = Fund Transfers"]
        C9["<b>O(V + E) Time</b><br/>Identifies circular cycles (A &rarr; B &rarr; C &rarr; A)"]
    end

    OP1 --> DS1 --> C1
    OP2 --> DS2 --> C2
    OP3 --> DS3 --> C3
    OP4 --> DS4 --> C4
    OP5 --> DS5 --> C5
    OP6 --> DS6 --> C6
    OP7 --> DS7 --> C7
    OP8 --> DS8 --> C8
    OP9 --> DS9 --> C9
```

---

## 4. Relational Database Architecture (MySQL 3NF)

The system enforces relational database integrity matching standard **Third Normal Form (3NF)** rules:

```mermaid
erDiagram
    CUSTOMERS ||--o{ ACCOUNTS : "owns (1:N)"
    CUSTOMERS ||--o{ PENDING_REQUESTS : "submits (1:N)"
    ACCOUNTS ||--o{ TRANSACTIONS : "source account (1:N)"
    ACCOUNTS ||--o{ TRANSACTIONS : "destination account (1:N)"

    CUSTOMERS {
        varchar(32) customer_id PK "Primary Key"
        varchar(128) full_name "Full Name"
        varchar(128) email "Email Address"
        varchar(32) phone "Phone Number"
        varchar(256) pin "PBKDF2 Salted Hash (Zero Plaintext)"
        timestamp created_at "Registration Timestamp"
    }

    ACCOUNTS {
        varchar(32) account_number PK "Primary Key"
        varchar(32) customer_id FK "Foreign Key -> customers(customer_id) ON DELETE CASCADE"
        varchar(32) account_type "Savings Account / Current Account"
        varchar(128) holder_name "Account Holder Name"
        decimal(15_2) balance "Current Balance"
        boolean is_active "Active / Blocked Status Flag"
        decimal(5_2) interest_rate "Annual Interest Rate (Savings)"
        decimal(15_2) minimum_balance "Mandatory Minimum Balance (Savings)"
        decimal(15_2) overdraft_limit "Authorized Overdraft (Current)"
        timestamp created_at "Creation Timestamp"
    }

    TRANSACTIONS {
        varchar(32) transaction_id PK "Primary Key (TXN-XXXXXX)"
        varchar(32) from_account FK "Source Account No / N/A"
        varchar(32) to_account FK "Destination Account No / N/A"
        decimal(15_2) amount "Transaction Amount"
        timestamp timestamp "Execution Timestamp"
        int type_id "DEPOSIT, WITHDRAWAL, TRANSFER"
        varchar(32) type_name "Readable Transaction Type"
        int status_id "SUCCESS, FAILED, PENDING"
        varchar(32) status_name "Transaction Status"
        text description "Audit Memo / Reason"
    }

    PENDING_REQUESTS {
        varchar(32) request_id PK "Primary Key (REQ-XXXX)"
        varchar(32) customer_id FK "Foreign Key -> customers(customer_id)"
        varchar(32) account_number "Target Account Number"
        varchar(32) type "LOAN_APPLICATION / ACCOUNT_UNBLOCK"
        decimal(15_2) amount "Requested Amount"
        varchar(32) status "PENDING, APPROVED, REJECTED"
        text notes "Customer / Officer Notes"
        timestamp created_at "Request Submission Timestamp"
    }

    AUDIT_LOGS {
        int log_id PK "Primary Key (AUTO_INCREMENT)"
        varchar(64) event_type "System Audit Event"
        text details "Structured Event Payload"
        timestamp logged_at "System Event Timestamp"
    }
```