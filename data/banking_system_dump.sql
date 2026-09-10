-- ============================================================================
-- SMART BANKING MANAGEMENT SYSTEM - COMPLETE MYSQL DATA DUMP
-- Generated automatically by C++ Database Manager
-- ============================================================================


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

-- ============================================================================
-- TABLE DATA INSERTS (ACID Relational Representation)
-- ============================================================================

-- Dumping data for table `customers`
INSERT INTO customers (customer_id, full_name, email, phone, pin) VALUES
('CUST-104', 'Grace Hopper', 'grace3313@navy.mil', '+1-555-3313', '$pbkdf2-sha256$10000$5b510463cd4e849ec84da0ae1a33dbe0$a5e8dda51a47f17e9f68e8a23c4e27db34718d02d54987bb0dc24a8e8b8569d6'),
('CUST-102', 'Bob Builder', 'bob@domain.com', '+1-555-2020', '$pbkdf2-sha256$10000$ddddb2091dac503434112ae5cb24cb2b$f5a28033a28b8b4f936550d4bfebcd1c4cc85ca032f8d5aad46531a99b36d230'),
('CUST-999', 'Alan Turing', 'alan@bletchley.ac.uk', '+44-123-456', '$pbkdf2-sha256$10000$52efd9e62bda1f5ce6d20df2bff11d16$0cec8ded5f1509c2d997d76ae4b61832cd3d329c6e38e2cbc47dd60e6302f001'),
('CUST-101', 'Alice Vance', 'alice@domain.com', '+1-555-1010', '$pbkdf2-sha256$10000$1ad3ab3d6595efee0c02d17686048422$ff395dc795840fbaf7f5c329d9cea0f1f3f6314c368b346612bb54b43bcf501e'),
('CUST-103', 'Bruce Wayne', 'bruce9161@wayne.com', '+1-555-9161', '$pbkdf2-sha256$10000$c80a7a5af9aea4535af108efaf40a984$62ba812494fbc76c6dcc519c0f6b1c421b287d859f18122f4714e6c09c098198'),
('CUST-105', 'Grace Hopper', 'grace8019@navy.mil', '+1-555-8019', '$pbkdf2-sha256$10000$34512290761af06761f347b3e5c45ad6$822b687b1f53da803e296c5441f2c23be82251deec24e306068cea5046e29b4e');

-- Dumping data for table `accounts`
INSERT INTO accounts (account_number, customer_id, account_type, holder_name, balance, is_active, interest_rate, minimum_balance, overdraft_limit) VALUES
('CUR-1002', 'CUST-102', 'Current Account', 'Bob Builder', 3410.00, TRUE, 0.00, 0.00, 1500.00),
('SAV-1001', 'CUST-104', 'Savings Account', 'Alan Turing', 57400.00, TRUE, 4.00, 500.00, 0.00);

-- Dumping data for table `transactions`
INSERT INTO transactions (transaction_id, from_account, to_account, amount, timestamp, type_id, type_name, status_id, status_name, description) VALUES
('TXN-100006', 'SAV-1001', 'CUR-1002', 25.00, '2026-09-10 18:21:41', 3, 'TRANSFER_OUT', 0, 'SUCCESS', 'Transfer to CUR-1002 (PIN Security Test Transfer)'),
('TXN-100007', 'SAV-1001', 'CUR-1002', 50.00, '2026-09-10 17:37:43', 3, 'TRANSFER_OUT', 0, 'SUCCESS', 'Transfer to CUR-1002 (DTO Test Transfer)'),
('TXN-100012', 'SAV-1001', 'CUR-1002', 250.00, '2026-09-10 17:37:44', 3, 'TRANSFER_OUT', 0, 'SUCCESS', 'Transfer to CUR-1002 (Test Step 3: Transfer)'),
('TXN-100019', 'SAV-1001', 'CUR-1002', 500.00, '2026-09-10 17:37:44', 3, 'TRANSFER_OUT', 0, 'SUCCESS', 'Transfer to CUR-1002 (E2E Workflow Transfer)'),
('TXN-100008', 'SAV-1001', 'CUR-1002', 50.00, '2026-09-10 17:37:43', 2, 'TRANSFER_IN', 0, 'SUCCESS', 'Transfer from SAV-1001 (DTO Test Transfer)'),
('TXN-100013', 'SAV-1001', 'CUR-1002', 250.00, '2026-09-10 17:37:44', 2, 'TRANSFER_IN', 0, 'SUCCESS', 'Transfer from SAV-1001 (Test Step 3: Transfer)'),
('TXN-100020', 'SAV-1001', 'CUR-1002', 500.00, '2026-09-10 17:37:44', 2, 'TRANSFER_IN', 0, 'SUCCESS', 'Transfer from SAV-1001 (E2E Workflow Transfer)'),
('TXN-100002', 'N/A', 'CUR-1002', 2000.00, '2026-09-10 16:31:58', 0, 'DEPOSIT', 0, 'SUCCESS', 'Initial Account Deposit'),
('TXN-100014', 'CUR-1002', 'N/A', 250.00, '2026-09-10 17:37:44', 1, 'WITHDRAWAL', 0, 'SUCCESS', 'Reversal: Transfer Refund to SAV-1001'),
('TXN-100021', 'CUR-1002', 'N/A', 500.00, '2026-09-10 17:37:44', 1, 'WITHDRAWAL', 0, 'SUCCESS', 'Reversal: Transfer Refund to SAV-1001'),
('TXN-100003', 'N/A', 'SAV-1001', 500.00, '2026-09-10 18:27:00', 0, 'DEPOSIT', 0, 'SUCCESS', 'Initial Account Deposit'),
('TXN-100004', 'N/A', 'SAV-1001', 300.00, '2026-09-10 18:27:00', 0, 'DEPOSIT', 0, 'SUCCESS', 'Salary Deposit'),
('TXN-100005', 'SAV-1001', 'N/A', 100.00, '2026-09-10 18:27:00', 1, 'WITHDRAWAL', 0, 'SUCCESS', 'ATM Cash'),
('TXN-100010', 'N/A', 'SAV-1001', 100.00, '2026-09-10 18:27:00', 0, 'DEPOSIT', 0, 'SUCCESS', 'Reversal: Transfer Recovered from CUR-1002'),
('TXN-100009', 'N/A', 'SAV-1001', 1000.00, '2026-09-10 18:36:20', 0, 'DEPOSIT', 0, 'SUCCESS', 'Approved Loan Disbursal (REQ-5002)'),
('TXN-100011', 'SAV-1001', 'N/A', 100.00, '2026-09-10 18:36:20', 1, 'WITHDRAWAL', 0, 'SUCCESS', 'Test Step 2: Withdrawal'),
('TXN-100015', 'N/A', 'SAV-1001', 250.00, '2026-09-10 18:36:20', 0, 'DEPOSIT', 0, 'SUCCESS', 'Reversal: Transfer Recovered from CUR-1002'),
('TXN-100016', 'N/A', 'SAV-1001', 100.00, '2026-09-10 18:36:20', 0, 'DEPOSIT', 0, 'SUCCESS', 'Reversal: Undo Withdrawal TXN-100011'),
('TXN-100017', 'SAV-1001', 'N/A', 300.00, '2026-09-10 18:36:20', 1, 'WITHDRAWAL', 0, 'SUCCESS', 'Reversal: Undo Deposit TXN-100010'),
('TXN-100018', 'N/A', 'SAV-1001', 150.00, '2026-09-10 18:36:21', 0, 'DEPOSIT', 0, 'SUCCESS', 'REST Test Deposit'),
('TXN-100022', 'N/A', 'SAV-1001', 500.00, '2026-09-10 18:36:21', 0, 'DEPOSIT', 0, 'SUCCESS', 'Reversal: Transfer Recovered from CUR-1002'),
('TXN-100023', 'N/A', 'SAV-1001', 250.00, '2026-09-10 18:36:21', 0, 'DEPOSIT', 0, 'SUCCESS', 'Approved Loan Disbursal (REQ-5005)');

-- Dumping data for table `pending_requests`
