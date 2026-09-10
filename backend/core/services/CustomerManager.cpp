#include "CustomerManager.h"
#include <iostream>
#include <iomanip>

bool CustomerManager::registerCustomer(const Customer& customer) {
    // std::unordered_map::find is O(1) average time complexity
    if (m_customers.find(customer.getCustomerId()) != m_customers.end()) {
        return false; // Customer already exists
    }
    m_customers[customer.getCustomerId()] = customer;
    return true;
}

Customer* CustomerManager::findCustomer(const std::string& customerId) {
    auto it = m_customers.find(customerId);
    if (it != m_customers.end()) {
        return &(it->second);
    }
    return nullptr;
}

const Customer* CustomerManager::findCustomer(const std::string& customerId) const {
    auto it = m_customers.find(customerId);
    if (it != m_customers.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool CustomerManager::authenticate(const std::string& customerId, const std::string& pin) {
    Customer* cust = findCustomer(customerId);
    if (cust == nullptr) {
        return false;
    }
    return cust->verifyPin(pin);
}

size_t CustomerManager::getCustomerCount() const {
    return m_customers.size();
}

void CustomerManager::displayAllCustomers() const {
    std::cout << "\n==============================================================================\n";
    std::cout << "                         REGISTERED BANK CUSTOMERS (" << m_customers.size() << ")\n";
    std::cout << "==============================================================================\n";
    std::cout << std::left 
              << std::setw(12) << "Customer ID" << " | "
              << std::setw(20) << "Full Name" << " | "
              << std::setw(25) << "Email" << " | "
              << "Phone\n";
    std::cout << "------------------------------------------------------------------------------\n";

    for (const auto& pair : m_customers) {
        const Customer& c = pair.second;
        std::cout << std::left
                  << std::setw(12) << c.getCustomerId() << " | "
                  << std::setw(20) << c.getFullName() << " | "
                  << std::setw(25) << c.getEmail() << " | "
                  << c.getPhone() << "\n";
    }
    std::cout << "==============================================================================\n\n";
}

const std::unordered_map<std::string, Customer>& CustomerManager::getAllCustomers() const {
    return m_customers;
}
