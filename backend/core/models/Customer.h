#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "BankingDTOs.h"

// ============================================================================
// CUSTOMER ENTITY CLASS
// Demonstrating: Encapsulation, Authentication, Account Linking
// ============================================================================

class Customer {
private:
    std::string m_customerId;
    std::string m_fullName;
    std::string m_email;
    std::string m_phone;
    std::string m_pinHash; // Cryptographic salted hash (PBKDF2-HMAC-SHA256)
    std::vector<std::string> m_linkedAccountNumbers; // STL vector of account IDs owned by customer

public:
    Customer();
    Customer(const std::string& id,
             const std::string& name,
             const std::string& email,
             const std::string& phone,
             const std::string& pinOrHash,
             bool isAlreadyHashed = false);

    // Getters
    std::string getCustomerId() const;
    std::string getFullName() const;
    std::string getEmail() const;
    std::string getPhone() const;
    std::string getPinHash() const;
    std::string getPin() const { return getPinHash(); } // Backward compatibility
    const std::vector<std::string>& getLinkedAccountNumbers() const;

    // Authentication & Profile Updates
    bool verifyPin(const std::string& pin) const;
    void changePin(const std::string& oldPin, const std::string& newPin);
    void updateProfile(const std::string& name, const std::string& email, const std::string& phone);

    // Account Association
    void linkAccount(const std::string& accountNumber);
    bool hasAccount(const std::string& accountNumber) const;

    // DTO Conversion
    CustomerDTO toDTO() const;

    // Display
    void displayCustomerProfile() const;
};
