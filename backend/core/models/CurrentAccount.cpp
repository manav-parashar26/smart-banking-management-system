#include "CurrentAccount.h"
#include <iomanip>
#include <sstream>

CurrentAccount::CurrentAccount(const std::string& accNum, 
                               const std::string& holder, 
                               double initialBalance, 
                               double overdraftLimit)
    : Account(accNum, holder, initialBalance),
      m_overdraftLimit(overdraftLimit) {
    if (overdraftLimit < 0.0) {
        throw InvalidAmountException("Overdraft limit cannot be negative.");
    }
}

void CurrentAccount::withdraw(double amount, 
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
        throw AccountBlockedException("Cannot withdraw from Current Account " + m_accountNumber + ": Account is BLOCKED.");
    }
    if (amount <= 0.0) {
        throw InvalidAmountException("Withdrawal amount must be strictly greater than 0.");
    }

    double totalAvailableFunds = m_balance + m_overdraftLimit;

    if (amount > totalAvailableFunds) {
        recordTransaction(Transaction(
            Transaction::generateTransactionId(),
            m_accountNumber,
            toAccount,
            amount,
            type,
            TransactionStatus::FAILED,
            "Withdrawal rejected: Overdraft limit exceeded"
        ));
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Withdrawal of $" << amount << " exceeds available funds including overdraft ($" 
            << totalAvailableFunds << " available: Balance $" << m_balance 
            << " + Overdraft Limit $" << m_overdraftLimit << ").";
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

double CurrentAccount::getOverdraftLimit() const {
    return m_overdraftLimit;
}

double CurrentAccount::getAvailableCredit() const {
    return m_balance + m_overdraftLimit;
}

void CurrentAccount::displayAccountDetails() const {
    std::cout << "----------------------------------------\n";
    std::cout << "Account Type     : " << getAccountType() << "\n";
    std::cout << "Account Number   : " << m_accountNumber << "\n";
    std::cout << "Account Holder   : " << m_holderName << "\n";
    std::cout << "Current Balance  : $" << std::fixed << std::setprecision(2) << m_balance << "\n";
    std::cout << "Overdraft Limit  : $" << std::fixed << std::setprecision(2) << m_overdraftLimit << "\n";
    std::cout << "Available Credit : $" << std::fixed << std::setprecision(2) << getAvailableCredit() << "\n";
    std::cout << "Account Status   : " << (m_isActive ? "ACTIVE" : "BLOCKED") << "\n";
    std::cout << "Transaction Count: " << m_transactionHistory.size() << " recorded\n";
    std::cout << "----------------------------------------\n";
}

std::string CurrentAccount::getAccountType() const {
    return "Current Account";
}

AccountDTO CurrentAccount::toDTO() const {
    return AccountDTO{
        m_accountNumber,
        getAccountType(),
        m_holderName,
        m_balance,
        m_isActive,
        0.0,
        0.0,
        m_overdraftLimit,
        getAvailableCredit()
    };
}

