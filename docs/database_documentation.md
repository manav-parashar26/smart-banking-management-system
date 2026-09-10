# Smart Banking Management System - Database Architecture & 3NF Normalization Documentation

## Executive Summary

The **Smart Banking Management System** implements a dual persistence architecture:
1. **Relational MySQL 8.0+ / InnoDB Schema (3NF)**: Designed following the principles of relational database design, Third Normal Form (3NF) normalization, entity-relationship integrity, foreign key constraints (`ON DELETE CASCADE`), B-tree indexes, and automated SQL dump synchronization (`DatabaseManager.cpp`).
2. **Resilient Local File Persistence Engine**: A zero-dependency file-based storage layer (`FilePersistence.cpp`) utilizing write-to-temporary staging (`.tmp`), atomic file renaming, and disaster recovery backup mirrors (`.bak`) for standalone offline operation.

All schemas, constraints, and persistence routines documented herein are verified directly against the production codebase: [`backend/database/MySqlSchema.h`](../backend/database/MySqlSchema.h), [`backend/database/DatabaseManager.cpp`](../backend/database/DatabaseManager.cpp), [`data/schema.sql`](../data/schema.sql), and [`data/banking_system_dump.sql`](../data/banking_system_dump.sql).

---

## 1. Relational Database Schema & Table Specifications

The relational schema is partitioned into five distinct tables to isolate entities, accounts, financial ledgers, administrative queues, and security audit events.

```
+-----------------------------------------------------------------------------------------------+
|                                      MYSQL DATABASE: smart_banking                            |
|                                       Storage Engine: InnoDB (ACID)                           |
+---------------------+-------------------+---------------------+------------------+------------+
| 1. customers        | 2. accounts       | 3. transactions     | 4. pending_reqs  | 5. audit   |
| (Customer Identity) | (Banking Balance) | (Immutable Ledger)  | (Admin Pipeline) | (Security) |
+---------------------+-------------------+---------------------+------------------+------------+
```

### 1.1. Table: `customers`
* **Purpose**: Primary customer identity registry storing contact info and salted cryptographic authentication credentials.
* **Storage Engine**: `InnoDB`, `DEFAULT CHARSET=utf8mb4`
* **Entity Integrity**: Enforced by Primary Key `pk_customers (customer_id)`

| Column Name | Data Type | Nullable | Default | Description & Constraint |
| :--- | :--- | :---: | :---: | :--- |
| `customer_id` | `VARCHAR(32)` | **NO** | *None* | **Primary Key**: Unique customer identifier (e.g. `"CUST-101"`). |
| `full_name` | `VARCHAR(128)` | **NO** | *None* | Legal customer name (e.g. `"Alice Vance"`). |
| `email` | `VARCHAR(128)` | **NO** | *None* | Unique primary email address for communication and alerts. |
| `phone` | `VARCHAR(32)` | **NO** | *None* | Verified phone contact (e.g. `"+1-555-1010"`). |
| `pin` | `VARCHAR(256)` | **NO** | *None* | **Cryptographic Hash**: Salted PBKDF2-HMAC-SHA256 string (zero plaintext). |
| `created_at` | `TIMESTAMP` | **NO** | `CURRENT_TIMESTAMP` | System registration timestamp. |

```sql
CREATE TABLE IF NOT EXISTS customers (
    customer_id     VARCHAR(32)     NOT NULL,
    full_name       VARCHAR(128)    NOT NULL,
    email           VARCHAR(128)    NOT NULL,
    phone           VARCHAR(32)     NOT NULL,
    pin             VARCHAR(256)    NOT NULL,
    created_at      TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT pk_customers PRIMARY KEY (customer_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

### 1.2. Table: `accounts`
* **Purpose**: Financial accounts held by customers, tracking balances, account types, interest rates, and overdraft parameters.
* **Referential Integrity**: Enforced via Foreign Key `fk_acc_customer` linked to `customers(customer_id)`.

| Column Name | Data Type | Nullable | Default | Description & Constraint |
| :--- | :--- | :---: | :---: | :--- |
| `account_number` | `VARCHAR(32)` | **NO** | *None* | **Primary Key**: Unique account number (e.g. `"SAV-1001"`, `"CUR-1002"`). |
| `customer_id` | `VARCHAR(32)` | **NO** | *None* | **Foreign Key**: References `customers(customer_id)`. |
| `account_type` | `VARCHAR(32)` | **NO** | *None* | Polymorphic type: `"Savings Account"` or `"Current Account"`. |
| `holder_name` | `VARCHAR(128)` | **NO** | *None* | Primary account holder display name. |
| `balance` | `DECIMAL(15, 2)` | **NO** | `0.00` | Current ledger balance in base currency ($). |
| `is_active` | `BOOLEAN` | **NO** | `TRUE` | Account operational status (`1` = Active, `0` = Frozen/Blocked). |
| `interest_rate` | `DECIMAL(5, 2)` | YES | `0.00` | Annual percentage yield for savings accounts (e.g. `4.00%`). |
| `minimum_balance`| `DECIMAL(15, 2)` | YES | `0.00` | Mandatory threshold for savings accounts (e.g. `$500.00`). |
| `overdraft_limit`| `DECIMAL(15, 2)` | YES | `0.00` | Authorized negative balance limit for current accounts (e.g. `$1000.00`). |
| `created_at` | `TIMESTAMP` | **NO** | `CURRENT_TIMESTAMP` | Account creation timestamp. |

```sql
CREATE TABLE IF NOT EXISTS accounts (
    account_number  VARCHAR(32)     NOT NULL,
    customer_id     VARCHAR(32)     NOT NULL,
    account_type    VARCHAR(32)     NOT NULL,
    holder_name     VARCHAR(128)    NOT NULL,
    balance         DECIMAL(15, 2)  NOT NULL DEFAULT 0.00,
    is_active       BOOLEAN         NOT NULL DEFAULT TRUE,
    interest_rate   DECIMAL(5, 2)   DEFAULT 0.00,
    minimum_balance DECIMAL(15, 2)  DEFAULT 0.00,
    overdraft_limit DECIMAL(15, 2)  DEFAULT 0.00,
    created_at      TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT pk_accounts PRIMARY KEY (account_number),
    CONSTRAINT fk_acc_customer FOREIGN KEY (customer_id) 
        REFERENCES customers(customer_id) 
        ON DELETE CASCADE 
        ON UPDATE CASCADE,
    INDEX idx_acc_customer (customer_id),
    INDEX idx_acc_balance (balance)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

### 1.3. Table: `transactions`
* **Purpose**: Double-entry financial audit ledger recording all monetary debits, credits, transfers, and status outcomes.
* **Auditability**: Records are strictly immutable and append-only.

| Column Name | Data Type | Nullable | Default | Description & Constraint |
| :--- | :--- | :---: | :---: | :--- |
| `transaction_id` | `VARCHAR(32)` | **NO** | *None* | **Primary Key**: Unique transaction identifier (e.g. `"TXN-100001"`). |
| `from_account` | `VARCHAR(32)` | **NO** | *None* | Source account ID, or `"N/A"` for direct external cash deposits. |
| `to_account` | `VARCHAR(32)` | **NO** | *None* | Destination account ID, or `"N/A"` for external cash withdrawals. |
| `amount` | `DECIMAL(15, 2)` | **NO** | *None* | Transferred monetary amount (strictly positive $> 0$). |
| `timestamp` | `VARCHAR(32)` | **NO** | *None* | Chronological ISO timestamp string (e.g. `"2026-09-10 12:45:00"`). |
| `type_id` | `INT` | **NO** | *None* | Integer enum code: `0` (DEPOSIT), `1` (WITHDRAWAL), `2` (TRANSFER_OUT), `3` (TRANSFER_IN). |
| `type_name` | `VARCHAR(32)` | **NO** | *None* | Human-readable string representation of transaction type. |
| `status_id` | `INT` | **NO** | *None* | Integer enum code: `0` (SUCCESS), `1` (FAILED), `2` (PENDING). |
| `status_name` | `VARCHAR(32)` | **NO** | *None* | Human-readable status (`"SUCCESS"`, `"FAILED"`). |
| `description` | `VARCHAR(255)` | YES | `''` | Purpose, memo, merchant name, or reason for failure. |

```sql
CREATE TABLE IF NOT EXISTS transactions (
    transaction_id  VARCHAR(32)     NOT NULL,
    from_account    VARCHAR(32)     NOT NULL,
    to_account      VARCHAR(32)     NOT NULL,
    amount          DECIMAL(15, 2)  NOT NULL,
    timestamp       VARCHAR(32)     NOT NULL,
    type_id         INT             NOT NULL,
    type_name       VARCHAR(32)     NOT NULL,
    status_id       INT             NOT NULL,
    status_name     VARCHAR(32)     NOT NULL,
    description     VARCHAR(255)    DEFAULT '',
    CONSTRAINT pk_transactions PRIMARY KEY (transaction_id),
    INDEX idx_txn_from (from_account),
    INDEX idx_txn_to (to_account),
    INDEX idx_txn_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

### 1.4. Table: `pending_requests`
* **Purpose**: Back-office administrative approval pipeline storing asynchronous customer requests (loan applications, account unblock requests).
* **Referential Integrity**: Linked to `customers(customer_id)` via `fk_req_customer`.

| Column Name | Data Type | Nullable | Default | Description & Constraint |
| :--- | :--- | :---: | :---: | :--- |
| `request_id` | `VARCHAR(32)` | **NO** | *None* | **Primary Key**: Unique request tracking ID (e.g. `"REQ-1001"`). |
| `customer_id` | `VARCHAR(32)` | **NO** | *None* | **Foreign Key**: Submitting customer ID referencing `customers`. |
| `account_number` | `VARCHAR(32)` | **NO** | *None* | Associated target account number for the request. |
| `type` | `VARCHAR(32)` | **NO** | *None* | Request category: `"LOAN_APPLICATION"` or `"ACCOUNT_UNBLOCK"`. |
| `amount` | `DECIMAL(15, 2)` | **NO** | `0.00` | Principal amount requested (for loans; `0.00` for unblocks). |
| `status` | `VARCHAR(32)` | **NO** | `'PENDING'` | Lifecycle state: `'PENDING'`, `'APPROVED'`, or `'REJECTED'`. |
| `notes` | `TEXT` | YES | `NULL` | Customer justification remarks and officer audit notes. |
| `created_at` | `TIMESTAMP` | **NO** | `CURRENT_TIMESTAMP` | Submission timestamp for strict FIFO order processing. |

```sql
CREATE TABLE IF NOT EXISTS pending_requests (
    request_id      VARCHAR(32)     NOT NULL,
    customer_id     VARCHAR(32)     NOT NULL,
    account_number  VARCHAR(32)     NOT NULL,
    type            VARCHAR(32)     NOT NULL,
    amount          DECIMAL(15, 2)  NOT NULL DEFAULT 0.00,
    status          VARCHAR(32)     NOT NULL DEFAULT 'PENDING',
    notes           TEXT,
    created_at      TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT pk_requests PRIMARY KEY (request_id),
    CONSTRAINT fk_req_customer FOREIGN KEY (customer_id) 
        REFERENCES customers(customer_id) 
        ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

### 1.5. Table: `audit_logs`
* **Purpose**: Independent administrative and security audit trail capturing privileged operations, authentication events, and administrative overrides.

| Column Name | Data Type | Nullable | Default | Description & Constraint |
| :--- | :--- | :---: | :---: | :--- |
| `log_id` | `INT AUTO_INCREMENT`| **NO** | *None* | **Primary Key**: Sequential integer event sequence identifier. |
| `event_type` | `VARCHAR(64)` | **NO** | *None* | Classification: `"CUSTOMER_REGISTRATION"`, `"ADMIN_LOGIN"`, `"UNDO_EXECUTION"`, `"ACCOUNT_BLOCK"`. |
| `details` | `TEXT` | **NO** | *None* | Structured diagnostic context or JSON parameter payload. |
| `logged_at` | `TIMESTAMP` | **NO** | `CURRENT_TIMESTAMP` | High-precision audit timestamp. |

```sql
CREATE TABLE IF NOT EXISTS audit_logs (
    log_id          INT AUTO_INCREMENT NOT NULL,
    event_type      VARCHAR(64)     NOT NULL,
    details         TEXT            NOT NULL,
    logged_at       TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT pk_audit_logs PRIMARY KEY (log_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

---

## 2. Normalization Analysis: Why This Schema Follows 3NF

Database **normalization** is the formal mathematical process of organizing relations, tables, and columns to minimize data redundancy, avoid insertion, update, and deletion anomalies, and guarantee referential integrity.

### 2.1. First Normal Form (1NF) â€” Atomicity & No Repeating Groups
* **1NF Rule**: Every column in every table must contain atomic (indivisible) values. There must be no multi-valued attributes, no comma-separated arrays stored in single text columns, and no repeating column groups (e.g. `account1`, `account2`, `account3`).
* **Implementation in System**:
  - In `customers`, phone numbers, full names, and emails are individual scalar values.
  - A customer's linked accounts are **not** stored as a comma-delimited string in `customers`. Instead, accounts are decomposed into their own independent relation `accounts`, where each row represents exactly one bank account.
  - In `transactions`, currency amounts, status codes, and timestamps are atomic columns.

### 2.2. Second Normal Form (2NF) â€” Full Functional Dependency
* **2NF Rule**: The relation must be in 1NF, and every non-primary-key attribute must depend on the **whole candidate key** (no partial functional dependencies on composite keys).
* **Implementation in System**:
  - Each table uses a single-column primary key (`customer_id`, `account_number`, `transaction_id`, `request_id`, `log_id`).
  - Because no table relies on a composite primary key, partial key dependencies are mathematically impossible.
  - For example, `accounts.balance` and `accounts.is_active` depend entirely and exclusively on `account_number`.

### 2.3. Third Normal Form (3NF) â€” Transitive Dependency Elimination
* **3NF Rule**: The relation must be in 2NF, and no non-key attribute may depend on another non-key attribute (no **transitive dependencies**; non-key $\to$ non-key is prohibited). In the famous words of Bill Kent: *"Every non-key attribute must provide a fact about the key, the whole key, and nothing but the key, so help me Codd."*
* **Implementation in System**:
  - In a naive, unnormalized design, the `accounts` table might store: `(account_number, customer_id, customer_name, customer_email, customer_phone, balance)`.
  - In that unnormalized design, `customer_name` and `customer_email` depend on `customer_id`, which in turn depends on `account_number`:
    $$\text{account\_number} \xrightarrow{\text{determines}} \text{customer\_id} \xrightarrow{\text{determines}} \text{customer\_name}$$
    This creates a **transitive dependency**.
  - *Why this is dangerous (Anomalies)*:
    1. **Redundancy**: If Alice has 4 accounts (Savings, Current, Fixed Deposit, Joint), her name, phone, and email are duplicated 4 times.
    2. **Update Anomaly**: If Alice updates her phone number, failing to update all 4 rows leaves the database in an inconsistent state.
    3. **Insertion Anomaly**: We cannot register a new customer until they open an account.
    4. **Deletion Anomaly**: Closing all of Alice's accounts would accidentally purge her customer contact profile from the bank.
  - *The 3NF Solution in Smart Banking*: We decompose the schema:
    - Customer contact information is placed exclusively in `customers`.
    - Account balance and state information is placed exclusively in `accounts`.
    - The two tables are linked solely by the foreign key `accounts.customer_id -> customers.customer_id`.
    - No customer attributes are duplicated across accounts or transactions.

---

## 3. Entity-Relationship (ER) Architecture & Cardinalities

The relationship topology matches the actual foreign-key constraints implemented in [`backend/database/MySqlSchema.h`](../backend/database/MySqlSchema.h):

```text
                  +--------------------------+
                  |        customers         |
                  |  PK: customer_id         |
                  +--------------------------+
                               â”‚
            â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
            â”‚ 1 : N                               â”‚ 1 : N
            â”‚ (fk_acc_customer)                   â”‚ (fk_req_customer)
            â–¼                                     â–¼
+--------------------------+         +--------------------------+
|         accounts         |         |     pending_requests     |
|  PK: account_number      |         |  PK: request_id          |
|  FK: customer_id         |         |  FK: customer_id         |
+--------------------------+         +--------------------------+
            â”‚
            â”‚ Logical Account References
            â”‚ (from_account, to_account)
            â–¼
+--------------------------+
|       transactions       |         +--------------------------+
|  PK: transaction_id      |         |        audit_logs        |
|  INDEX: idx_txn_from     |         |  PK: log_id (Auto-Inc)   |
|  INDEX: idx_txn_to       |         |  Independent Event Log   |
+--------------------------+         +--------------------------+
```

### Relationship Breakdown:
1. **`customers` $\to$ `accounts` ($1 : N$ Cardinality)**:
   - A single customer entity can open and own multiple distinct banking accounts (e.g. one Savings Account for interest generation and one Current Account for business cash flow).
   - Each bank account belongs to exactly one registered customer owner.
   - Enforced by `CONSTRAINT fk_acc_customer FOREIGN KEY (customer_id) REFERENCES customers(customer_id) ON DELETE CASCADE ON UPDATE CASCADE`.
2. **`accounts` $\to$ `transactions` (Reference Cardinality)**:
   - Every financial transaction references a source account (`from_account`) and/or a destination account (`to_account`).
   - For internal fund transfers, both accounts must exist in `accounts`.
   - For cash deposits or ATM withdrawals, the external clearing side is recorded as `"N/A"`, allowing the audit ledger to preserve financial history even for external settlement counterparties.
3. **`customers` $\to$ `pending_requests` ($1 : N$ Cardinality)**:
   - A customer may submit multiple asynchronous requests over time (microloans, limit adjustments, unblock reviews).
   - Enforced by `CONSTRAINT fk_req_customer FOREIGN KEY (customer_id) REFERENCES customers(customer_id) ON DELETE CASCADE`.
4. **`audit_logs` (Independent Relation)**:
   - System security and administrative events are stored independently to prevent tight coupling and ensure that security logs remain tamper-evident and decoupled from entity lifecycles.

---

## 4. Database Constraints & Indexing Strategy

### 4.1. Constraints Enforced

| Constraint Name | Type | Table & Target | Purpose & Business Logic |
| :--- | :--- | :--- | :--- |
| `pk_customers` | `PRIMARY KEY` | `customers(customer_id)` | Guarantees entity uniqueness; prevents duplicate customer registrations. |
| `pk_accounts` | `PRIMARY KEY` | `accounts(account_number)` | Prevents duplicate account number collisions. |
| `pk_transactions` | `PRIMARY KEY` | `transactions(transaction_id)` | Enforces unique transaction tracking IDs (`TXN-XXXXXX`). |
| `pk_requests` | `PRIMARY KEY` | `pending_requests(request_id)` | Ensures unique tracking for all submitted administrative requests. |
| `pk_audit_logs` | `PRIMARY KEY` | `audit_logs(log_id)` | Auto-incrementing sequential log identifier. |
| `fk_acc_customer` | `FOREIGN KEY` | `accounts(customer_id)` $\to$ `customers(customer_id)` | Eliminates orphaned bank accounts; links each account to a valid customer. |
| `fk_req_customer` | `FOREIGN KEY` | `pending_requests(customer_id)` $\to$ `customers(customer_id)` | Ensures requests originate from a verified customer profile. |

### 4.2. Cascade Actions Explained
- **`ON DELETE CASCADE`**:
  - *Behavior*: If a customer profile is legally deleted or purged, MySQL InnoDB automatically deletes all child records associated with that customer ID in `accounts` and `pending_requests`.
  - *Why Useful*: Prevents "orphan records" (accounts without owners) from accumulating in the database and corrupting balance sheets.
- **`ON UPDATE CASCADE`**:
  - *Behavior*: If a customer's primary identifier is migrated or amended in `customers`, the foreign key values in child tables are updated automatically by the DBMS engine.
  - *Why Useful*: Maintains unbroken referential links without requiring complex multi-table update scripts.

### 4.3. B-Tree Indexing Strategy

In InnoDB, primary keys automatically create clustered indexes. Secondary B-Tree indexes were created in [`MySqlSchema.h`](../backend/database/MySqlSchema.h) to accelerate frequent query patterns:

```sql
INDEX idx_acc_customer (customer_id)      -- accounts table
INDEX idx_acc_balance (balance)           -- accounts table
INDEX idx_txn_from (from_account)         -- transactions table
INDEX idx_txn_to (to_account)             -- transactions table
INDEX idx_txn_timestamp (timestamp)       -- transactions table
```

1. **`idx_acc_customer` on `accounts(customer_id)`**:
   - *Query Accelerated*: `SELECT * FROM accounts WHERE customer_id = 'CUST-101';`
   - *Impact*: When a customer logs into the React frontend, their dashboard immediately retrieves all owned accounts. The B-Tree index resolves this query in $O(\log N)$ tree traversals instead of an $O(N)$ full table scan across millions of bank accounts.
2. **`idx_acc_balance` on `accounts(balance)`**:
   - *Query Accelerated*: `SELECT * FROM accounts WHERE balance >= 5000.00 ORDER BY balance DESC;`
   - *Impact*: Used by bank relationship managers to locate high-net-worth customers and manage liquidity reserves without scanning every account.
3. **`idx_txn_from` and `idx_txn_to` on `transactions`**:
   - *Query Accelerated*: Locating all debits or credits affecting an account.
   - *Impact*: Essential for calculating current balances and rendering customer bank statements in sub-millisecond response times.
4. **`idx_txn_timestamp` on `transactions`**:
   - *Query Accelerated*: Date-range queries (`WHERE timestamp BETWEEN '2026-01-01' AND '2026-06-30'`).
   - *Impact*: Enables the query engine to jump directly to the start timestamp boundary via B-Tree search rather than inspecting every historic transaction.

---

## 5. Dual Storage Architecture: C++ Engine, MySQL DAL, & Resilient File Persistence

The banking system implements two complementary persistence pathways:

```
                               â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
                               â”‚     C++ Banking Engine      â”‚
                               â”‚  (BankSystem, Models, DTOs) â”‚
                               â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
                                              â”‚
                    â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
                    â”‚                                                   â”‚
                    â–¼                                                   â–¼
       â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”                          â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
       â”‚    DatabaseManager     â”‚                          â”‚    FilePersistence     â”‚
       â”‚  (Relational SQL DAL)  â”‚                          â”‚ (Resilient Disk Layer) â”‚
       â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜                          â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”¬â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
                    â”‚                                                   â”‚
        â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”                           â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”´â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
        â–¼                       â–¼                           â–¼                       â–¼
â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”        â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”             â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”        â”Œâ”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”
â”‚  schema.sql  â”‚        â”‚  SQL Dump    â”‚             â”‚  .tmp Stage  â”‚        â”‚  .bak Backup â”‚
â”‚ (MySQL DDL)  â”‚        â”‚ (ACID Insertsâ”‚             â”‚ (Crash-Safe) â”‚        â”‚ (Auto-Mirror)â”‚
â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜        â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜             â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜        â””â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”˜
```

### 5.1. The Relational Database Layer (`DatabaseManager.cpp`)
- **Role**: Provides relational algebra abstraction, SQL DDL generation, referential integrity verification, and synchronized SQL dump serialization.
- **SQL DDL Generation (`data/schema.sql`)**: Emits complete MySQL 8.0/InnoDB compatible creation scripts with primary keys, foreign keys, cascade rules, and indexes.
- **Synchronized Data Dump (`data/banking_system_dump.sql`)**: Automatically exports memory state into valid SQL `INSERT` statements with deduplicated transaction IDs and PBKDF2 hashed credentials. This dump can be piped directly into standard database tools (`mysql -u root -p smart_banking < banking_system_dump.sql` or imported into phpMyAdmin / MySQL Workbench).
- **Referential Integrity Scanner (`DatabaseManager::verifyReferentialIntegrity()`)**: Programmatically verifies in C++ that no orphaned accounts exist and that all customer foreign key relationships are valid.

### 5.2. The Resilient File Persistence Layer (`FilePersistence.cpp`)
- **Role**: Provides zero-dependency, local file storage so the application runs immediately without requiring an active MySQL daemon installed on the host OS.
- **Write-to-Temporary Staging (`.tmp`)**: Data is never written directly to primary storage files (`customers.txt`, `accounts.txt`). It is first serialized into staging files (`customers.txt.tmp`). If the process crashes mid-write (e.g. power failure), primary files remain uncorrupted.
- **Disaster Recovery Backup Mirrors (`.bak`)**: Before committing new data, existing primary files are copied to `.bak` mirrors (`customers.txt.bak`).
- **Atomic File Replacement**: Uses OS atomic file replacement (`std::remove` followed by `std::rename`) to swap staging files into place instantly.
- **Corrupted Record Tolerance**: If a disk block is damaged or a row contains corrupted text, the parser safely skips the bad record, records a structured diagnostic warning in `PersistenceResult`, and continues loading remaining accounts rather than crashing.

---

## 6. Transaction Persistence & Financial Audit Trail

Financial ledgers require immutability and complete traceability:

1. **Operation Execution**: When a customer executes a transfer of $500 from `SAV-1001` to `CUR-1002`:
   - An outgoing transaction record (`TXN-XXXXXX`, `TRANSFER_OUT`) is created with timestamp and status `SUCCESS`.
   - An incoming transaction record (`TXN-XXXXXX`, `TRANSFER_IN`) is credited to the recipient.
2. **Audit Ledger Logging**: Both entries are appended to the respective accounts' in-memory `m_transactionHistory` vectors and recorded in the global ledger (`getAllTransactionsLedger()`).
3. **Atomic Persistence Commit**: Upon state mutation, the C++ backend automatically triggers disk synchronization:
   - Flushes atomic flat-file ledgers to `data/transactions.txt`.
   - Generates deduplicated SQL `INSERT` statements in `data/banking_system_dump.sql`.
4. **Cascading Reversibility**: A reciprocal compensation entry is pushed onto `BankSystem::m_reversalStack` (`std::stack`). If an admin executes an undo operation, the system does not delete the historic transaction (which violates banking compliance laws); it appends a new compensating transaction (`"Reversal: Transfer Refund"`) that reverses balances while keeping the full historical audit trail intact.

---

## 7. SQL Dump & Schema File Artifacts

The repository maintains two live database artifacts in the `data/` directory:

### 1. `data/schema.sql` (Pure DDL Architecture)
Contains the idempotent Data Definition Language (DDL) commands:
- Database creation (`CREATE DATABASE IF NOT EXISTS smart_banking`)
- All 5 table definitions with data types, defaults, and primary/foreign key constraints
- All B-Tree index specifications

### 2. `data/banking_system_dump.sql` (Complete Database State)
Contains the DDL followed by table data inserts:
- Customer records containing salted PBKDF2 hash strings
- Account records with balances, overdraft limits, and foreign key `customer_id` links
- Immutable transaction entries with amounts, timestamps, and status flags
- Pending administrative requests in the approval queue

---

## 8. Security & Cryptographic Credential Storage

A critical database design requirement is that sensitive customer credentials must never be stored in plaintext.

```text
Customer Input ("1234") â”€â”€â–º PBKDF2-HMAC-SHA256 (10,000 Iterations + 128-bit CSPRNG Salt)
                                               â”‚
                                               â–¼
                              Database Stored PIN (VARCHAR(256)):
"$pbkdf2-sha256$10000$ddddb2091dac503434112ae5cb24cb2b$f5a28033a28b8b4f936550d4bfebcd1c4cc85ca032f8d5aad46531a99b36d230"
```

1. **PBKDF2 Hashing**: Every customer PIN is hashed upon registration using **PBKDF2-HMAC-SHA256 (NIST SP 800-132 / RFC 2898)** with **10,000 iterations** and a **16-byte (128-bit) cryptographically secure salt** generated via Windows CryptoAPI `CryptGenRandom`.
2. **Database Column Definition**: The `pin` column in `customers` is defined as `VARCHAR(256) NOT NULL`, accommodating the full serialization envelope: `$pbkdf2-sha256$<iterations>$<salt>$<hash>`.
3. **Zero Plaintext Exposure**: Plaintext PINs never appear in `data/customers.txt`, `data/banking_system_dump.sql`, or API JSON payloads. Even if an attacker obtains an offline copy of the database dump, they cannot reverse the hashes without substantial computational work.

---

## 9. Common Viva Questions & Model Answers

### Question 1: What is a Relational Database?
> **Answer**: A relational database is a structured data store that organizes information into two-dimensional tables (relations) consisting of rows (tuples) and columns (attributes). Data across tables is related through shared mathematical keys (Primary and Foreign Keys), and data integrity and transactions are governed by the relational algebra and ACID principles (Atomicity, Consistency, Isolation, Durability).

---

### Question 2: What is Third Normal Form (3NF)?
> **Answer**: A relational database schema is in Third Normal Form if:
> 1. It is in First Normal Form (all column values are atomic, with no repeating groups).
> 2. It is in Second Normal Form (no non-key attribute has a partial functional dependency on any composite candidate key).
> 3. It contains no transitive functional dependencies, meaning no non-key attribute depends on another non-key attribute ($X \to Y$ is prohibited unless $X$ is a superkey). Every non-key attribute depends strictly on the primary key alone.

---

### Question 3: Why did you separate `customers` and `accounts` into distinct tables instead of using one table?
> **Answer**: To achieve 3NF normalization and avoid redundancy and data anomalies. A single customer can own multiple bank accounts (a $1 : N$ relationship). If combined into a single table, customer attributes (name, email, phone, salted PIN) would be duplicated for every account they open. Updating an email would risk update anomalies; closing all accounts would accidentally erase the customer's contact record (deletion anomaly). Separating them eliminates duplication and allows independent entity lifecycles.

---

### Question 4: What is a Primary Key?
> **Answer**: A primary key is a column or minimal set of columns that uniquely identifies each individual record within a database table. Primary keys enforce **Entity Integrity**: they must be unique across all rows and cannot contain `NULL` values (`NOT NULL`). In our project, `customer_id` is the primary key for `customers`, and `account_number` is the primary key for `accounts`.

---

### Question 5: What is a Foreign Key?
> **Answer**: A foreign key is a column (or combination of columns) in a child table whose values must match the primary key of a parent table, or be `NULL`. Foreign keys enforce **Referential Integrity**, ensuring that relationships between tables remain consistent and preventing child records from referencing non-existent parent entities. For example, `accounts.customer_id` is a foreign key referencing `customers.customer_id`.

---

### Question 6: Why do we use Database Indexes?
> **Answer**: Indexes are auxiliary data structures (typically balanced B-Trees or B+ Trees) that allow the DBMS query engine to locate matching rows in $O(\log N)$ time without performing an expensive $O(N)$ full table scan. In our system, an index on `accounts(customer_id)` allows the customer dashboard to retrieve an individual's accounts instantly, and indexes on `transactions(from_account)` and `transactions(to_account)` accelerate bank statement generation.

---

### Question 7: What is Referential Integrity?
> **Answer**: Referential integrity is a relational database property ensuring that all foreign key references are valid and consistent throughout the database lifecycle. A child table cannot reference a parent record that does not exist. If a parent record is deleted or its key is updated, referential integrity rules (such as `CASCADE`, `RESTRICT`, or `SET NULL`) dictate how the DBMS handles dependent child rows to prevent orphaned records.

---

### Question 8: What does `ON DELETE CASCADE` do?
> **Answer**: `ON DELETE CASCADE` is a foreign key constraint action. When a parent record in the referenced table is deleted, the DBMS automatically and recursively deletes all associated child records in referencing tables. In our project, if a customer record is deleted, `ON DELETE CASCADE` on `fk_acc_customer` ensures all that customer's accounts and pending requests are removed atomically, preventing orphaned financial records.

---

### Question 9: Why is transaction history stored separately from account balances?
> **Answer**: In financial systems, account balance and transaction history serve two distinct architectural needs:
> 1. `accounts.balance` represents the **current consolidated state** of an account, optimized for high-speed balance checks during withdrawals.
> 2. `transactions` represents an **immutable, append-only historical audit trail** documenting how and why that balance changed over time.
> Separating them preserves historical auditability, supports double-entry bookkeeping, enables forensic fraud analysis, and allows transaction rollbacks without destructive data overwrites.

---

### Question 10: What is the purpose of the `audit_logs` table?
> **Answer**: The `audit_logs` table provides an independent, tamper-evident security audit trail for administrative and compliance monitoring. While `transactions` logs monetary movements, `audit_logs` records system-level security events: admin logins, customer registration attempts, administrative account unblocks, loan application approvals, and manual transaction reversals.

---

### Question 11: What is the difference between file persistence and MySQL persistence in this project?
> **Answer**: 
> - **File Persistence (`FilePersistence.cpp`)**: A lightweight, standalone persistence engine built directly in C++ that writes structured text files with `.tmp` staging, atomic file renaming, and `.bak` backups. It allows the project to run out of the box with zero external software dependencies.
> - **MySQL Persistence (`DatabaseManager.cpp`)**: An enterprise relational persistence layer that formats the banking state into 3NF normalized tables with foreign keys, indexes, and full SQL dump exports (`banking_system_dump.sql`) ready for MySQL Server, MariaDB, or cloud RDBMS deployments.

---

### Question 12: Why should customer PINs never be stored as plaintext in the database?
> **Answer**: If credentials are stored in plaintext, any unauthorized read access - such as a database SQL dump leak, an internal insider threat, backup disk theft, or an SQL injection vulnerability - immediately compromises every customer's account. By storing PINs as salted **PBKDF2-HMAC-SHA256** hashes with 10,000 rounds:
> 1. The original PIN cannot be reversed mathematically from the hash.
> 2. The unique 128-bit salt prevents precomputed **Rainbow Table attacks**.
> 3. The 10,000 iterations make offline brute-force and dictionary attacks computationally expensive.