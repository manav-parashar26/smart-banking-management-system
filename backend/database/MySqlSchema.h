#pragma once

#include <string>

// ============================================================================
// RELATIONAL DATABASE SCHEMA DEFINITIONS (MySQL 8.0 / InnoDB)
// Demonstrating 3NF Normalization, Primary Keys, Foreign Keys, Indexes & Constraints
// ============================================================================

namespace MySqlSchema {

inline std::string getCompleteDDL() {
    return R"(
-- ============================================================================
-- SMART BANKING MANAGEMENT SYSTEM - RELATIONAL DATABASE SCHEMA (3NF)
-- DBMS: MySQL 8.0+ / MariaDB 10.3+
-- Engine: InnoDB (Full ACID Support & Referential Integrity)
-- ============================================================================

CREATE DATABASE IF NOT EXISTS smart_banking;
USE smart_banking;

-- 1. Customers Table (Entity Integrity)
CREATE TABLE IF NOT EXISTS customers (
    customer_id     VARCHAR(32)     NOT NULL,
    full_name       VARCHAR(128)    NOT NULL,
    email           VARCHAR(128)    NOT NULL,
    phone           VARCHAR(32)     NOT NULL,
    pin             VARCHAR(256)    NOT NULL,
    created_at      TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT pk_customers PRIMARY KEY (customer_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 2. Bank Accounts Table (Referential Integrity via Foreign Key)
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

-- 3. Financial Transaction Audit Ledger
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

-- 4. Pending Banking Requests (Queue Storage)
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

-- 5. System Audit Trail
CREATE TABLE IF NOT EXISTS audit_logs (
    log_id          INT AUTO_INCREMENT NOT NULL,
    event_type      VARCHAR(64)     NOT NULL,
    details         TEXT            NOT NULL,
    logged_at       TIMESTAMP       DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT pk_audit_logs PRIMARY KEY (log_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
)";
}

} // namespace MySqlSchema
