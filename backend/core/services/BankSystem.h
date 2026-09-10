#pragma once

#include <string>
#include <unordered_map>
#include <queue>
#include <stack>
#include <memory>
#include <vector>

#include "Account.h"
#include "SavingsAccount.h"
#include "CurrentAccount.h"
#include "Customer.h"
#include "CustomerManager.h"
#include "BankingRequest.h"
#include "TransactionReversal.h"
#include "BankingDTOs.h"

// ============================================================================
// CENTRAL BANK MANAGEMENT SYSTEM
// Thread-Safe Banking Service Engine with DTO Contracts & Rich Operations
// ============================================================================

class BankSystem {
private:
    CustomerManager m_customerManager;
    std::unordered_map<std::string, std::shared_ptr<Account>> m_accounts;
    std::queue<BankingRequest> m_pendingRequests;
    std::stack<TransactionReversal> m_reversalStack;

    uint64_t m_nextAccountNumber;
    uint64_t m_nextRequestId;
    uint64_t m_nextReversalId;

public:
    BankSystem();

    // Customer operations
    bool registerCustomer(const Customer& customer);
    Customer* getCustomer(const std::string& customerId);
    CustomerManager& getCustomerManager();
    const CustomerManager& getCustomerManager() const;

    // Account creation & retrieval
    std::string createSavingsAccount(const std::string& customerId, 
                                     double initialDeposit, 
                                     double interestRate = 4.0, 
                                     double minBalance = 500.0);

    std::string createCurrentAccount(const std::string& customerId, 
                                     double initialDeposit, 
                                     double overdraftLimit = 1000.0);

    std::shared_ptr<Account> getAccount(const std::string& accountNumber);
    void addAccountDirect(const std::shared_ptr<Account>& account);

    // Core Banking Operations
    void executeDeposit(const std::string& accountNumber, double amount, const std::string& description = "Deposit");
    void executeWithdrawal(const std::string& accountNumber, double amount, const std::string& description = "Withdrawal");
    void executeTransfer(const std::string& fromAccount, const std::string& toAccount, double amount, const std::string& description = "Transfer");

    // Account State Management
    bool blockAccount(const std::string& accountNumber);
    bool activateAccount(const std::string& accountNumber);

    // ========================================================================
    // DTO-DRIVEN BANKING SERVICE INTERFACE (For CLI, REST API & Web Clients)
    // ========================================================================
    AccountDTO getAccountDTO(const std::string& accountNumber);
    std::vector<AccountDTO> getAllAccountDTOs() const;
    std::vector<AccountDTO> getCustomerAccountDTOs(const std::string& customerId) const;

    CustomerDTO getCustomerDTO(const std::string& customerId);
    std::vector<CustomerDTO> getAllCustomerDTOs() const;

    std::vector<TransactionDTO> getAccountTransactionDTOs(const std::string& accountNumber) const;
    std::vector<TransactionDTO> getAllTransactionDTOs() const;

    AccountDTO depositMoneyDTO(const DepositWithdrawDTO& request);
    AccountDTO withdrawMoneyDTO(const DepositWithdrawDTO& request);
    TransactionDTO transferMoneyDTO(const TransferRequestDTO& request);

    AuthResponseDTO authenticateCustomer(const AuthRequestDTO& request);
    AuthResponseDTO registerCustomerDTO(const CustomerDTO& customerDto, const std::string& pin);

    // Queue DSA: Pending Request Processing (FIFO)
    std::string submitBankingRequest(const std::string& customerId, 
                                     const std::string& accountNumber, 
                                     RequestType type, 
                                     double amount, 
                                     const std::string& notes);
    bool processNextBankingRequest(bool approve, std::string& outMessage);
    size_t getPendingRequestCount() const;
    std::vector<BankingRequest> getPendingRequestsList() const;
    void displayPendingRequests() const;

    // Stack DSA: Transaction Reversal / Undo Simulation (LIFO)
    bool undoLastTransaction(std::string& outMessage);
    size_t getReversalStackSize() const;
    std::vector<TransactionReversal> getReversalStackList() const;
    void displayReversalStack() const;

    // Data Accessors & Reporting
    std::vector<std::shared_ptr<Account>> getAllAccountsList() const;
    std::vector<Transaction> getAllTransactionsLedger() const;
    void displayAllAccounts() const;
    const std::unordered_map<std::string, std::shared_ptr<Account>>& getAllAccounts() const;
};
