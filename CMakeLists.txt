cmake_minimum_required(VERSION 3.10)
project(SmartBankingSystem VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    backend/core/models
    backend/core/services
    backend/core/structures
    backend/core/dto
    backend/storage
    backend/cli
    backend/api
    backend/database
)

set(SOURCES
    backend/core/models/Account.cpp
    backend/core/models/SavingsAccount.cpp
    backend/core/models/CurrentAccount.cpp
    backend/core/models/Customer.cpp
    backend/core/models/Transaction.cpp
    backend/core/services/CustomerManager.cpp
    backend/core/services/SecurityHelper.cpp
    backend/core/services/BankSystem.cpp
    backend/core/services/TransactionAnalytics.cpp
    backend/core/services/TransactionGraph.cpp
    backend/core/services/SmartAuditor.cpp
    backend/core/services/DSABenchmark.cpp
    backend/storage/FilePersistence.cpp
    backend/api/HttpServer.cpp
    backend/database/DatabaseManager.cpp
    backend/cli/BankingApp.cpp
    backend/main.cpp
)

add_executable(SmartBankingSystem ${SOURCES})
target_link_libraries(SmartBankingSystem ws2_32 advapi32)
