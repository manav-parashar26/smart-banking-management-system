#include "BankSystem.h"
#include <iostream>
#include <iomanip>
#include <sstream>

BankSystem::BankSystem() 
    : m_nextAccountNumber(1001), 
      m_nextRequestId(5001), 
      m_nextReversalId(9001) {}

bool BankSystem::registerCustomer(const Customer& customer) {
    return m_customerManager.registerCustomer(customer);
}

Customer* BankSystem::getCustomer(const std::string& customerId) {
    return m_customerManager.findCustomer(customerId);
}

CustomerManager& BankSystem::getCustomerManager() {
    return m_customerManager;
}

const CustomerManager& BankSystem::getCustomerManager() const {
    return m_customerManager;
}

std::string BankSystem::createSavingsAccount(const std::string& customerId, 
                                             double initialDeposit, 
                                             double interestRate, 
                                             double minBalance) {
    Customer* cust = getCustomer(customerId);
    if (!cust) {
        throw BankingException("Account creation failed: Customer " + customerId + " is not registered.");
    }

    std::string accNo = "SAV-" + std::to_string(m_nextAccountNumber++);
    auto acc = std::make_shared<SavingsAccount>(accNo, cust->getFullName(), initialDeposit, interestRate, minBalance);
    m_accounts[accNo] = acc;
    cust->linkAccount(accNo);
    return accNo;
}

std::string BankSystem::createCurrentAccount(const std::string& customerId, 
                                             double initialDeposit, 
                                             double overdraftLimit) {
    Customer* cust = getCustomer(customerId);
    if (!cust) {
        throw BankingException("Account creation failed: Customer " + customerId + " is not registered.");
    }

    std::string accNo = "CUR-" + std::to_string(m_nextAccountNumber++);
    auto acc = std::make_shared<CurrentAccount>(accNo, cust->getFullName(), initialDeposit, overdraftLimit);
    m_accounts[accNo] = acc;
    cust->linkAccount(accNo);
    return accNo;
}

std::shared_ptr<Account> BankSystem::getAccount(const std::string& accountNumber) {
    auto it = m_accounts.find(accountNumber);
    if (it != m_accounts.end()) {
        return it->second;
    }
    return nullptr;
}

void BankSystem::addAccountDirect(const std::shared_ptr<Account>& account) {
    if (account) {
        m_accounts[account->getAccountNumber()] = account;
    }
}

void BankSystem::executeDeposit(const std::string& accountNumber, double amount, const std::string& description) {
    auto acc = getAccount(accountNumber);
    if (!acc) throw EntityNotFoundException("Account " + accountNumber + " does not exist.");

    acc->deposit(amount, description);

    // Push to reversal stack (LIFO)
    const auto& history = acc->getTransactionHistory();
    std::string txnId = history.empty() ? "TXN-UNKNOWN" : history.back().getTransactionId();

    TransactionReversal rev;
    rev.reversalId = "REV-" + std::to_string(m_nextReversalId++);
    rev.originalTxnId = txnId;
    rev.sourceAccount = "N/A";
    rev.targetAccount = accountNumber;
    rev.amount = amount;
    rev.originalType = TransactionType::DEPOSIT;
    rev.reason = description;
    m_reversalStack.push(rev);
}

void BankSystem::executeWithdrawal(const std::string& accountNumber, double amount, const std::string& description) {
    auto acc = getAccount(accountNumber);
    if (!acc) throw EntityNotFoundException("Account " + accountNumber + " does not exist.");

    acc->withdraw(amount, description);

    // Push to reversal stack (LIFO)
    const auto& history = acc->getTransactionHistory();
    std::string txnId = history.empty() ? "TXN-UNKNOWN" : history.back().getTransactionId();

    TransactionReversal rev;
    rev.reversalId = "REV-" + std::to_string(m_nextReversalId++);
    rev.originalTxnId = txnId;
    rev.sourceAccount = accountNumber;
    rev.targetAccount = "N/A";
    rev.amount = amount;
    rev.originalType = TransactionType::WITHDRAWAL;
    rev.reason = description;
    m_reversalStack.push(rev);
}

void BankSystem::executeTransfer(const std::string& fromAccount, const std::string& toAccount, double amount, const std::string& description) {
    if (fromAccount == toAccount) {
        throw InvalidAmountException("Self-transfer is not permitted: Source and destination must be different.");
    }
    auto src = getAccount(fromAccount);
    auto dest = getAccount(toAccount);
    if (!src) throw EntityNotFoundException("Source account " + fromAccount + " does not exist.");
    if (!dest) throw EntityNotFoundException("Destination account " + toAccount + " does not exist.");

    src->transferTo(*dest, amount, description);

    // Push to reversal stack (LIFO)
    const auto& history = src->getTransactionHistory();
    std::string txnId = history.empty() ? "TXN-UNKNOWN" : history.back().getTransactionId();

    TransactionReversal rev;
    rev.reversalId = "REV-" + std::to_string(m_nextReversalId++);
    rev.originalTxnId = txnId;
    rev.sourceAccount = fromAccount;
    rev.targetAccount = toAccount;
    rev.amount = amount;
    rev.originalType = TransactionType::TRANSFER_OUT;
    rev.reason = description;
    m_reversalStack.push(rev);
}

std::string BankSystem::submitBankingRequest(const std::string& customerId, 
                                             const std::string& accountNumber, 
                                             RequestType type, 
                                             double amount, 
                                             const std::string& notes) {
    std::string reqId = "REQ-" + std::to_string(m_nextRequestId++);
    BankingRequest req{reqId, customerId, accountNumber, type, amount, RequestStatus::PENDING, notes};
    m_pendingRequests.push(req); // O(1) enqueue
    return reqId;
}

bool BankSystem::processNextBankingRequest(bool approve, std::string& outMessage) {
    if (m_pendingRequests.empty()) {
        outMessage = "Queue Empty: No pending banking requests.";
        return false;
    }

    // Dequeue from front of FIFO queue
    BankingRequest req = m_pendingRequests.front(); // O(1)
    m_pendingRequests.pop();                        // O(1)

    req.status = approve ? RequestStatus::APPROVED : RequestStatus::REJECTED;

    std::ostringstream oss;
    if (approve) {
        oss << "APPROVED Request [" << req.requestId << "] (" << BankingRequest::typeToString(req.type) << ") for " << req.customerId;
        auto acc = getAccount(req.accountNumber);
        if (acc) {
            if (req.type == RequestType::ACCOUNT_UNBLOCK) {
                acc->activateAccount();
                oss << " -> Account " << req.accountNumber << " unblocked.";
            } else if (req.type == RequestType::LOAN_APPLICATION) {
                acc->deposit(req.requestedAmount, "Approved Loan Disbursal (" + req.requestId + ")");
                oss << " -> Disbursed $" << std::fixed << std::setprecision(2) << req.requestedAmount << " to " << req.accountNumber;
            }
        }
    } else {
        oss << "REJECTED Request [" << req.requestId << "] (" << BankingRequest::typeToString(req.type) << ") for " << req.customerId;
    }

    outMessage = oss.str();
    return true;
}

size_t BankSystem::getPendingRequestCount() const {
    return m_pendingRequests.size();
}

void BankSystem::displayPendingRequests() const {
    std::cout << "\n==============================================================================\n";
    std::cout << "                 PENDING BANKING REQUEST QUEUE (FIFO) [" << m_pendingRequests.size() << "]\n";
    std::cout << "==============================================================================\n";
    if (m_pendingRequests.empty()) {
        std::cout << "No requests pending in queue.\n";
    } else {
        // Copy queue to iterate nondestructively
        auto tempQueue = m_pendingRequests;
        int order = 1;
        while (!tempQueue.empty()) {
            std::cout << "Priority #" << order++ << " -> ";
            tempQueue.front().display();
            tempQueue.pop();
        }
    }
    std::cout << "==============================================================================\n\n";
}

bool BankSystem::undoLastTransaction(std::string& outMessage) {
    if (m_reversalStack.empty()) {
        outMessage = "Reversal Stack Empty: No transactions available to undo.";
        return false;
    }

    // Pop the most recent transaction from LIFO stack
    TransactionReversal rev = m_reversalStack.top(); // O(1)
    m_reversalStack.pop();                           // O(1)

    std::ostringstream oss;
    oss << "Successfully Rolled Back [" << rev.originalTxnId << "] via Reversal " << rev.reversalId << ": ";

    if (rev.originalType == TransactionType::DEPOSIT) {
        auto acc = getAccount(rev.targetAccount);
        if (acc) {
            acc->withdraw(rev.amount, "Reversal: Undo Deposit " + rev.originalTxnId);
            oss << "Debited $" << rev.amount << " from " << rev.targetAccount;
        }
    } else if (rev.originalType == TransactionType::WITHDRAWAL) {
        auto acc = getAccount(rev.sourceAccount);
        if (acc) {
            acc->deposit(rev.amount, "Reversal: Undo Withdrawal " + rev.originalTxnId);
            oss << "Re-credited $" << rev.amount << " back to " << rev.sourceAccount;
        }
    } else if (rev.originalType == TransactionType::TRANSFER_OUT) {
        auto src = getAccount(rev.sourceAccount);
        auto dest = getAccount(rev.targetAccount);
        if (src && dest) {
            dest->withdraw(rev.amount, "Reversal: Transfer Refund to " + rev.sourceAccount);
            src->deposit(rev.amount, "Reversal: Transfer Recovered from " + rev.targetAccount);
            oss << "Restored $" << rev.amount << " from " << rev.targetAccount << " back to " << rev.sourceAccount;
        }
    }

    outMessage = oss.str();
    return true;
}

size_t BankSystem::getReversalStackSize() const {
    return m_reversalStack.size();
}

void BankSystem::displayReversalStack() const {
    std::cout << "\n==============================================================================\n";
    std::cout << "               TRANSACTION REVERSAL / UNDO STACK (LIFO) [" << m_reversalStack.size() << "]\n";
    std::cout << "==============================================================================\n";
    if (m_reversalStack.empty()) {
        std::cout << "No undoable transactions on stack.\n";
    } else {
        auto tempStack = m_reversalStack;
        int depth = 1;
        while (!tempStack.empty()) {
            std::cout << "Top -" << (depth - 1) << " -> ";
            tempStack.top().display();
            tempStack.pop();
            depth++;
        }
    }
    std::cout << "==============================================================================\n\n";
}

void BankSystem::displayAllAccounts() const {
    std::cout << "\n==============================================================================\n";
    std::cout << "                       ALL REGISTERED BANK ACCOUNTS (" << m_accounts.size() << ")\n";
    std::cout << "==============================================================================\n";
    std::cout << std::left 
              << std::setw(12) << "Account No" << " | "
              << std::setw(16) << "Type" << " | "
              << std::setw(20) << "Holder Name" << " | "
              << std::setw(12) << "Balance" << " | "
              << "Status\n";
    std::cout << "------------------------------------------------------------------------------\n";

    for (const auto& pair : m_accounts) {
        const auto& acc = pair.second;
        std::cout << std::left
                  << std::setw(12) << acc->getAccountNumber() << " | "
                  << std::setw(16) << acc->getAccountType() << " | "
                  << std::setw(20) << acc->getHolderName() << " | $"
                  << std::right << std::setw(10) << std::fixed << std::setprecision(2) << acc->getBalance() << " | "
                  << std::left
                  << (acc->isActive() ? "ACTIVE" : "BLOCKED") << "\n";
    }
    std::cout << "==============================================================================\n\n";
}

const std::unordered_map<std::string, std::shared_ptr<Account>>& BankSystem::getAllAccounts() const {
    return m_accounts;
}

bool BankSystem::blockAccount(const std::string& accountNumber) {
    auto acc = getAccount(accountNumber);
    if (!acc) return false;
    acc->blockAccount();
    return true;
}

bool BankSystem::activateAccount(const std::string& accountNumber) {
    auto acc = getAccount(accountNumber);
    if (!acc) return false;
    acc->activateAccount();
    return true;
}

std::vector<BankingRequest> BankSystem::getPendingRequestsList() const {
    std::vector<BankingRequest> list;
    auto tempQueue = m_pendingRequests;
    while (!tempQueue.empty()) {
        list.push_back(tempQueue.front());
        tempQueue.pop();
    }
    return list;
}

std::vector<TransactionReversal> BankSystem::getReversalStackList() const {
    std::vector<TransactionReversal> list;
    auto tempStack = m_reversalStack;
    while (!tempStack.empty()) {
        list.push_back(tempStack.top());
        tempStack.pop();
    }
    return list;
}

std::vector<std::shared_ptr<Account>> BankSystem::getAllAccountsList() const {
    std::vector<std::shared_ptr<Account>> list;
    for (const auto& pair : m_accounts) {
        list.push_back(pair.second);
    }
    return list;
}

std::vector<Transaction> BankSystem::getAllTransactionsLedger() const {
    std::vector<Transaction> all;
    for (const auto& pair : m_accounts) {
        const auto& history = pair.second->getTransactionHistory();
        all.insert(all.end(), history.begin(), history.end());
    }
    return all;
}

AccountDTO BankSystem::getAccountDTO(const std::string& accountNumber) {
    auto acc = getAccount(accountNumber);
    if (!acc) throw EntityNotFoundException("Account " + accountNumber + " not found.");
    return acc->toDTO();
}

std::vector<AccountDTO> BankSystem::getAllAccountDTOs() const {
    std::vector<AccountDTO> dtos;
    for (const auto& pair : m_accounts) {
        dtos.push_back(pair.second->toDTO());
    }
    return dtos;
}

std::vector<AccountDTO> BankSystem::getCustomerAccountDTOs(const std::string& customerId) const {
    const Customer* cust = m_customerManager.findCustomer(customerId);
    if (!cust) throw EntityNotFoundException("Customer " + customerId + " not found.");
    std::vector<AccountDTO> dtos;
    for (const auto& accNo : cust->getLinkedAccountNumbers()) {
        auto it = m_accounts.find(accNo);
        if (it != m_accounts.end()) {
            dtos.push_back(it->second->toDTO());
        }
    }
    return dtos;
}

CustomerDTO BankSystem::getCustomerDTO(const std::string& customerId) {
    Customer* cust = getCustomer(customerId);
    if (!cust) throw EntityNotFoundException("Customer " + customerId + " not found.");
    return cust->toDTO();
}

std::vector<CustomerDTO> BankSystem::getAllCustomerDTOs() const {
    std::vector<CustomerDTO> dtos;
    for (const auto& pair : m_customerManager.getAllCustomers()) {
        dtos.push_back(pair.second.toDTO());
    }
    return dtos;
}

std::vector<TransactionDTO> BankSystem::getAccountTransactionDTOs(const std::string& accountNumber) const {
    auto it = m_accounts.find(accountNumber);
    if (it == m_accounts.end()) throw EntityNotFoundException("Account " + accountNumber + " not found.");
    std::vector<TransactionDTO> dtos;
    for (const auto& t : it->second->getTransactionHistory()) {
        dtos.push_back(t.toDTO());
    }
    return dtos;
}

std::vector<TransactionDTO> BankSystem::getAllTransactionDTOs() const {
    std::vector<TransactionDTO> dtos;
    for (const auto& t : getAllTransactionsLedger()) {
        dtos.push_back(t.toDTO());
    }
    return dtos;
}

AccountDTO BankSystem::depositMoneyDTO(const DepositWithdrawDTO& request) {
    executeDeposit(request.accountNumber, request.amount, request.description.empty() ? "Deposit" : request.description);
    return getAccount(request.accountNumber)->toDTO();
}

AccountDTO BankSystem::withdrawMoneyDTO(const DepositWithdrawDTO& request) {
    executeWithdrawal(request.accountNumber, request.amount, request.description.empty() ? "Withdrawal" : request.description);
    return getAccount(request.accountNumber)->toDTO();
}

TransactionDTO BankSystem::transferMoneyDTO(const TransferRequestDTO& request) {
    executeTransfer(request.fromAccount, request.toAccount, request.amount, request.description.empty() ? "Transfer" : request.description);
    auto acc = getAccount(request.fromAccount);
    return acc->getTransactionHistory().back().toDTO();
}

AuthResponseDTO BankSystem::authenticateCustomer(const AuthRequestDTO& request) {
    Customer* cust = getCustomer(request.customerId);
    if (!cust) {
        throw EntityNotFoundException("Customer " + request.customerId + " does not exist.");
    }
    if (!cust->verifyPin(request.pin)) {
        throw AuthenticationException("Invalid PIN provided for customer " + request.customerId);
    }
    return AuthResponseDTO{true, cust->getCustomerId(), cust->getFullName(), "tok_" + cust->getCustomerId() + "_session", "Login successful"};
}

AuthResponseDTO BankSystem::registerCustomerDTO(const CustomerDTO& customerDto, const std::string& pin) {
    std::string cid = customerDto.customerId;
    if (cid.empty()) {
        cid = "CUST-" + std::to_string(101 + m_customerManager.getCustomerCount());
    }
    Customer c(cid, customerDto.fullName, customerDto.email, customerDto.phone, pin);
    if (!registerCustomer(c)) {
        throw BankingException("Customer " + cid + " is already registered.", 409, "CONFLICT");
    }
    // Auto-create initial default Savings Account with $500 minimum balance
    createSavingsAccount(cid, 500.00, 4.0, 500.00);

    return AuthResponseDTO{true, cid, c.getFullName(), "tok_" + cid + "_session", "Customer registered successfully"};
}


