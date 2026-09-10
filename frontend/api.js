// ============================================================================
// SMART BANKING REST API CLIENT SERVICE
// Connects React Web Frontend directly to the C++17 Winsock Backend
// ============================================================================

const API_BASE = window.location.origin;
const DEFAULT_ADMIN_KEY = 'admin123';

const api = {
    // Educational Admin Key configuration
    getAdminKey() {
        return sessionStorage.getItem('smart_banking_admin_key') || DEFAULT_ADMIN_KEY;
    },

    setAdminKey(key) {
        if (key) {
            sessionStorage.setItem('smart_banking_admin_key', key);
        } else {
            sessionStorage.removeItem('smart_banking_admin_key');
        }
    },

    async request(endpoint, options = {}) {
        const url = `${API_BASE}${endpoint}`;
        const defaultHeaders = {
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        };

        const headers = {
            ...defaultHeaders,
            ...options.headers
        };

        // Automatically attach X-Admin-Key for administrative routes (/api/admin/*)
        if (endpoint.startsWith('/api/admin/')) {
            headers['X-Admin-Key'] = this.getAdminKey();
        }

        const config = {
            ...options,
            headers
        };

        try {
            const response = await fetch(url, config);
            const data = await response.json();
            
            if (!response.ok) {
                throw new Error(data.message || `HTTP error ${response.status}`);
            }
            return data;
        } catch (error) {
            console.error(`[API Error] ${options.method || 'GET'} ${endpoint}:`, error);
            throw error;
        }
    },

    // 1. Authentication
    async login(customerId, pin) {
        return this.request('/api/auth/login', {
            method: 'POST',
            body: JSON.stringify({ customerId, pin })
        });
    },

    async registerCustomer(fullName, email, phone, pin) {
        return this.request('/api/auth/register', {
            method: 'POST',
            body: JSON.stringify({ fullName, email, phone, pin })
        });
    },

    // 2. Accounts
    async getAccounts(customerId = '') {
        const query = customerId ? `?customerId=${encodeURIComponent(customerId)}` : '';
        return this.request(`/api/accounts${query}`);
    },

    async getAccount(accountNumber) {
        return this.request(`/api/accounts/${encodeURIComponent(accountNumber)}`);
    },

    async deposit(accountNumber, amount, description = 'Online Deposit') {
        return this.request('/api/accounts/deposit', {
            method: 'POST',
            body: JSON.stringify({ accountNumber, amount: parseFloat(amount), description })
        });
    },

    async withdraw(accountNumber, amount, description = 'Online Withdrawal') {
        return this.request('/api/accounts/withdraw', {
            method: 'POST',
            body: JSON.stringify({ accountNumber, amount: parseFloat(amount), description })
        });
    },

    async transfer(fromAccount, toAccount, amount, description = 'P2P Fund Transfer') {
        return this.request('/api/accounts/transfer', {
            method: 'POST',
            body: JSON.stringify({ fromAccount, toAccount, amount: parseFloat(amount), description })
        });
    },

    // 3. Transactions & Statement
    async getTransactions(accountNumber = '') {
        const query = accountNumber ? `?accountNumber=${encodeURIComponent(accountNumber)}` : '';
        return this.request(`/api/transactions${query}`);
    },

    // 4. Analytics & Health
    async getFinancialHealth(accountNumber) {
        return this.request(`/api/analytics/health/${encodeURIComponent(accountNumber)}`);
    },

    async getCategories() {
        return this.request('/api/analytics/categories');
    },

    // 5. Admin Operations (Queue, Undo Stack, Graph)
    async getAdminRequests() {
        return this.request('/api/admin/requests');
    },

    async processAdminRequest(approve = true) {
        return this.request('/api/admin/process-request', {
            method: 'POST',
            body: JSON.stringify({ approve })
        });
    },

    async undoAdminTransaction() {
        return this.request('/api/admin/undo', {
            method: 'POST',
            body: JSON.stringify({})
        });
    },

    async getNetworkGraph() {
        return this.request('/api/admin/network-graph');
    }
};

window.api = api;
