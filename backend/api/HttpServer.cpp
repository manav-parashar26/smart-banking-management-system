#include "HttpServer.h"
#include "TransactionAnalytics.h"
#include "TransactionGraph.h"
#include "SmartAuditor.h"
#include "FilePersistence.h"
#include "DatabaseManager.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>

static std::string readStaticFile(const std::string& filePath) {
    std::vector<std::string> candidates = {
        filePath,
        "frontend/" + filePath,
        "../frontend/" + filePath,
        "../../frontend/" + filePath
    };
    for (const auto& path : candidates) {
        std::ifstream in(path, std::ios::binary);
        if (in.is_open()) {
            std::ostringstream sstr;
            sstr << in.rdbuf();
            return sstr.str();
        }
    }
    return "";
}

std::string HttpResponse::toRawHttpString() const {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    oss << "Content-Type: " << contentType << "\r\n";
    oss << "Content-Length: " << body.length() << "\r\n";
    oss << "Access-Control-Allow-Origin: *\r\n";
    oss << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n";
    oss << "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With, X-Admin-Key\r\n";
    oss << "Connection: close\r\n";
    oss << "\r\n";
    oss << body;
    return oss.str();
}

HttpServer::HttpServer(BankSystem& bank, int port, const std::string& adminKey)
    : m_bank(bank), m_port(port), m_adminKey(adminKey), m_isRunning(false), m_serverSocket(0) {
    const char* envKey = std::getenv("SMART_BANKING_ADMIN_KEY");
    if (envKey != nullptr && std::string(envKey).length() > 0) {
        m_adminKey = std::string(envKey);
    }
}

HttpServer::~HttpServer() {
    stop();
}

HttpRequest HttpServer::parseRawRequest(const std::string& raw) {
    HttpRequest req;
    if (raw.empty()) return req;

    std::istringstream stream(raw);
    std::string line;

    // Line 1: Request Line (METHOD PATH HTTP/1.1)
    if (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::istringstream lineStream(line);
        lineStream >> req.method;
        std::string fullPath;
        lineStream >> fullPath;

        // Parse query parameters
        size_t queryPos = fullPath.find('?');
        if (queryPos != std::string::npos) {
            req.path = fullPath.substr(0, queryPos);
            req.queryString = fullPath.substr(queryPos + 1);

            std::istringstream queryStream(req.queryString);
            std::string pair;
            while (std::getline(queryStream, pair, '&')) {
                size_t eqPos = pair.find('=');
                if (eqPos != std::string::npos) {
                    req.queryParams[pair.substr(0, eqPos)] = pair.substr(eqPos + 1);
                } else {
                    req.queryParams[pair] = "";
                }
            }
        } else {
            req.path = fullPath;
        }
    }

    // Headers
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break; // End of headers

        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string name = line.substr(0, colon);
            std::string val = line.substr(colon + 1);
            while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(val.begin());
            req.headers[name] = val;
        }
    }

    // Body: Extract directly from raw string after header delimiter
    size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        req.body = raw.substr(headerEnd + 4);
    } else {
        size_t nlEnd = raw.find("\n\n");
        if (nlEnd != std::string::npos) {
            req.body = raw.substr(nlEnd + 2);
        }
    }

    return req;
}

HttpResponse HttpServer::dispatch(const HttpRequest& req) {
    // ------------------------------------------------------------------------
    // 1. CORS Preflight Handling (Crucial for React & modern browsers)
    // ------------------------------------------------------------------------
    if (req.method == "OPTIONS") {
        return HttpResponse(204, "");
    }

    try {
        // --------------------------------------------------------------------
        // Centralized Admin API Authorization Middleware
        // Intercepts all /api/admin/* routes and validates X-Admin-Key header
        // --------------------------------------------------------------------
        if (req.path.rfind("/api/admin/", 0) == 0) {
            std::string providedKey = req.getHeader("X-Admin-Key");
            if (providedKey.empty() || providedKey != m_adminKey) {
                return HttpResponse(401, JsonHelper::formatError(401, "UNAUTHORIZED", "Missing or invalid X-Admin-Key header for admin endpoint access."));
            }
        }

        // --------------------------------------------------------------------
        // 2. Authentication Routes
        // --------------------------------------------------------------------
        if (req.method == "POST" && req.path == "/api/auth/login") {
            std::string custId = JsonHelper::getString(req.body, "customerId");
            std::string pin = JsonHelper::getString(req.body, "pin");
            auto authRes = m_bank.authenticateCustomer(AuthRequestDTO{custId, pin});
            return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(authRes), "Login successful"));
        }

        if (req.method == "POST" && req.path == "/api/auth/register") {
            CustomerDTO c;
            c.customerId = JsonHelper::getString(req.body, "customerId");
            c.fullName   = JsonHelper::getString(req.body, "fullName");
            c.email      = JsonHelper::getString(req.body, "email");
            c.phone      = JsonHelper::getString(req.body, "phone");
            std::string pin = JsonHelper::getString(req.body, "pin");

            auto authRes = m_bank.registerCustomerDTO(c, pin);
            FilePersistence::saveBankState(m_bank, "data");
            DatabaseManager::exportToMySQLDump(m_bank, "data/banking_system_dump.sql");
            return HttpResponse(201, JsonHelper::formatSuccess(JsonHelper::toJson(authRes), "Customer registered successfully"));
        }

        // --------------------------------------------------------------------
        // 3. Account Routes
        // --------------------------------------------------------------------
        if (req.method == "GET" && req.path == "/api/accounts") {
            std::string custId = req.getQueryParam("customerId");
            if (!custId.empty()) {
                auto accs = m_bank.getCustomerAccountDTOs(custId);
                return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(accs)));
            } else {
                auto accs = m_bank.getAllAccountDTOs();
                return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(accs)));
            }
        }

        // GET /api/accounts/{accountNumber}
        if (req.method == "GET" && req.path.rfind("/api/accounts/", 0) == 0) {
            std::string accNo = req.path.substr(14);
            auto acc = m_bank.getAccountDTO(accNo);
            return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(acc)));
        }

        if (req.method == "POST" && req.path == "/api/accounts/deposit") {
            std::string accNo = JsonHelper::getString(req.body, "accountNumber");
            double amount     = JsonHelper::getDouble(req.body, "amount");
            std::string desc  = JsonHelper::getString(req.body, "description", "API Deposit");

            auto updated = m_bank.depositMoneyDTO(DepositWithdrawDTO{accNo, amount, desc});
            FilePersistence::saveBankState(m_bank, "data");
            DatabaseManager::exportToMySQLDump(m_bank, "data/banking_system_dump.sql");
            return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(updated), "Deposit successful"));
        }

        if (req.method == "POST" && req.path == "/api/accounts/withdraw") {
            std::string accNo = JsonHelper::getString(req.body, "accountNumber");
            double amount     = JsonHelper::getDouble(req.body, "amount");
            std::string desc  = JsonHelper::getString(req.body, "description", "API Withdrawal");

            auto updated = m_bank.withdrawMoneyDTO(DepositWithdrawDTO{accNo, amount, desc});
            FilePersistence::saveBankState(m_bank, "data");
            DatabaseManager::exportToMySQLDump(m_bank, "data/banking_system_dump.sql");
            return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(updated), "Withdrawal successful"));
        }

        if (req.method == "POST" && req.path == "/api/accounts/transfer") {
            std::string fromAcc = JsonHelper::getString(req.body, "fromAccount");
            std::string toAcc   = JsonHelper::getString(req.body, "toAccount");
            double amount       = JsonHelper::getDouble(req.body, "amount");
            std::string desc    = JsonHelper::getString(req.body, "description", "API Fund Transfer");

            auto txn = m_bank.transferMoneyDTO(TransferRequestDTO{fromAcc, toAcc, amount, desc});
            FilePersistence::saveBankState(m_bank, "data");
            DatabaseManager::exportToMySQLDump(m_bank, "data/banking_system_dump.sql");
            return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(txn), "Transfer executed successfully"));
        }

        // --------------------------------------------------------------------
        // 4. Transaction & Ledger Routes
        // --------------------------------------------------------------------
        if (req.method == "GET" && req.path == "/api/transactions") {
            std::string accNo = req.getQueryParam("accountNumber");
            if (!accNo.empty()) {
                auto txns = m_bank.getAccountTransactionDTOs(accNo);
                return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(txns)));
            } else {
                auto txns = m_bank.getAllTransactionDTOs();
                return HttpResponse(200, JsonHelper::formatSuccess(JsonHelper::toJson(txns)));
            }
        }

        // --------------------------------------------------------------------
        // 5. Analytics & Financial Health Routes
        // --------------------------------------------------------------------
        if (req.method == "GET" && req.path == "/api/analytics/categories") {
            auto allTxns = m_bank.getAllTransactionsLedger();
            auto catMap = TransactionAnalytics::generateCategorySpendingSummary(allTxns);

            std::ostringstream jsonStream;
            jsonStream << "{";
            size_t idx = 0;
            for (const auto& pair : catMap) {
                jsonStream << "\"" << JsonHelper::escape(pair.first) << "\":" 
                           << std::fixed << std::setprecision(2) << pair.second
                           << (idx + 1 < catMap.size() ? "," : "");
                idx++;
            }
            jsonStream << "}";

            return HttpResponse(200, JsonHelper::formatSuccess(jsonStream.str()));
        }

        // GET /api/analytics/health/{accountNumber}
        if (req.method == "GET" && req.path.rfind("/api/analytics/health/", 0) == 0) {
            std::string accNo = req.path.substr(22);
            auto acc = m_bank.getAccount(accNo);
            if (!acc) {
                return HttpResponse(404, JsonHelper::formatError(404, "NOT_FOUND", "Account " + accNo + " not found."));
            }

            auto health = SmartAuditor::evaluateFinancialHealth(acc->getTransactionHistory());
            std::ostringstream oss;
            oss << "{"
                << "\"totalIncome\":" << health.totalIncome << ","
                << "\"totalExpenses\":" << health.totalExpenses << ","
                << "\"netSavings\":" << health.netSavings << ","
                << "\"savingsRate\":" << health.savingsRate << ","
                << "\"statusRating\":\"" << JsonHelper::escape(health.statusRating) << "\","
                << "\"recommendation\":\"" << JsonHelper::escape(health.recommendation) << "\""
                << "}";
            return HttpResponse(200, JsonHelper::formatSuccess(oss.str()));
        }

        // --------------------------------------------------------------------
        // 6. Admin & Queue / Undo Stack Routes
        // --------------------------------------------------------------------
        if (req.method == "GET" && req.path == "/api/admin/requests") {
            auto reqs = m_bank.getPendingRequestsList();
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < reqs.size(); ++i) {
                oss << "{"
                    << "\"requestId\":\"" << reqs[i].requestId << "\","
                    << "\"customerId\":\"" << reqs[i].customerId << "\","
                    << "\"accountNumber\":\"" << reqs[i].accountNumber << "\","
                    << "\"type\":\"" << BankingRequest::typeToString(reqs[i].type) << "\","
                    << "\"amount\":" << reqs[i].requestedAmount << ","
                    << "\"notes\":\"" << JsonHelper::escape(reqs[i].notes) << "\""
                    << "}" << (i + 1 < reqs.size() ? "," : "");
            }
            oss << "]";
            return HttpResponse(200, JsonHelper::formatSuccess(oss.str()));
        }

        if (req.method == "POST" && req.path == "/api/admin/process-request") {
            bool approve = JsonHelper::getBool(req.body, "approve", true);
            std::string message;
            bool ok = m_bank.processNextBankingRequest(approve, message);
            if (ok) {
                FilePersistence::saveBankState(m_bank, "data");
                DatabaseManager::exportToMySQLDump(m_bank, "data/banking_system_dump.sql");
                return HttpResponse(200, JsonHelper::formatSuccess("{}", message));
            } else {
                return HttpResponse(400, JsonHelper::formatError(400, "QUEUE_EMPTY", message));
            }
        }

        if (req.method == "POST" && req.path == "/api/admin/undo") {
            std::string message;
            bool ok = m_bank.undoLastTransaction(message);
            if (ok) {
                FilePersistence::saveBankState(m_bank, "data");
                DatabaseManager::exportToMySQLDump(m_bank, "data/banking_system_dump.sql");
                return HttpResponse(200, JsonHelper::formatSuccess("{}", message));
            } else {
                return HttpResponse(400, JsonHelper::formatError(400, "STACK_EMPTY", message));
            }
        }

        if (req.method == "GET" && req.path == "/api/admin/network-graph") {
            TransactionGraph graph;
            graph.populateFromLedger(m_bank.getAllTransactionsLedger());
            const auto& adj = graph.getAdjacencyList();
            auto cycles = graph.findCircularRoutingRings();

            std::ostringstream oss;
            oss << "{\"vertices\":[";
            size_t vIdx = 0;
            for (const auto& v : graph.getVertices()) {
                oss << "\"" << v << "\"" << (vIdx + 1 < graph.getVertices().size() ? "," : "");
                vIdx++;
            }
            oss << "],\"edges\":[";
            size_t eIdx = 0;
            for (const auto& pair : adj) {
                for (const auto& edge : pair.second) {
                    if (eIdx > 0) oss << ",";
                    oss << "{\"from\":\"" << pair.first << "\","
                        << "\"to\":\"" << edge.toAccount << "\","
                        << "\"amount\":" << edge.amount << ","
                        << "\"txnId\":\"" << edge.txnId << "\"}";
                    eIdx++;
                }
            }
            oss << "],\"cycles\":[";
            for (size_t cIdx = 0; cIdx < cycles.size(); ++cIdx) {
                if (cIdx > 0) oss << ",";
                oss << "[";
                for (size_t pIdx = 0; pIdx < cycles[cIdx].size(); ++pIdx) {
                    if (pIdx > 0) oss << ",";
                    oss << "\"" << cycles[cIdx][pIdx] << "\"";
                }
                oss << "]";
            }
            oss << "]}";

            return HttpResponse(200, JsonHelper::formatSuccess(oss.str()));
        }

        // --------------------------------------------------------------------
        // 9. Static Asset Serving for Frontend Single-Page Application
        // --------------------------------------------------------------------
        if (req.method == "GET") {
            if (req.path == "/" || req.path == "/index.html") {
                std::string content = readStaticFile("index.html");
                if (!content.empty()) {
                    return HttpResponse(200, content, "text/html; charset=UTF-8");
                }
            } else if (req.path == "/style.css") {
                std::string content = readStaticFile("style.css");
                if (!content.empty()) {
                    return HttpResponse(200, content, "text/css");
                }
            } else if (req.path == "/api.js") {
                std::string content = readStaticFile("api.js");
                if (!content.empty()) {
                    return HttpResponse(200, content, "application/javascript");
                }
            } else if (req.path == "/App.js") {
                std::string content = readStaticFile("App.js");
                if (!content.empty()) {
                    return HttpResponse(200, content, "application/javascript");
                }
            }
        }

        // 404 Route Not Found
        return HttpResponse(404, JsonHelper::formatError(404, "NOT_FOUND", "Resource not found: " + req.path));

    } catch (const BankingException& ex) {
        return HttpResponse(ex.getStatusCode(), JsonHelper::formatError(ex.getStatusCode(), ex.getErrorCode(), ex.what()));
    } catch (const std::exception& ex) {
        return HttpResponse(500, JsonHelper::formatError(500, "INTERNAL_ERROR", ex.what()));
    }
}

std::string HttpServer::handleRawRequest(const std::string& rawRequestString) {
    HttpRequest req = parseRawRequest(rawRequestString);
    HttpResponse res = dispatch(req);
    return res.toRawHttpString();
}

bool HttpServer::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[HttpServer] WSAStartup failed.\n";
        return false;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        std::cerr << "[HttpServer] Socket creation failed.\n";
        WSACleanup();
        return false;
    }

    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(m_port));

    if (bind(s, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Bind failed on port " << m_port << ".\n";
        closesocket(s);
        WSACleanup();
        return false;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "[HttpServer] Listen failed.\n";
        closesocket(s);
        WSACleanup();
        return false;
    }

    m_serverSocket = static_cast<uintptr_t>(s);
    m_isRunning = true;
    std::cout << "\n==============================================================================\n";
    std::cout << " [✓] C++ REST API Server LIVE on http://localhost:" << m_port << "\n";
    std::cout << "     CORS: Enabled (All Origins) | Ready for React Frontend Clients           \n";
    std::cout << "==============================================================================\n\n";

    return true;
}

void HttpServer::stop() {
    if (m_isRunning) {
        m_isRunning = false;
        if (m_serverSocket != 0) {
            closesocket(static_cast<SOCKET>(m_serverSocket));
            m_serverSocket = 0;
        }
        WSACleanup();
        std::cout << "[HttpServer] Stopped cleanly.\n";
    }
}

void HttpServer::runEventLoop(int maxRequests) {
    if (!m_isRunning && !start()) return;

    SOCKET s = static_cast<SOCKET>(m_serverSocket);
    int requestCount = 0;

    std::cout << "Awaiting incoming HTTP requests (Press Ctrl+C to stop)...\n";

    while (m_isRunning && (maxRequests < 0 || requestCount < maxRequests)) {
        sockaddr_in clientAddr{};
        int clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(s, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);

        if (clientSocket == INVALID_SOCKET) {
            if (!m_isRunning) break;
            continue;
        }

        // Read incoming request buffer with Content-Length accumulation
        std::string rawReq;
        char buffer[4096];
        int contentLength = -1;
        size_t headerEndPos = std::string::npos;

        while (true) {
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (bytesReceived <= 0) break;
            rawReq.append(buffer, bytesReceived);

            if (headerEndPos == std::string::npos) {
                headerEndPos = rawReq.find("\r\n\r\n");
                if (headerEndPos != std::string::npos) {
                    std::string headerPart = rawReq.substr(0, headerEndPos);
                    size_t clPos = headerPart.find("Content-Length:");
                    if (clPos == std::string::npos) clPos = headerPart.find("content-length:");
                    if (clPos != std::string::npos) {
                        size_t valStart = headerPart.find_first_not_of(" \t", clPos + 15);
                        size_t valEnd = headerPart.find_first_of("\r\n", valStart);
                        try {
                            contentLength = std::stoi(headerPart.substr(valStart, valEnd - valStart));
                        } catch (...) {
                            contentLength = 0;
                        }
                    } else {
                        contentLength = 0;
                    }
                }
            }

            if (headerEndPos != std::string::npos) {
                size_t bodyBytesReceived = rawReq.length() - (headerEndPos + 4);
                if (contentLength <= 0 || bodyBytesReceived >= static_cast<size_t>(contentLength)) {
                    break;
                }
            }
        }

        if (!rawReq.empty()) {
            std::string rawResp = handleRawRequest(rawReq);
            send(clientSocket, rawResp.c_str(), static_cast<int>(rawResp.length()), 0);
            requestCount++;
        }

        closesocket(clientSocket);
    }
}
