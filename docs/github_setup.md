# GitHub Setup & Repository Publication Guide

This document provides a step-by-step guide for publishing the **Smart Banking Management System** to GitHub, configuring repository metadata, and managing ongoing code changes.

---

## 1. Prerequisites

Before publishing the repository, ensure you have:
1. **Git Installed**: Download and install [Git for Windows](https://git-scm.com/download/win) (includes Git Bash).
2. **GitHub Account**: Sign up at [github.com](https://github.com) if you do not already have an account.
3. **C++17 Compiler**: MinGW-w64 (`g++`) with Win32 threads or MSVC (already verified on your workstation).

Verify your Git installation in PowerShell or Git Bash:
```bash
git --version
```

If this is your first time using Git on this machine, configure your identity:
```bash
git config --global user.name "Your Full Name"
git config --global user.email "your.email@example.com"
```

---

## 2. Project Directory Verification

Open PowerShell or Git Bash and navigate to the project root directory:
```bash
cd path/to/SmartBankingSystem
```

Confirm that the directory structure matches the expected repository layout:
```text
SmartBankingSystem/
│
├── backend/                  # C++17 source files and headers (api, cli, core, database, storage)
├── frontend/                 # React 18 single-page application (index.html, App.js, api.js, style.css)
├── data/                     # Data stores (customers.txt, accounts.txt, transactions.txt, schema.sql)
├── docs/                     # Comprehensive project documentation
│   ├── diagrams/             # System, auth, DSA, and DB architecture SVG diagrams & viewer
│   ├── presentation/         # College project presentation slides (MD, HTML, PDF)
│   ├── screenshots/          # UI screenshots directory (.gitkeep)
│   ├── dsa_documentation.md  # Detailed DSA implementation & complexity analysis
│   ├── database_documentation.md # MySQL 3NF database schema documentation
│   └── github_setup.md       # This Git setup guide
├── .gitignore                # Git exclusions (binaries, build artifacts, logs, secrets)
├── CMakeLists.txt            # Cross-platform CMake build configuration
├── build.bat                 # Windows MinGW-w64 build script
├── LICENSE                   # MIT License
└── README.md                 # Root documentation and project overview
```

---

## 3. Initializing the Local Git Repository

### Step 3.1: Initialize Git
Run the following command in the project root:
```bash
git init
```

### Step 3.2: Inspect Repository Status
Check which files are recognized and confirm that build artifacts and binaries are ignored:
```bash
git status
```

> **Note on `.gitignore`**:
> The `.gitignore` file automatically excludes:
> - Compiled executables (`SmartBankingSystem.exe`, `*.exe`)
> - Object files (`*.obj`, `*.o`, `*.a`, `*.lib`)
> - Build directories (`build/`, `bin/`, `CMakeFiles/`, `CMakeCache.txt`)
> - Crash logs and temporary test sandboxes (`*.log`, `*.tmp`, `data/*.tmp`)
> - IDE files (`.vscode/`, `.idea/`, `.vs/`)
>
> High-value assets such as architecture SVGs, SQL schemas, presentation decks, and text persistence files are preserved and tracked.

---

## 4. Staging and Committing Code

### Step 4.1: Stage All Tracked Files
```bash
git add .
```

### Step 4.2: Verify Staged Changes
```bash
git status
```
Ensure that no `.exe` or temporary `.tmp` files are in the "Changes to be committed" list.

### Step 4.3: Create Initial Commit
```bash
git commit -m "feat: initial commit of Smart Banking Management System"
```

### Step 4.4: Set Default Branch to `main`
```bash
git branch -M main
```

---

## 5. Creating the GitHub Remote Repository

1. Log into your GitHub account: [https://github.com](https://github.com).
2. Click the **`+`** icon in the top right corner and select **New repository**.
3. Fill in the repository details:
   - **Repository name**: `SmartBankingSystem` (or `smart-banking-management-system`)
   - **Description**: `A full-stack educational banking management system featuring a C++17 REST API engine, OOP, DSA, cryptographic PIN hashing, MySQL 3NF schema, and a React 18 frontend.`
   - **Visibility**: Select **Public** (or **Private** depending on submission guidelines).
   - **Initialize this repository with**: Leave **UNCHECKED** (do NOT check "Add a README file", "Add .gitignore", or "Choose a license" — these already exist locally).
4. Click **Create repository**.

---

## 6. Linking Remote and Pushing Code

Copy the repository URL provided by GitHub (HTTPS or SSH), then run:

### Using HTTPS:
```bash
git remote add origin https://github.com/<YOUR_GITHUB_USERNAME>/SmartBankingSystem.git
git push -u origin main
```

*(You will be prompted to authenticate via GitHub Personal Access Token or browser login).*

### Using SSH:
```bash
git remote add origin git@github.com:<YOUR_GITHUB_USERNAME>/SmartBankingSystem.git
git push -u origin main
```

---

## 7. Recommended GitHub Repository Metadata

To optimize the repository presentation for recruiters, college evaluators, and the open-source community, configure the following settings on your GitHub repository page:

### About Section
- **Description**:
  > A full-stack educational banking management system featuring a C++17 REST API engine, OOP, DSA, cryptographic PIN hashing, MySQL 3NF schema, and a React 18 frontend.
- **Website**: Link to your live GitHub Pages or demo portfolio (optional).
- **Topics / Tags**:
  ```text
  cpp
  cpp17
  banking-system
  rest-api
  react18
  mysql
  data-structures
  algorithms
  graph-algorithms
  cryptography
  winsock
  college-project
  object-oriented-programming
  ```

---

## 8. Ongoing Development Workflow

When making modifications or adding new features in the future, follow this standard Git workflow:

```bash
# 1. Check current status
git status

# 2. Stage modified files
git add .

# 3. Commit with a conventional descriptive message
git commit -m "docs: add presentation demo recording"

# 4. Push updates to GitHub
git push origin main
```

### Common Conventional Commit Types:
- `feat:` A new feature (e.g., `feat: implement recurring deposit calculator`)
- `fix:` A bug fix (e.g., `fix: correct balance formatting in transfer response`)
- `docs:` Documentation updates (e.g., `docs: update DSA complexity table in README`)
- `style:` Code style, formatting, or UI aesthetic improvements
- `refactor:` Code restructuring without changing functional behavior
- `test:` Adding or updating automated test suites

---

## 9. Security and Demo Notice

- **Zero Secret Exposure**: This repository contains **no** private cryptographic keys, database passwords, or personal credentials.
- **Demo Credentials**:
  - Admin REST API Key: `X-Admin-Key: admin123` *(configurable in `HttpServer.h` / `main.cpp`)*
  - Customer Accounts: `CUST-101` (PIN: `1234`), `CUST-102` (PIN: `1234`)
  - All customer PINs in `data/customers.txt` are cryptographically stored as salted SHA-256 / PBKDF2 hashes.
