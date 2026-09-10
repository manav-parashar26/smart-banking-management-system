#include "SavingsAccount.h"
#include <iomanip>
#include <sstream>

SavingsAccount::SavingsAccount(const std::string& accNum, 
                               const std::string& holder, 
                               double initialBalance, 
                               double interestRate, 
                               double minBalance)
    : Account(accNum, holder, initialBalance),
      m_interestRate(interestRate),
      m_minimumBalance(minBalance) {
    if (initialBalance < minBalance) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Initial balance ($" << initialBalance 
            << ") cannot be below minimum balance requirement ($" << minBalance << ").";
        throw InvalidAmountException(oss.str());
    }
    if (interestRate < 0.0) {
        throw InvalidAmountException("Interest rate cannot be negative.");
    }
}

void SavingsAccount::withdraw(double amount, 
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
        throw AccountBlockedException("Cannot withdraw from Savings Account " + m_accountNumber + ": Account is BLOCKED.");
    }
    if (amount <= 0.0) {
        throw InvalidAmountException("Withdrawal amount must be strictly greater than 0.");
    }
    if ((m_balance - amount) < m_minimumBalance) {
        recordTransaction(Transaction(
            Transaction::generateTransactionId(),
            m_accountNumber,
            toAccount,
            amount,
            type,
            TransactionStatus::FAILED,
            "Withdrawal rejected: Minimum balance breach"
        ));
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Withdrawal of $" << amount << " would breach minimum balance requirement of $" 
            << m_minimumBalance << " (Current balance: $" << m_balance 
            << ", Max withdrawable: $" << (m_balance - m_minimumBalance) << ").";
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

double SavingsAccount::calculateInterest() const {
    return m_balance * (m_interestRate / 100.0);
}

double SavingsAccount::applyInterest() {
    double interest = calculateInterest();
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Periodic Interest Credited (" << m_interestRate << "%)";
    deposit(interest, oss.str());
    return interest;
}

void SavingsAccount::displayAccountDetails() const {
    std::cout << "----------------------------------------\n";
    std::cout << "Account Type     : " << getAccountType() << "\n";
    std::cout << "Account Number   : " << m_accountNumber << "\n";
    std::cout << "Account Holder   : " << m_holderName << "\n";
    std::cout << "Current Balance  : $" << std::fixed << std::setprecision(2) << m_balance << "\n";
    std::cout << "Interest Rate    : " << m_interestRate << "%\n";
    std::cout << "Minimum Balance  : $" << std::fixed << std::setprecision(2) << m_minimumBalance << "\n";
    std::cout << "Account Status   : " << (m_isActive ? "ACTIVE" : "BLOCKED") << "\n";
    std::cout << "Transaction Count: " << m_transactionHistory.size() << " recorded\n";
    std::cout << "----------------------------------------\n";
}

std::string SavingsAccount::getAccountType() const {
    return "Savings Account";
}

double SavingsAccount::getInterestRate() const {
    return m_interestRate;
}

double SavingsAccount::getMinimumBalance() const {
    return m_minimumBalance;
}

AccountDTO SavingsAccount::toDTO() const {
    return AccountDTO{
        m_accountNumber,
        getAccountType(),
        m_holderName,
        m_balance,
        m_isActive,
        m_interestRate,
        m_minimumBalance,
        0.0,
        m_balance
    };
}

