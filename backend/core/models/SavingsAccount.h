#pragma once

#include "Account.h"

// ============================================================================
// SAVINGS ACCOUNT CLASS
// ============================================================================

class SavingsAccount : public Account {
private:
    double m_interestRate;
    double m_minimumBalance;

public:
    SavingsAccount(const std::string& accNum, 
                   const std::string& holder, 
                   double initialBalance, 
                   double interestRate = 4.0, 
                   double minBalance = 500.0);

    // Overridden withdraw matching the base signature with Transaction tracking
    void withdraw(double amount, 
                  const std::string& description = "Withdrawal",
                  TransactionType type = TransactionType::WITHDRAWAL,
                  const std::string& toAccount = "N/A") override;

    void displayAccountDetails() const override;
    std::string getAccountType() const override;
    AccountDTO toDTO() const override;

    double calculateInterest() const;
    double applyInterest();

    double getInterestRate() const;
    double getMinimumBalance() const;
};
