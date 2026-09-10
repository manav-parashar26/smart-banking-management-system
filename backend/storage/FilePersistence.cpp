#include "FilePersistence.h"
#include "SecurityHelper.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <set>

static std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

static bool copyFile(const std::string& src, const std::string& dst) {
    std::ifstream in(src, std::ios::binary);
    if (!in.is_open()) return false;
    std::ofstream out(dst, std::ios::binary);
    if (!out.is_open()) return false;
    out << in.rdbuf();
    return out.good();
}

bool FilePersistence::safeParseDouble(const std::string& str, double& outVal) {
    if (str.empty()) return false;
    try {
        size_t idx = 0;
        outVal = std::stod(str, &idx);
        return idx == str.length();
    } catch (...) {
        return false;
    }
}

bool FilePersistence::safeParseInt(const std::string& str, int& outVal) {
    if (str.empty()) return false;
    try {
        size_t idx = 0;
        outVal = std::stoi(str, &idx);
        return idx == str.length();
    } catch (...) {
        return false;
    }
}

bool FilePersistence::saveBankState(const BankSystem& bank, const std::string& directory) {
    PersistenceResult res;
    return saveBankStateAtomic(bank, directory, res);
}

bool FilePersistence::saveBankStateAtomic(const BankSystem& bank, const std::string& directory, PersistenceResult& outResult) {
    outResult = PersistenceResult();

    std::string custFinal = directory + "/customers.txt";
    std::string custTmp   = directory + "/customers.txt.tmp";
    std::string custBak   = directory + "/customers.txt.bak";

    std::string accFinal  = directory + "/accounts.txt";
    std::string accTmp    = directory + "/accounts.txt.tmp";
    std::string accBak    = directory + "/accounts.txt.bak";

    std::string txnFinal  = directory + "/transactions.txt";
    std::string txnTmp    = directory + "/transactions.txt.tmp";
    std::string txnBak    = directory + "/transactions.txt.bak";

    // ------------------------------------------------------------------------
    // Step 1: Write Customers to .tmp
    // ------------------------------------------------------------------------
    {
        std::ofstream outCust(custTmp);
        if (!outCust.is_open()) {
            outResult.success = false;
            outResult.errorMessage = "Failed to open temporary customer file: " + custTmp;
            return false;
        }

        outCust << "# SMART BANKING SYSTEM - CUSTOMER REGISTRY (VERSION 2.0 - SECURE HASH)\n";
        outCust << "# CustomerId|FullName|Email|Phone|PINHash|LinkedAccounts\n";

        for (const auto& pair : bank.getCustomerManager().getAllCustomers()) {
            const Customer& c = pair.second;
            outCust << c.getCustomerId() << "|"
                    << c.getFullName() << "|"
                    << c.getEmail() << "|"
                    << c.getPhone() << "|"
                    << c.getPinHash() << "|";
            
            const auto& accounts = c.getLinkedAccountNumbers();
            for (size_t i = 0; i < accounts.size(); ++i) {
                outCust << accounts[i] << (i + 1 < accounts.size() ? "," : "");
            }
            outCust << "\n";
            outResult.recordsSaved++;
        }
        outCust.close();
    }

    // ------------------------------------------------------------------------
    // Step 2: Write Accounts to .tmp
    // ------------------------------------------------------------------------
    {
        std::ofstream outAcc(accTmp);
        if (!outAcc.is_open()) {
            std::remove(custTmp.c_str());
            outResult.success = false;
            outResult.errorMessage = "Failed to open temporary account file: " + accTmp;
            return false;
        }

        outAcc << "# SMART BANKING SYSTEM - ACCOUNT REGISTRY (VERSION 2.0)\n";
        outAcc << "# AccountNo|AccountType|HolderName|Balance|IsActive|Param1|Param2\n";

        for (const auto& pair : bank.getAllAccounts()) {
            const auto& acc = pair.second;
            outAcc << acc->getAccountNumber() << "|"
                   << acc->getAccountType() << "|"
                   << acc->getHolderName() << "|"
                   << acc->getBalance() << "|"
                   << (acc->isActive() ? "1" : "0") << "|";

            if (acc->getAccountType() == "Savings Account") {
                auto sav = std::dynamic_pointer_cast<SavingsAccount>(acc);
                outAcc << (sav ? sav->getInterestRate() : 4.0) << "|"
                       << (sav ? sav->getMinimumBalance() : 500.0) << "\n";
            } else if (acc->getAccountType() == "Current Account") {
                auto cur = std::dynamic_pointer_cast<CurrentAccount>(acc);
                outAcc << (cur ? cur->getOverdraftLimit() : 1000.0) << "|0.0\n";
            } else {
                outAcc << "0.0|0.0\n";
            }
            outResult.recordsSaved++;
        }
        outAcc.close();
    }

    // ------------------------------------------------------------------------
    // Step 3: Write Transactions to .tmp (Deduplicated via std::set)
    // ------------------------------------------------------------------------
    {
        std::ofstream outTxn(txnTmp);
        if (!outTxn.is_open()) {
            std::remove(custTmp.c_str());
            std::remove(accTmp.c_str());
            outResult.success = false;
            outResult.errorMessage = "Failed to open temporary transaction file: " + txnTmp;
            return false;
        }

        outTxn << "# SMART BANKING SYSTEM - TRANSACTION AUDIT LEDGER (VERSION 2.0)\n";
        outTxn << "# TxnId|FromAcc|ToAcc|Amount|Timestamp|Type|Status|Description\n";

        std::set<std::string> savedTxnIds;
        for (const auto& pair : bank.getAllAccounts()) {
            for (const auto& t : pair.second->getTransactionHistory()) {
                if (savedTxnIds.insert(t.getTransactionId()).second) {
                    outTxn << t.getTransactionId() << "|"
                           << t.getFromAccount() << "|"
                           << t.getToAccount() << "|"
                           << t.getAmount() << "|"
                           << t.getTimestamp() << "|"
                           << static_cast<int>(t.getType()) << "|"
                           << static_cast<int>(t.getStatus()) << "|"
                           << t.getDescription() << "\n";
                    outResult.recordsSaved++;
                }
            }
        }
        outTxn.close();
    }

    // ------------------------------------------------------------------------
    // Step 4: Atomic Commit (Backup Existing -> Replace with .tmp)
    // ------------------------------------------------------------------------
    // Create .bak copies
    copyFile(custFinal, custBak);
    copyFile(accFinal, accBak);
    copyFile(txnFinal, txnBak);

    // Atomic replace on disk
    std::remove(custFinal.c_str());
    std::rename(custTmp.c_str(), custFinal.c_str());

    std::remove(accFinal.c_str());
    std::rename(accTmp.c_str(), accFinal.c_str());

    std::remove(txnFinal.c_str());
    std::rename(txnTmp.c_str(), txnFinal.c_str());

    outResult.success = true;
    return true;
}

bool FilePersistence::loadBankState(BankSystem& bank, const std::string& directory) {
    PersistenceResult res;
    return loadBankStateResilient(bank, directory, res);
}

bool FilePersistence::loadBankStateResilient(BankSystem& bank, const std::string& directory, PersistenceResult& outResult) {
    outResult = PersistenceResult();

    // ------------------------------------------------------------------------
    // 1. Load Customers
    // ------------------------------------------------------------------------
    std::string custFile = directory + "/customers.txt";
    std::ifstream inCust(custFile);
    if (!inCust.is_open()) {
        // Attempt recovery from backup
        std::string custBak = directory + "/customers.txt.bak";
        std::ifstream inCustBak(custBak);
        if (inCustBak.is_open()) {
            outResult.addWarning("Primary customer file missing. Recovering from backup: " + custBak);
            inCust.swap(inCustBak);
        } else {
            outResult.success = false;
            outResult.errorMessage = "Could not open customer file: " + custFile;
            return false;
        }
    }

    std::string line;
    size_t lineNum = 0;
    bool anyMigrated = false;
    while (std::getline(inCust, line)) {
        lineNum++;
        if (line.empty() || line[0] == '#') continue; // Skip comments & blanks

        auto tokens = split(line, '|');
        if (tokens.size() < 5) {
            outResult.addWarning("customers.txt (Line " + std::to_string(lineNum) + "): Skipped malformed record (insufficient tokens: " + std::to_string(tokens.size()) + ")");
            continue;
        }

        std::string pinField = tokens[4];
        if (!SecurityHelper::isHashed(pinField)) {
            // Transparent migration: upgrade legacy plaintext PIN to PBKDF2 salted hash
            pinField = SecurityHelper::hashPin(pinField);
            anyMigrated = true;
            outResult.addWarning("Customer " + tokens[0] + ": Transparently migrated legacy plaintext PIN to PBKDF2-HMAC-SHA256 salted hash.");
        }

        Customer c(tokens[0], tokens[1], tokens[2], tokens[3], pinField, /*isAlreadyHashed=*/true);
        if (tokens.size() >= 6 && !tokens[5].empty()) {
            auto accs = split(tokens[5], ',');
            for (const auto& a : accs) {
                if (!a.empty()) c.linkAccount(a);
            }
        }
        bank.registerCustomer(c);
        outResult.recordsLoaded++;
    }
    inCust.close();

    // ------------------------------------------------------------------------
    // 2. Load Accounts
    // ------------------------------------------------------------------------
    std::string accFile = directory + "/accounts.txt";
    std::ifstream inAcc(accFile);
    if (!inAcc.is_open()) {
        std::string accBak = directory + "/accounts.txt.bak";
        std::ifstream inAccBak(accBak);
        if (inAccBak.is_open()) {
            outResult.addWarning("Primary account file missing. Recovering from backup: " + accBak);
            inAcc.swap(inAccBak);
        } else {
            outResult.success = false;
            outResult.errorMessage = "Could not open account file: " + accFile;
            return false;
        }
    }

    lineNum = 0;
    while (std::getline(inAcc, line)) {
        lineNum++;
        if (line.empty() || line[0] == '#') continue;

        auto tokens = split(line, '|');
        if (tokens.size() < 7) {
            outResult.addWarning("accounts.txt (Line " + std::to_string(lineNum) + "): Skipped malformed record (expected 7 fields, got " + std::to_string(tokens.size()) + ")");
            continue;
        }

        std::string accNo  = tokens[0];
        std::string type   = tokens[1];
        std::string holder = tokens[2];

        double balance = 0.0;
        if (!safeParseDouble(tokens[3], balance)) {
            outResult.addWarning("accounts.txt (Line " + std::to_string(lineNum) + "): Skipped account " + accNo + " due to non-numeric balance '" + tokens[3] + "'");
            continue;
        }

        bool isActive = (tokens[4] == "1");

        double param1 = 0.0, param2 = 0.0;
        if (!safeParseDouble(tokens[5], param1) || !safeParseDouble(tokens[6], param2)) {
            outResult.addWarning("accounts.txt (Line " + std::to_string(lineNum) + "): Warning on parameters for " + accNo + "; defaulting to standard values");
            param1 = 4.0;
            param2 = 500.0;
        }

        std::shared_ptr<Account> acc;
        if (type == "Savings Account") {
            acc = std::make_shared<SavingsAccount>(accNo, holder, balance >= param2 ? balance : param2, param1, param2);
        } else {
            acc = std::make_shared<CurrentAccount>(accNo, holder, balance >= 0 ? balance : 0.0, param1);
        }
        acc->setBalanceDirect(balance);
        acc->setActiveDirect(isActive);
        acc->clearTransactions();
        bank.addAccountDirect(acc);
        outResult.recordsLoaded++;
    }
    inAcc.close();

    // ------------------------------------------------------------------------
    // 3. Load Transactions
    // ------------------------------------------------------------------------
    std::string txnFile = directory + "/transactions.txt";
    std::ifstream inTxn(txnFile);
    if (!inTxn.is_open()) {
        std::string txnBak = directory + "/transactions.txt.bak";
        std::ifstream inTxnBak(txnBak);
        if (inTxnBak.is_open()) {
            outResult.addWarning("Primary transactions file missing. Recovering from backup: " + txnBak);
            inTxn.swap(inTxnBak);
        }
    }

    if (inTxn.is_open()) {
        lineNum = 0;
        while (std::getline(inTxn, line)) {
            lineNum++;
            if (line.empty() || line[0] == '#') continue;

            auto tokens = split(line, '|');
            if (tokens.size() < 8) {
                outResult.addWarning("transactions.txt (Line " + std::to_string(lineNum) + "): Skipped malformed transaction record");
                continue;
            }

            std::string txnId   = tokens[0];
            std::string fromAcc = tokens[1];
            std::string toAcc   = tokens[2];

            double amount = 0.0;
            if (!safeParseDouble(tokens[3], amount)) {
                outResult.addWarning("transactions.txt (Line " + std::to_string(lineNum) + "): Skipped transaction " + txnId + " due to invalid amount '" + tokens[3] + "'");
                continue;
            }

            std::string timestamp = tokens[4];
            int typeInt = 0, statusInt = 0;
            if (!safeParseInt(tokens[5], typeInt) || !safeParseInt(tokens[6], statusInt)) {
                outResult.addWarning("transactions.txt (Line " + std::to_string(lineNum) + "): Skipped transaction " + txnId + " due to invalid enum types");
                continue;
            }

            std::string desc = tokens[7];

            Transaction t(txnId, fromAcc, toAcc, amount,
                          static_cast<TransactionType>(typeInt),
                          static_cast<TransactionStatus>(statusInt),
                          desc, timestamp);

            auto destAcc = bank.getAccount(toAcc);
            if (destAcc) {
                destAcc->loadTransaction(t);
            }
            if (fromAcc != toAcc) {
                auto srcAcc = bank.getAccount(fromAcc);
                if (srcAcc) {
                    srcAcc->loadTransaction(t);
                }
            }
            outResult.recordsLoaded++;
        }
        inTxn.close();
    }

    outResult.success = true;
    if (anyMigrated) {
        PersistenceResult autoSaveRes;
        saveBankStateAtomic(bank, directory, autoSaveRes);
    }
    return true;
}

bool FilePersistence::restoreFromBackup(BankSystem& bank, const std::string& directory, PersistenceResult* outResult) {
    std::string custBak = directory + "/customers.txt.bak";
    std::string accBak  = directory + "/accounts.txt.bak";
    std::string txnBak  = directory + "/transactions.txt.bak";

    std::string custFinal = directory + "/customers.txt";
    std::string accFinal  = directory + "/accounts.txt";
    std::string txnFinal  = directory + "/transactions.txt";

    bool okCust = copyFile(custBak, custFinal);
    bool okAcc  = copyFile(accBak, accFinal);
    bool okTxn  = copyFile(txnBak, txnFinal);

    if (!okCust && !okAcc && !okTxn) {
        if (outResult) outResult->errorMessage = "No valid .bak backup files found in " + directory;
        return false;
    }

    PersistenceResult loadRes;
    bool loaded = loadBankStateResilient(bank, directory, loadRes);
    if (outResult) *outResult = loadRes;
    return loaded;
}

bool FilePersistence::verifyStorageIntegrity(const std::string& directory, std::vector<std::string>& outIssues) {
    outIssues.clear();

    std::string custFile = directory + "/customers.txt";
    std::string accFile  = directory + "/accounts.txt";
    std::string txnFile  = directory + "/transactions.txt";

    std::ifstream inCust(custFile);
    if (!inCust.is_open()) outIssues.push_back("Missing or unreadable file: " + custFile);

    std::ifstream inAcc(accFile);
    if (!inAcc.is_open()) outIssues.push_back("Missing or unreadable file: " + accFile);

    std::ifstream inTxn(txnFile);
    if (!inTxn.is_open()) outIssues.push_back("Missing or unreadable file: " + txnFile);

    return outIssues.empty();
}
