#include "Customer.h"
#include "Account.h" // For BankingException
#include "SecurityHelper.h"
#include <algorithm>
#include <iomanip>

Customer::Customer() 
    : m_customerId(""), m_fullName(""), m_email(""), m_phone(""), m_pinHash("") {}

Customer::Customer(const std::string& id,
                   const std::string& name,
                   const std::string& email,
                   const std::string& phone,
                   const std::string& pinOrHash,
                   bool isAlreadyHashed)
    : m_customerId(id), m_fullName(name), m_email(email), m_phone(phone) {
    if (id.empty() || name.empty()) {
        throw BankingException("Customer ID and Name cannot be empty.");
    }
    if (isAlreadyHashed || SecurityHelper::isHashed(pinOrHash)) {
        m_pinHash = pinOrHash;
    } else {
        if (pinOrHash.length() < 4) {
            throw BankingException("Customer PIN must contain at least 4 digits.");
        }
        m_pinHash = SecurityHelper::hashPin(pinOrHash);
    }
}

std::string Customer::getCustomerId() const { return m_customerId; }
std::string Customer::getFullName() const { return m_fullName; }
std::string Customer::getEmail() const { return m_email; }
std::string Customer::getPhone() const { return m_phone; }
std::string Customer::getPinHash() const { return m_pinHash; }
const std::vector<std::string>& Customer::getLinkedAccountNumbers() const { return m_linkedAccountNumbers; }

bool Customer::verifyPin(const std::string& pin) const {
    return SecurityHelper::verifyPin(pin, m_pinHash);
}

void Customer::changePin(const std::string& oldPin, const std::string& newPin) {
    if (!verifyPin(oldPin)) {
        throw BankingException("Security Error: Current PIN verification failed.");
    }
    if (newPin.length() < 4) {
        throw BankingException("Security Error: New PIN must be at least 4 digits.");
    }
    m_pinHash = SecurityHelper::hashPin(newPin);
}

void Customer::updateProfile(const std::string& name, const std::string& email, const std::string& phone) {
    if (!name.empty()) m_fullName = name;
    if (!email.empty()) m_email = email;
    if (!phone.empty()) m_phone = phone;
}

void Customer::linkAccount(const std::string& accountNumber) {
    auto it = std::find(m_linkedAccountNumbers.begin(), m_linkedAccountNumbers.end(), accountNumber);
    if (it == m_linkedAccountNumbers.end()) {
        m_linkedAccountNumbers.push_back(accountNumber);
    }
}

bool Customer::hasAccount(const std::string& accountNumber) const {
    return std::find(m_linkedAccountNumbers.begin(), m_linkedAccountNumbers.end(), accountNumber) != m_linkedAccountNumbers.end();
}

void Customer::displayCustomerProfile() const {
    std::cout << "----------------------------------------------------\n";
    std::cout << "Customer ID    : " << m_customerId << "\n";
    std::cout << "Full Name      : " << m_fullName << "\n";
    std::cout << "Email Address  : " << m_email << "\n";
    std::cout << "Phone Number   : " << m_phone << "\n";
    std::cout << "Linked Accounts: ";
    if (m_linkedAccountNumbers.empty()) {
        std::cout << "None\n";
    } else {
        for (size_t i = 0; i < m_linkedAccountNumbers.size(); ++i) {
            std::cout << m_linkedAccountNumbers[i] << (i + 1 < m_linkedAccountNumbers.size() ? ", " : "\n");
        }
    }
    std::cout << "----------------------------------------------------\n";
}

CustomerDTO Customer::toDTO() const {
    return CustomerDTO{
        m_customerId,
        m_fullName,
        m_email,
        m_phone,
        m_linkedAccountNumbers
    };
}

