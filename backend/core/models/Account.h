#pragma once

#include <string>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "Transaction.h"
#include "BankingDTOs.h"

// ============================================================================
// CUSTOM EXCEPTION HIERARCHY WITH DOMAIN ERROR CODES & HTTP STATUS MAPPINGS
// Demonstrates: Modern OOP Exception Handling & REST API Readiness
// ============================================================================

class BankingException : public std::runtime_error {
protected:
    int m_statusCode;
    std::string m_errorCode;

public:
    explicit BankingException(const std::string& message, 
                             int statusCode = 400, 
                             const std::string& errorCode = "BANKING_ERROR")
        : std::runtime_error(message), m_statusCode(statusCode), m_errorCode(errorCode) {}

    int getStatusCode() const { return m_statusCode; }
    std::string getErrorCode() const { return m_errorCode; }
};

class InvalidAmountException : public BankingException {
public:
    explicit InvalidAmountException(const std::string& message = "Transaction amount must be positive.")
        : BankingException(message, 400, "INVALID_AMOUNT") {}
};

class InsufficientFundsException : public BankingException {
public:
    explicit InsufficientFundsException(const std::string& message = "Insufficient funds to complete this withdrawal.")
        : BankingException(message, 422, "INSUFFICIENT_FUNDS") {}
};

class AccountBlockedException : public BankingException {
public:
    explicit AccountBlockedException(const std::string& message = "Account is blocked. Transactions are suspended.")
        : BankingException(message, 403, "ACCOUNT_BLOCKED") {}
};

class EntityNotFoundException : public BankingException {
public:
    explicit EntityNotFoundException(const std::string& message = "Requested account or customer not found.")
        : BankingException(message, 404, "NOT_FOUND") {}
};

class AuthenticationException : public BankingException {
public:
    explicit AuthenticationException(const std::string& message = "Authentication failed: Invalid credentials.")
        : BankingException(message, 401, "UNAUTHORIZED") {}
};

// ============================================================================
// ABSTRACT BASE ACCOUNT CLASS (ABC)
// Demonstrates: Encapsulation, Pure Virtual Functions, Polymorphic Interface, DTOs
// ============================================================================

class Account {
protected:
    std::string m_accountNumber;
    std::string m_holderName;
    double m_balance;
    bool m_isActive;

    // STL Vector: Audit log of all transactions on this account (O(1) amortized append)
    std::vector<Transaction> m_transactionHistory;

    void recordTransaction(const Transaction& txn);

public:
    // Parameterized Constructor
    Account(const std::string& accNum, const std::string& holder, double initialBalance);

    // Virtual Destructor ensures derived destructor is invoked through base pointer
    virtual ~Account() = default;

    // Getters (Encapsulation)
    std::string getAccountNumber() const;
    std::string getHolderName() const;
    double getBalance() const;
    bool isActive() const;

    // State Modifiers
    void blockAccount();
    void activateAccount();

    // Banking Operations
    virtual void deposit(double amount, 
                         const std::string& description = "Deposit",
                         TransactionType type = TransactionType::DEPOSIT,
                         const std::string& fromAccount = "N/A");

    virtual void withdraw(double amount, 
                          const std::string& description = "Withdrawal",
                          TransactionType type = TransactionType::WITHDRAWAL,
                          const std::string& toAccount = "N/A");

    void transferTo(Account& destination, 
                    double amount, 
                    const std::string& description = "Transfer");

    virtual void displayAccountDetails() const;

    // Pure Virtual Abstraction: Forces subclasses to define their account identity and DTO mapping
    virtual std::string getAccountType() const = 0;
    virtual AccountDTO toDTO() const = 0;

    // Transaction Ledger & Statement
    const std::vector<Transaction>& getTransactionHistory() const;
    void printAccountStatement() const;

    // Persistence Rehydration Helpers
    void loadTransaction(const Transaction& txn);
    void clearTransactions();
    void setBalanceDirect(double balance);
    void setActiveDirect(bool active);
};
