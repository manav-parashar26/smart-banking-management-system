@echo off
echo ===================================================
echo Compiling Smart Banking Management System (C++17)...
echo ===================================================

g++ -std=c++17 -Wall -Wextra ^
  -Ibackend/core/models ^
  -Ibackend/core/services ^
  -Ibackend/core/structures ^
  -Ibackend/core/dto ^
  -Ibackend/storage ^
  -Ibackend/cli ^
  -Ibackend/api ^
  -Ibackend/database ^
  backend/core/models/Account.cpp ^
  backend/core/models/SavingsAccount.cpp ^
  backend/core/models/CurrentAccount.cpp ^
  backend/core/models/Customer.cpp ^
  backend/core/models/Transaction.cpp ^
  backend/core/services/CustomerManager.cpp ^
  backend/core/services/SecurityHelper.cpp ^
  backend/core/services/BankSystem.cpp ^
  backend/core/services/TransactionAnalytics.cpp ^
  backend/core/services/TransactionGraph.cpp ^
  backend/core/services/SmartAuditor.cpp ^
  backend/core/services/DSABenchmark.cpp ^
  backend/storage/FilePersistence.cpp ^
  backend/api/HttpServer.cpp ^
  backend/database/DatabaseManager.cpp ^
  backend/cli/BankingApp.cpp ^
  backend/main.cpp ^
  -lws2_32 -ladvapi32 ^
  -o SmartBankingSystem.exe

if %ERRORLEVEL% equ 0 (
    echo Build Successful: SmartBankingSystem.exe generated.
) else (
    echo Build Failed! Check errors above.
)
