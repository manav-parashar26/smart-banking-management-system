#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "Customer.h"

// ============================================================================
// CUSTOMER MANAGER / REGISTRY
// Demonstrating DSA: STL std::unordered_map for O(1) Average Lookup
// ============================================================================

class CustomerManager {
private:
    // Hash Table: Key = Customer ID (std::string), Value = Customer entity
    // Time Complexity: O(1) Average search/insert/delete
    std::unordered_map<std::string, Customer> m_customers;

public:
    CustomerManager() = default;

    // Core Operations
    bool registerCustomer(const Customer& customer);
    Customer* findCustomer(const std::string& customerId);
    const Customer* findCustomer(const std::string& customerId) const;
    bool authenticate(const std::string& customerId, const std::string& pin);

    // Iteration & Metrics
    size_t getCustomerCount() const;
    void displayAllCustomers() const;
    const std::unordered_map<std::string, Customer>& getAllCustomers() const;
};
