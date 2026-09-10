#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "BankingDTOs.h"

// ============================================================================
// LIGHTWEIGHT ZERO-DEPENDENCY JSON SERIALIZER & PARSER
// Handcrafted C++17 JSON utilities for REST API contracts
// ============================================================================

class JsonHelper {
public:
    // String escaping for valid JSON syntax
    static std::string escape(const std::string& s) {
        std::ostringstream o;
        for (char c : s) {
            switch (c) {
                case '"':  o << "\\\""; break;
                case '\\': o << "\\\\"; break;
                case '\b': o << "\\b";  break;
                case '\f': o << "\\f";  break;
                case '\n': o << "\\n";  break;
                case '\r': o << "\\r";  break;
                case '\t': o << "\\t";  break;
                default:   o << c;       break;
            }
        }
        return o.str();
    }

    // ------------------------------------------------------------------------
    // DTO Serializers
    // ------------------------------------------------------------------------
    static std::string toJson(const CustomerDTO& c) {
        std::ostringstream oss;
        oss << "{"
            << "\"customerId\":\"" << escape(c.customerId) << "\","
            << "\"fullName\":\"" << escape(c.fullName) << "\","
            << "\"email\":\"" << escape(c.email) << "\","
            << "\"phone\":\"" << escape(c.phone) << "\","
            << "\"linkedAccounts\":[";
        for (size_t i = 0; i < c.linkedAccounts.size(); ++i) {
            oss << "\"" << escape(c.linkedAccounts[i]) << "\"" << (i + 1 < c.linkedAccounts.size() ? "," : "");
        }
        oss << "]}";
        return oss.str();
    }

    static std::string toJson(const std::vector<CustomerDTO>& list) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            oss << toJson(list[i]) << (i + 1 < list.size() ? "," : "");
        }
        oss << "]";
        return oss.str();
    }

    static std::string toJson(const AccountDTO& a) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "{"
            << "\"accountNumber\":\"" << escape(a.accountNumber) << "\","
            << "\"accountType\":\"" << escape(a.accountType) << "\","
            << "\"holderName\":\"" << escape(a.holderName) << "\","
            << "\"balance\":" << a.balance << ","
            << "\"isActive\":" << (a.isActive ? "true" : "false") << ","
            << "\"interestRate\":" << a.interestRate << ","
            << "\"minimumBalance\":" << a.minimumBalance << ","
            << "\"overdraftLimit\":" << a.overdraftLimit << ","
            << "\"availableCredit\":" << a.availableCredit
            << "}";
        return oss.str();
    }

    static std::string toJson(const std::vector<AccountDTO>& list) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            oss << toJson(list[i]) << (i + 1 < list.size() ? "," : "");
        }
        oss << "]";
        return oss.str();
    }

    static std::string toJson(const TransactionDTO& t) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "{"
            << "\"transactionId\":\"" << escape(t.transactionId) << "\","
            << "\"fromAccount\":\"" << escape(t.fromAccount) << "\","
            << "\"toAccount\":\"" << escape(t.toAccount) << "\","
            << "\"amount\":" << t.amount << ","
            << "\"timestamp\":\"" << escape(t.timestamp) << "\","
            << "\"type\":\"" << escape(t.type) << "\","
            << "\"status\":\"" << escape(t.status) << "\","
            << "\"description\":\"" << escape(t.description) << "\""
            << "}";
        return oss.str();
    }

    static std::string toJson(const std::vector<TransactionDTO>& list) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < list.size(); ++i) {
            oss << toJson(list[i]) << (i + 1 < list.size() ? "," : "");
        }
        oss << "]";
        return oss.str();
    }

    static std::string toJson(const AuthResponseDTO& a) {
        std::ostringstream oss;
        oss << "{"
            << "\"success\":" << (a.success ? "true" : "false") << ","
            << "\"customerId\":\"" << escape(a.customerId) << "\","
            << "\"fullName\":\"" << escape(a.fullName) << "\","
            << "\"token\":\"" << escape(a.token) << "\","
            << "\"message\":\"" << escape(a.message) << "\""
            << "}";
        return oss.str();
    }

    // ------------------------------------------------------------------------
    // API Response Envelope Builders
    // ------------------------------------------------------------------------
    static std::string formatSuccess(const std::string& rawDataJson, const std::string& message = "Operation successful") {
        std::ostringstream oss;
        oss << "{"
            << "\"status\":200,"
            << "\"success\":true,"
            << "\"message\":\"" << escape(message) << "\","
            << "\"data\":" << (rawDataJson.empty() ? "{}" : rawDataJson)
            << "}";
        return oss.str();
    }

    static std::string formatError(int statusCode, const std::string& errorCode, const std::string& message) {
        std::ostringstream oss;
        oss << "{"
            << "\"status\":" << statusCode << ","
            << "\"success\":false,"
            << "\"errorCode\":\"" << escape(errorCode) << "\","
            << "\"message\":\"" << escape(message) << "\""
            << "}";
        return oss.str();
    }

    // ------------------------------------------------------------------------
    // Safe JSON Parsers for Incoming Payloads
    // ------------------------------------------------------------------------
    static std::string getString(const std::string& json, const std::string& key, const std::string& defaultVal = "") {
        std::string pattern = "\"" + key + "\"";
        size_t pos = json.find(pattern);
        if (pos == std::string::npos) return defaultVal;

        size_t colon = json.find(':', pos + pattern.length());
        if (colon == std::string::npos) return defaultVal;

        size_t quoteStart = json.find('"', colon + 1);
        if (quoteStart == std::string::npos) return defaultVal;

        size_t quoteEnd = quoteStart + 1;
        while (quoteEnd < json.length()) {
            if (json[quoteEnd] == '"' && json[quoteEnd - 1] != '\\') break;
            quoteEnd++;
        }
        if (quoteEnd >= json.length()) return defaultVal;

        return json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
    }

    static double getDouble(const std::string& json, const std::string& key, double defaultVal = 0.0) {
        std::string pattern = "\"" + key + "\"";
        size_t pos = json.find(pattern);
        if (pos == std::string::npos) return defaultVal;

        size_t colon = json.find(':', pos + pattern.length());
        if (colon == std::string::npos) return defaultVal;

        size_t valStart = json.find_first_not_of(" \t\r\n\"", colon + 1);
        if (valStart == std::string::npos) return defaultVal;

        size_t valEnd = json.find_first_of(",}\" \t\r\n", valStart);
        std::string numStr = (valEnd == std::string::npos) ? json.substr(valStart) : json.substr(valStart, valEnd - valStart);

        try {
            return std::stod(numStr);
        } catch (...) {
            return defaultVal;
        }
    }

    static int getInt(const std::string& json, const std::string& key, int defaultVal = 0) {
        return static_cast<int>(getDouble(json, key, defaultVal));
    }

    static bool getBool(const std::string& json, const std::string& key, bool defaultVal = false) {
        std::string pattern = "\"" + key + "\"";
        size_t pos = json.find(pattern);
        if (pos == std::string::npos) return defaultVal;

        size_t colon = json.find(':', pos + pattern.length());
        if (colon == std::string::npos) return defaultVal;

        size_t valStart = json.find_first_not_of(" \t\r\n\"", colon + 1);
        if (valStart == std::string::npos) return defaultVal;

        if (json.substr(valStart, 4) == "true") return true;
        if (json.substr(valStart, 5) == "false") return false;
        return defaultVal;
    }
};
