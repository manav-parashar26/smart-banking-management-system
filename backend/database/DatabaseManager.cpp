#include "DatabaseManager.h"
#include "MySqlSchema.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <set>

void CustomerAccountJoinRecord::display() const {
    std::cout << std::left 
              << std::setw(10) << customerId << " | "
              << std::setw(16) << customerName << " | "
              << std::setw(12) << accountNumber << " | "
              << std::setw(18) << accountType << " | $"
              << std::right << std::setw(10) << std::fixed << std::setprecision(2) << balance << " | "
              << (isActive ? "ACTIVE" : "BLOCKED") << "\n";
}

bool DatabaseManager::generateMySQLSchemaScript(const std::string& outputPath) {
    std::ofstream out(outputPath);
    if (!out.is_open()) return false;
    out << MySqlSchema::getCompleteDDL();
    out.close();
    return true;
}

bool DatabaseManager::exportToMySQLDump(const BankSystem& bank, const std::string& outputPath) {
    std::ofstream out(outputPath);
    if (!out.is_open()) return false;

    // Header & Schema
    out << "-- ============================================================================\n";
    out << "-- SMART BANKING MANAGEMENT SYSTEM - COMPLETE MYSQL DATA DUMP\n";
    out << "-- Generated automatically by C++ Database Manager\n";
    out << "-- ============================================================================\n\n";
    out << MySqlSchema::getCompleteDDL();
    out << "\n-- ============================================================================\n";
    out << "-- TABLE DATA INSERTS (ACID Relational Representation)\n";
    out << "-- ============================================================================\n\n";

    // 1. Insert Customers
    out << "-- Dumping data for table `customers`\n";
    const auto& customers = bank.getCustomerManager().getAllCustomers();
    if (!customers.empty()) {
        out << "INSERT INTO customers (customer_id, full_name, email, phone, pin) VALUES\n";
        size_t cIdx = 0;
        for (const auto& pair : customers) {
            const Customer& c = pair.second;
            out << "('" << c.getCustomerId() << "', "
                << "'" << c.getFullName() << "', "
                << "'" << c.getEmail() << "', "
                << "'" << c.getPhone() << "', "
                << "'" << c.getPinHash() << "')"
                << (cIdx + 1 < customers.size() ? ",\n" : ";\n\n");
            cIdx++;
        }
    }

    // 2. Insert Accounts (With Foreign Key customer_id mapping)
    out << "-- Dumping data for table `accounts`\n";
    const auto& accounts = bank.getAllAccounts();
    if (!accounts.empty()) {
        out << "INSERT INTO accounts (account_number, customer_id, account_type, holder_name, balance, is_active, interest_rate, minimum_balance, overdraft_limit) VALUES\n";
        size_t aIdx = 0;
        for (const auto& pair : accounts) {
            const auto& acc = pair.second;
            // Find parent customer ID
            std::string custId = "CUST-UNKNOWN";
            for (const auto& cPair : customers) {
                if (cPair.second.hasAccount(acc->getAccountNumber())) {
                    custId = cPair.second.getCustomerId();
                    break;
                }
            }

            double intRate = 0.0, minBal = 0.0, overLimit = 0.0;
            if (acc->getAccountType() == "Savings Account") {
                auto sav = std::dynamic_pointer_cast<SavingsAccount>(acc);
                if (sav) { intRate = sav->getInterestRate(); minBal = sav->getMinimumBalance(); }
            } else if (acc->getAccountType() == "Current Account") {
                auto cur = std::dynamic_pointer_cast<CurrentAccount>(acc);
                if (cur) { overLimit = cur->getOverdraftLimit(); }
            }

            out << "('" << acc->getAccountNumber() << "', "
                << "'" << custId << "', "
                << "'" << acc->getAccountType() << "', "
                << "'" << acc->getHolderName() << "', "
                << std::fixed << std::setprecision(2) << acc->getBalance() << ", "
                << (acc->isActive() ? "TRUE" : "FALSE") << ", "
                << intRate << ", "
                << minBal << ", "
                << overLimit << ")"
                << (aIdx + 1 < accounts.size() ? ",\n" : ";\n\n");
            aIdx++;
        }
    }

    // 3. Insert Transactions (Deduplicated)
    out << "-- Dumping data for table `transactions`\n";
    auto txns = bank.getAllTransactionsLedger();
    std::set<std::string> writtenTxns;
    std::vector<Transaction> uniqueTxns;
    for (const auto& t : txns) {
        if (writtenTxns.insert(t.getTransactionId()).second) {
            uniqueTxns.push_back(t);
        }
    }

    if (!uniqueTxns.empty()) {
        out << "INSERT INTO transactions (transaction_id, from_account, to_account, amount, timestamp, type_id, type_name, status_id, status_name, description) VALUES\n";
        for (size_t i = 0; i < uniqueTxns.size(); ++i) {
            const auto& t = uniqueTxns[i];
            out << "('" << t.getTransactionId() << "', "
                << "'" << t.getFromAccount() << "', "
                << "'" << t.getToAccount() << "', "
                << std::fixed << std::setprecision(2) << t.getAmount() << ", "
                << "'" << t.getTimestamp() << "', "
                << static_cast<int>(t.getType()) << ", "
                << "'" << Transaction::typeToString(t.getType()) << "', "
                << static_cast<int>(t.getStatus()) << ", "
                << "'" << Transaction::statusToString(t.getStatus()) << "', "
                << "'" << t.getDescription() << "')"
                << (i + 1 < uniqueTxns.size() ? ",\n" : ";\n\n");
        }
    }

    // 4. Insert Pending Requests
    out << "-- Dumping data for table `pending_requests`\n";
    auto reqs = bank.getPendingRequestsList();
    if (!reqs.empty()) {
        out << "INSERT INTO pending_requests (request_id, customer_id, account_number, type, amount, status, notes) VALUES\n";
        for (size_t i = 0; i < reqs.size(); ++i) {
            const auto& r = reqs[i];
            out << "('" << r.requestId << "', "
                << "'" << r.customerId << "', "
                << "'" << r.accountNumber << "', "
                << "'" << BankingRequest::typeToString(r.type) << "', "
                << std::fixed << std::setprecision(2) << r.requestedAmount << ", "
                << "'" << BankingRequest::statusToString(r.status) << "', "
                << "'" << r.notes << "')"
                << (i + 1 < reqs.size() ? ",\n" : ";\n\n");
        }
    }

    out.close();
    return true;
}

std::vector<CustomerAccountJoinRecord> DatabaseManager::executeCustomerAccountJoin(const BankSystem& bank) {
    std::vector<CustomerAccountJoinRecord> records;

    // Relational INNER JOIN simulation: customers c INNER JOIN accounts a ON c.customer_id = a.customer_id
    for (const auto& cPair : bank.getCustomerManager().getAllCustomers()) {
        const Customer& c = cPair.second;
        for (const auto& accNo : c.getLinkedAccountNumbers()) {
            auto acc = const_cast<BankSystem&>(bank).getAccount(accNo);
            if (acc) {
                CustomerAccountJoinRecord rec;
                rec.customerId = c.getCustomerId();
                rec.customerName = c.getFullName();
                rec.email = c.getEmail();
                rec.accountNumber = acc->getAccountNumber();
                rec.accountType = acc->getAccountType();
                rec.balance = acc->getBalance();
                rec.isActive = acc->isActive();
                records.push_back(rec);
            }
        }
    }

    return records;
}

std::vector<AccountDTO> DatabaseManager::queryAccountsByBalanceThreshold(const BankSystem& bank, double minBalance) {
    std::vector<AccountDTO> matching;
    auto all = bank.getAllAccountDTOs();

    // SQL Simulation: SELECT * FROM accounts WHERE balance >= minBalance ORDER BY balance DESC
    for (const auto& a : all) {
        if (a.balance >= minBalance) {
            matching.push_back(a);
        }
    }

    std::sort(matching.begin(), matching.end(), [](const AccountDTO& x, const AccountDTO& y) {
        return x.balance > y.balance;
    });

    return matching;
}

bool DatabaseManager::verifyReferentialIntegrity(const BankSystem& bank, std::vector<std::string>& outViolations) {
    outViolations.clear();
    const auto& customers = bank.getCustomerManager().getAllCustomers();
    const auto& accounts = bank.getAllAccounts();

    // Check 1: Every account must have a valid customer owner (Foreign Key: accounts.customer_id -> customers.customer_id)
    for (const auto& aPair : accounts) {
        const std::string& accNo = aPair.first;
        bool hasOwner = false;
        for (const auto& cPair : customers) {
            if (cPair.second.hasAccount(accNo)) {
                hasOwner = true;
                break;
            }
        }
        if (!hasOwner) {
            outViolations.push_back("FK Violation: Account " + accNo + " is orphaned (no associated customer_id)");
        }
    }

    // Check 2: Every customer's linked account must exist in the accounts table
    for (const auto& cPair : customers) {
        const Customer& c = cPair.second;
        for (const auto& accNo : c.getLinkedAccountNumbers()) {
            if (accounts.find(accNo) == accounts.end()) {
                outViolations.push_back("FK Violation: Customer " + c.getCustomerId() + " references non-existent account " + accNo);
            }
        }
    }

    return outViolations.empty();
}
