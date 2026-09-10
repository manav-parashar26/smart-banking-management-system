#include "Transaction.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

static uint64_t s_txnCounter = 100000;

Transaction::Transaction(const std::string& id,
                         const std::string& fromAcc,
                         const std::string& toAcc,
                         double amount,
                         TransactionType type,
                         TransactionStatus status,
                         const std::string& desc,
                         const std::string& timestamp)
    : m_transactionId(id),
      m_fromAccount(fromAcc),
      m_toAccount(toAcc),
      m_amount(amount),
      m_timestamp(timestamp.empty() ? getCurrentTimestamp() : timestamp),
      m_type(type),
      m_status(status),
      m_description(desc) {}

std::string Transaction::getTransactionId() const { return m_transactionId; }
std::string Transaction::getFromAccount() const { return m_fromAccount; }
std::string Transaction::getToAccount() const { return m_toAccount; }
double Transaction::getAmount() const { return m_amount; }
std::string Transaction::getTimestamp() const { return m_timestamp; }
TransactionType Transaction::getType() const { return m_type; }
TransactionStatus Transaction::getStatus() const { return m_status; }
std::string Transaction::getDescription() const { return m_description; }

std::string Transaction::generateTransactionId() {
    return "TXN-" + std::to_string(++s_txnCounter);
}

std::string Transaction::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm* timeInfo = std::localtime(&nowTime);
    std::ostringstream oss;
    oss << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Transaction::typeToString(TransactionType type) {
    switch (type) {
        case TransactionType::DEPOSIT: return "DEPOSIT";
        case TransactionType::WITHDRAWAL: return "WITHDRAWAL";
        case TransactionType::TRANSFER_IN: return "TRANSFER_IN";
        case TransactionType::TRANSFER_OUT: return "TRANSFER_OUT";
        default: return "UNKNOWN";
    }
}

std::string Transaction::statusToString(TransactionStatus status) {
    switch (status) {
        case TransactionStatus::SUCCESS: return "SUCCESS";
        case TransactionStatus::FAILED: return "FAILED";
        default: return "UNKNOWN";
    }
}

void Transaction::displayTransaction() const {
    std::cout << std::left 
              << std::setw(12) << m_transactionId << " | "
              << std::setw(19) << m_timestamp << " | "
              << std::setw(13) << typeToString(m_type) << " | $"
              << std::right << std::setw(9) << std::fixed << std::setprecision(2) << m_amount << " | "
              << std::left
              << std::setw(10) << m_fromAccount << " -> "
              << std::setw(10) << m_toAccount << " | "
              << std::setw(8) << statusToString(m_status) << " | "
              << m_description << "\n";
}

TransactionDTO Transaction::toDTO() const {
    return TransactionDTO{
        m_transactionId,
        m_fromAccount,
        m_toAccount,
        m_amount,
        m_timestamp,
        typeToString(m_type),
        statusToString(m_status),
        m_description
    };
}

