#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "BankSystem.h"
#include "JsonHelper.h"

// ============================================================================
// EMBEDDED C++ HTTP/1.1 REST API SERVER
// Features: Winsock2 Socket I/O, CORS Preflight, In-Process Test Harness,
// DTO-to-JSON Serialization, and Status Code Exception Translation
// ============================================================================

struct HttpRequest {
    std::string method;                             // "GET", "POST", "OPTIONS"
    std::string path;                               // "/api/accounts"
    std::string queryString;                        // "customerId=CUST-101"
    std::map<std::string, std::string> queryParams; // parsed query key-value pairs
    std::map<std::string, std::string> headers;     // header key-value pairs
    std::string body;                               // raw request body (JSON)

    std::string getQueryParam(const std::string& key, const std::string& defaultVal = "") const {
        auto it = queryParams.find(key);
        return it != queryParams.end() ? it->second : defaultVal;
    }

    // Case-insensitive header lookup (e.g. X-Admin-Key vs x-admin-key)
    std::string getHeader(const std::string& name) const {
        for (const auto& pair : headers) {
            if (pair.first.length() == name.length()) {
                bool match = true;
                for (size_t i = 0; i < name.length(); ++i) {
                    if (tolower(static_cast<unsigned char>(pair.first[i])) != tolower(static_cast<unsigned char>(name[i]))) {
                        match = false;
                        break;
                    }
                }
                if (match) return pair.second;
            }
        }
        return "";
    }
};

struct HttpResponse {
    int statusCode;
    std::string statusText;
    std::string contentType;
    std::string body;

    HttpResponse(int code = 200, const std::string& b = "", const std::string& type = "application/json")
        : statusCode(code), contentType(type), body(b) {
        if (code == 200) statusText = "OK";
        else if (code == 201) statusText = "Created";
        else if (code == 204) statusText = "No Content";
        else if (code == 400) statusText = "Bad Request";
        else if (code == 401) statusText = "Unauthorized";
        else if (code == 403) statusText = "Forbidden";
        else if (code == 404) statusText = "Not Found";
        else if (code == 500) statusText = "Internal Server Error";
        else statusText = "Response";
    }

    std::string toRawHttpString() const;
};

class HttpServer {
private:
    BankSystem& m_bank;
    int m_port;
    std::string m_adminKey;   // Configurable API secret for /api/admin/* endpoints
    bool m_isRunning;
    uintptr_t m_serverSocket; // Abstracted SOCKET handle

public:
    explicit HttpServer(BankSystem& bank, int port = 8080, const std::string& adminKey = "admin123");
    ~HttpServer();

    // In-memory Request Router (for testing & direct execution)
    HttpResponse dispatch(const HttpRequest& req);
    std::string handleRawRequest(const std::string& rawRequestString);

    // Live Socket Network Operations
    bool start();
    void stop();
    void runEventLoop(int maxRequests = -1);

    int getPort() const { return m_port; }
    bool isRunning() const { return m_isRunning; }
    const std::string& getAdminKey() const { return m_adminKey; }
    void setAdminKey(const std::string& key) { m_adminKey = key; }

    // Static Protocol Helpers
    static HttpRequest parseRawRequest(const std::string& raw);
};
