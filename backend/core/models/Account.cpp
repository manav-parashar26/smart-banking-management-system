#include "Account.h"
#include <iomanip>
#include <sstream>

Account::Account(const std::string& accNum, const std::string& holder, double initialBalance)
    : m_accountNumber(accNum), m_holderName(holder), m_balance(0.0), m_isActive(true) {
    if (initialBalance < 0.0) {
        throw InvalidAmountException("Initial balance cannot be negative.");
    }
    m_balance = initialBalance;
    if (initialBalance > 0.0) {
        recordTransaction(Transaction(
            Transaction::generateTransactionId(),
            "N/A",
            m_accountNumber,
            initialBalance,
            TransactionType::DEPOSIT,
            TransactionStatus::SUCCESS,
            "Initial Account Deposit"
        ));
    }
}

void Account::recordTransaction(const Transaction& txn) {
    m_transactionHistory.push_back(txn);
}

std::string Account::getAccountNumber() const { return m_accountNumber; }
std::string Account::getHolderName() const { return m_holderName; }
double Account::getBalance() const { return m_balance; }
bool Account::isActive() const { return m_isActive; }

void Account::blockAccount() { m_isActive = false; }
void Account::activateAccount() { m_isActive = true; }

void Account::deposit(double amount, 
                      const std::string& description,
                      TransactionType type,
                      const std::string& fromAccount) {
    if (!m_isActive) {
        recordTransaction(Transaction(
            Transaction::generateTransactionId(),
            fromAccount,
            m_accountNumber,
            amount,
            type,
            TransactionStatus::FAILED,
            "Deposit rejected: Account is blocked"
        ));
        throw AccountBlockedException("Cannot deposit to account " + m_accountNumber + ": Account is BLOCKED.");
    }
    if (amount <= 0.0) {
        throw InvalidAmountException("Deposit amount must be strictly greater than 0.");
    }

    m_balance += amount;

    recordTransaction(Transaction(
        Transaction::generateTransactionId(),
        fromAccount,
        m_accountNumber,
        amount,
        type,
        TransactionStatus::SUCCESS,
        description
    ));
}

void Account::withdraw(double amount, 
                       const std::string& description,
                       TransactionType type,
                       const std::string& toAccount) {
    if (!m_isActive) {
        recordTransaction(Transaction(
            Transaction::generateTransactionId(),
            m_accountNumber,
            toAccount,
            amount,
            type,
            TransactionStatus::FAILED,
            "Withdrawal rejected: Account is blocked"
        ));
        throw AccountBlockedException("Cannot withdraw from account " + m_accountNumber + ": Account is BLOCKED.");
    }
    if (amount <= 0.0) {
        throw InvalidAmountException("Withdrawal amount must be strictly greater than 0.");
    }
    if (amount > m_balance) {
        recordTransaction(Transaction(
            Transaction::generateTransactionId(),
            m_accountNumber,
            toAccount,
            amount,
            type,
            TransactionStatus::FAILED,
            "Withdrawal rejected: Insufficient balance"
        ));
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Withdrawal of $" << amount << " exceeds available balance of $" << m_balance << ".";
        throw InsufficientFundsException(oss.str());
    }

    m_balance -= amount;

    recordTransaction(Transaction(
        Transaction::generateTransactionId(),
        m_accountNumber,
        toAccount,
        amount,
        type,
        TransactionStatus::SUCCESS,
        description
    ));
}

void Account::transferTo(Account& destination, double amount, const std::string& description) {
    if (&destination == this) {
        throw BankingException("Transfer failed: Source and destination accounts cannot be identical.");
    }
    if (!m_isActive) {
        throw AccountBlockedException("Transfer failed from " + m_accountNumber + ": Source account is BLOCKED.");
    }
    if (!destination.isActive()) {
        throw AccountBlockedException("Transfer failed to " + destination.getAccountNumber() + ": Destination account is BLOCKED.");
    }
    if (amount <= 0.0) {
        throw InvalidAmountException("Transfer amount must be strictly greater than 0.");
    }

    // Step 1: Withdraw from source account (will enforce subclass-specific balance/overdraft rules)
    this->withdraw(
        amount,
        "Transfer to " + destination.getAccountNumber() + (description.empty() ? "" : " (" + description + ")"),
        TransactionType::TRANSFER_OUT,
        destination.getAccountNumber()
    );

    // Step 2: Deposit into destination account
    destination.deposit(
        amount,
        "Transfer from " + this->getAccountNumber() + (description.empty() ? "" : " (" + description + ")"),
        TransactionType::TRANSFER_IN,
        this->getAccountNumber()
    );
}

void Account::displayAccountDetails() const {
    std::cout << "----------------------------------------\n";
    std::cout << "Account Type     : " << getAccountType() << "\n";
    std::cout << "Account Number   : " << m_accountNumber << "\n";
    std::cout << "Account Holder   : " << m_holderName << "\n";
    std::cout << "Current Balance  : $" << std::fixed << std::setprecision(2) << m_balance << "\n";
    std::cout << "Account Status   : " << (m_isActive ? "ACTIVE" : "BLOCKED") << "\n";
    std::cout << "Transaction Count: " << m_transactionHistory.size() << " recorded\n";
    std::cout << "----------------------------------------\n";
}


const std::vector<Transaction>& Account::getTransactionHistory() const {
    return m_transactionHistory;
}

void Account::printAccountStatement() const {
    std::cout << "\n========================================================================================================\n";
    std::cout << "                             ACCOUNT STATEMENT: " << m_accountNumber << " (" << m_holderName << ")\n";
    std::cout << "                             Current Balance  : $" << std::fixed << std::setprecision(2) << m_balance << "\n";
    std::cout << "========================================================================================================\n";
    std::cout << std::left 
              << std::setw(12) << "Txn ID" << " | "
              << std::setw(19) << "Timestamp" << " | "
              << std::setw(13) << "Type" << " | "
              << std::setw(10) << "Amount" << " | "
              << std::setw(24) << "Route (From -> To)" << " | "
              << std::setw(8) << "Status" << " | "
              << "Description\n";
    std::cout << "--------------------------------------------------------------------------------------------------------\n";

    if (m_transactionHistory.empty()) {
        std::cout << "No transaction records found.\n";
    } else {
        for (const auto& txn : m_transactionHistory) {
            txn.displayTransaction();
        }
    }
    std::cout << "========================================================================================================\n\n";
}

void Account::loadTransaction(const Transaction& txn) {
    m_transactionHistory.push_back(txn);
}

void Account::clearTransactions() {
    m_transactionHistory.clear();
}

void Account::setBalanceDirect(double balance) {
    m_balance = balance;
}

void Account::setActiveDirect(bool active) {
    m_isActive = active;
}

