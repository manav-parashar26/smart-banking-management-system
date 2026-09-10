#pragma once

#include "Account.h"

// ============================================================================
// CURRENT ACCOUNT CLASS
// ============================================================================

class CurrentAccount : public Account {
private:
    double m_overdraftLimit;

public:
    CurrentAccount(const std::string& accNum, 
                   const std::string& holder, 
                   double initialBalance, 
                   double overdraftLimit = 1000.0);

    // Overridden withdraw matching base signature with Transaction tracking
    void withdraw(double amount, 
                  const std::string& description = "Withdrawal",
                  TransactionType type = TransactionType::WITHDRAWAL,
                  const std::string& toAccount = "N/A") override;

    void displayAccountDetails() const override;
    std::string getAccountType() const override;
    AccountDTO toDTO() const override;

    double getOverdraftLimit() const;
    double getAvailableCredit() const;
};
