const { useState, useEffect } = React;

// ============================================================================
// AML TRANSACTION NETWORK VISUALIZER COMPONENT
// Directed Adjacency Graph & DFS Cycle Detection (AML Forensic Engine)
// ============================================================================
function AmlNetworkVisualizer({ networkGraph, onRefresh }) {
    const [selectedItem, setSelectedItem] = useState(null);

    const vertices = networkGraph?.vertices || [];
    const edges = networkGraph?.edges || [];
    const cycles = networkGraph?.cycles || [];

    // Parse cycle edges and nodes computed by C++ backend
    const cycleEdgeSet = new Set();
    const cycleNodeSet = new Set();

    cycles.forEach(cycle => {
        for (let i = 0; i < cycle.length - 1; i++) {
            const from = cycle[i];
            const to = cycle[i + 1];
            cycleEdgeSet.add(`${from}->${to}`);
            cycleNodeSet.add(from);
            cycleNodeSet.add(to);
        }
    });

    // Compute coordinate layout for vertices (elliptical distribution)
    const nodeCoords = {};
    const totalNodes = vertices.length;
    const centerX = 380;
    const centerY = 220;

    if (totalNodes === 1) {
        nodeCoords[vertices[0]] = { x: centerX, y: centerY };
    } else if (totalNodes === 2) {
        nodeCoords[vertices[0]] = { x: 230, y: centerY };
        nodeCoords[vertices[1]] = { x: 530, y: centerY };
    } else if (totalNodes > 2) {
        const radiusX = Math.min(270, 180 + totalNodes * 10);
        const radiusY = Math.min(145, 100 + totalNodes * 6);
        vertices.forEach((v, idx) => {
            const angle = (2 * Math.PI * idx) / totalNodes - Math.PI / 2;
            nodeCoords[v] = {
                x: Math.round(centerX + radiusX * Math.cos(angle)),
                y: Math.round(centerY + radiusY * Math.sin(angle))
            };
        });
    }

    // Set of edges to detect bidirectional flows and curve appropriately
    const edgeKeySet = new Set(edges.map(e => `${e.from}->${e.to}`));

    return (
        <div className="table-container" style={{ marginTop: '2rem' }}>
            <div className="table-header" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                <div>
                    <h2>AML Transaction Network (Directed Graph & DFS Cycle Detection)</h2>
                    <p style={{ color: '#94a3b8', fontSize: '0.85rem', margin: '0.25rem 0 0 0' }}>
                        Forensic fund flow analysis and anti-money laundering circular routing topology
                    </p>
                </div>
                {onRefresh && (
                    <button 
                        className="btn btn-secondary" 
                        style={{ padding: '0.4rem 0.85rem', fontSize: '0.8rem' }}
                        onClick={onRefresh}
                    >
                        ↻ Refresh Graph
                    </button>
                )}
            </div>

            {/* Educational / System Explanation Box */}
            <div className="aml-info-panel">
                <p>
                    <strong>Topological DSA Architecture:</strong> Accounts are represented as vertices and fund transfers as directed edges. DFS cycle detection identifies circular transaction paths that may warrant further review.
                </p>
                <small>
                    C++ Engine: <code>TransactionGraph</code> adjacency list <code>std::unordered_map&lt;std::string, std::vector&lt;TransferEdge&gt;&gt;</code> with recursive backtracking cycle detection.
                </small>
            </div>

            {/* Summary Metrics Grid */}
            <div className="aml-stats-grid">
                <div className="aml-stat-box">
                    <div className="aml-stat-num">{vertices.length}</div>
                    <div className="aml-stat-label">Network Vertices (Accounts)</div>
                </div>
                <div className="aml-stat-box">
                    <div className="aml-stat-num" style={{ color: '#38bdf8' }}>{edges.length}</div>
                    <div className="aml-stat-label">Directed Edges (Transfers)</div>
                </div>
                <div className="aml-stat-box">
                    <div className="aml-stat-num" style={{ color: cycles.length > 0 ? '#f43f5e' : '#10b981' }}>
                        {cycles.length}
                    </div>
                    <div className="aml-stat-label">Circular Routing Rings (Cycles)</div>
                </div>
            </div>

            {/* Cycle Alert Status Card */}
            {cycles.length > 0 ? (
                <div className="aml-alert-card warning">
                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.6rem', fontWeight: '700', fontSize: '0.95rem' }}>
                        <span>⚠️</span>
                        <span>Potential Circular Layering Ring(s) Detected ({cycles.length})</span>
                    </div>
                    <div style={{ fontSize: '0.85rem', color: '#fca5a5' }}>
                        The DFS cycle detection algorithm detected closed-loop fund routing across the following account paths:
                    </div>
                    <div style={{ display: 'flex', gap: '0.5rem', flexWrap: 'wrap', marginTop: '0.4rem' }}>
                        {cycles.map((c, idx) => (
                            <div key={idx} className="aml-cycle-pill">
                                <span>Loop #{idx + 1}:</span>
                                <strong>{c.join(' ➔ ')}</strong>
                            </div>
                        ))}
                    </div>
                </div>
            ) : (
                <div className="aml-alert-card safe">
                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.6rem', fontWeight: '600', fontSize: '0.9rem' }}>
                        <span>✓</span>
                        <span>Transaction Topology Acyclic (Clean)</span>
                    </div>
                    <div style={{ fontSize: '0.8rem', color: '#6ee7b7' }}>
                        No circular routing loops detected across current inter-account transfer edges.
                    </div>
                </div>
            )}

            {/* Visual SVG Network Canvas */}
            <div className="aml-canvas-container">
                {vertices.length === 0 ? (
                    <div style={{ textAlign: 'center', color: '#94a3b8', padding: '3rem 1rem' }}>
                        <div style={{ fontSize: '2.5rem', marginBottom: '0.5rem' }}>🕸️</div>
                        <div style={{ fontWeight: '600', color: '#e2e8f0', marginBottom: '0.25rem' }}>
                            No Inter-Account Transfer Relationships Recorded
                        </div>
                        <p style={{ fontSize: '0.85rem', maxWidth: '420px', margin: '0 auto' }}>
                            Initiate a fund transfer between customer accounts to populate graph vertices and directed edges.
                        </p>
                    </div>
                ) : (
                    <svg className="aml-svg" viewBox="0 0 760 440" xmlns="http://www.w3.org/2000/svg">
                        <defs>
                            {/* Arrowhead marker for normal edges */}
                            <marker
                                id="aml-arrow-normal"
                                viewBox="0 0 10 10"
                                refX="8"
                                refY="5"
                                markerWidth="6"
                                markerHeight="6"
                                orient="auto-start-reverse"
                            >
                                <path d="M 0 1 L 10 5 L 0 9 z" fill="#38bdf8" />
                            </marker>

                            {/* Arrowhead marker for cycle/flagged edges */}
                            <marker
                                id="aml-arrow-cycle"
                                viewBox="0 0 10 10"
                                refX="8"
                                refY="5"
                                markerWidth="7"
                                markerHeight="7"
                                orient="auto-start-reverse"
                            >
                                <path d="M 0 1 L 10 5 L 0 9 z" fill="#f43f5e" />
                            </marker>

                            {/* Radial gradients for node backgrounds */}
                            <radialGradient id="node-gradient-normal" cx="40%" cy="40%" r="60%">
                                <stop offset="0%" stopColor="#1e293b" />
                                <stop offset="100%" stopColor="#0f172a" />
                            </radialGradient>
                            <radialGradient id="node-gradient-cycle" cx="40%" cy="40%" r="60%">
                                <stop offset="0%" stopColor="#4c0519" />
                                <stop offset="100%" stopColor="#1f121b" />
                            </radialGradient>
                        </defs>

                        {/* Background subtle grid dots */}
                        <g opacity="0.12">
                            {Array.from({ length: 19 }).map((_, xi) =>
                                Array.from({ length: 11 }).map((_, yi) => (
                                    <circle key={`${xi}-${yi}`} cx={xi * 40 + 20} cy={yi * 40 + 20} r="1" fill="#94a3b8" />
                                ))
                            )}
                        </g>

                        {/* Render Directed Edges */}
                        <g className="edges-layer">
                            {edges.map((edge, idx) => {
                                const fromCoord = nodeCoords[edge.from];
                                const toCoord = nodeCoords[edge.to];
                                if (!fromCoord || !toCoord) return null;

                                const isCycleEdge = cycleEdgeSet.has(`${edge.from}->${edge.to}`);
                                const hasReverse = edgeKeySet.has(`${edge.to}->${edge.from}`);

                                const dx = toCoord.x - fromCoord.x;
                                const dy = toCoord.y - fromCoord.y;
                                const dist = Math.sqrt(dx * dx + dy * dy) || 1;

                                const ux = dx / dist;
                                const uy = dy / dist;
                                const nx = -uy;
                                const ny = ux;

                                // Offset curve perpendicular to line to avoid overlap
                                const curveOffset = hasReverse ? 38 : (dist > 250 ? 24 : 14);
                                const ctrlX = (fromCoord.x + toCoord.x) / 2 + nx * curveOffset;
                                const ctrlY = (fromCoord.y + toCoord.y) / 2 + ny * curveOffset;

                                // Boundary radius for node circle
                                const nodeRadius = 26;
                                const cFromDx = ctrlX - fromCoord.x;
                                const cFromDy = ctrlY - fromCoord.y;
                                const cFromDist = Math.sqrt(cFromDx * cFromDx + cFromDy * cFromDy) || 1;
                                const startX = fromCoord.x + (cFromDx / cFromDist) * nodeRadius;
                                const startY = fromCoord.y + (cFromDy / cFromDist) * nodeRadius;

                                const cToDx = ctrlX - toCoord.x;
                                const cToDy = ctrlY - toCoord.y;
                                const cToDist = Math.sqrt(cToDx * cToDx + cToDy * cToDist) || 1;
                                const endX = toCoord.x + (cToDx / cToDist) * (nodeRadius + 7);
                                const endY = toCoord.y + (cToDy / cToDist) * (nodeRadius + 7);

                                // Midpoint for amount label (t = 0.5)
                                const labelX = 0.25 * startX + 0.5 * ctrlX + 0.25 * endX;
                                const labelY = 0.25 * startY + 0.5 * ctrlY + 0.25 * endY;

                                const isSelected = selectedItem === `${edge.from}->${edge.to}`;

                                return (
                                    <g 
                                        key={`edge-${edge.from}-${edge.to}-${edge.txnId || idx}`}
                                        onClick={() => setSelectedItem(isSelected ? null : `${edge.from}->${edge.to}`)}
                                        style={{ cursor: 'pointer' }}
                                    >
                                        {/* Invisible wider hit area for easy hover/click */}
                                        <path
                                            d={`M ${startX} ${startY} Q ${ctrlX} ${ctrlY} ${endX} ${endY}`}
                                            fill="none"
                                            stroke="transparent"
                                            strokeWidth="18"
                                        />
                                        {/* Directed visible path */}
                                        <path
                                            d={`M ${startX} ${startY} Q ${ctrlX} ${ctrlY} ${endX} ${endY}`}
                                            fill="none"
                                            stroke={isCycleEdge ? '#f43f5e' : (isSelected ? '#60a5fa' : '#38bdf8')}
                                            strokeWidth={isCycleEdge ? (isSelected ? 3.5 : 2.5) : (isSelected ? 3 : 1.8)}
                                            strokeDasharray={isCycleEdge ? '5 3' : 'none'}
                                            markerEnd={isCycleEdge ? 'url(#aml-arrow-cycle)' : 'url(#aml-arrow-normal)'}
                                            style={{ transition: 'stroke-width 0.2s, stroke 0.2s' }}
                                        />
                                        {/* Amount pill */}
                                        <g transform={`translate(${labelX}, ${labelY})`}>
                                            <rect
                                                x="-28"
                                                y="-9"
                                                width="56"
                                                height="18"
                                                rx="9"
                                                fill={isCycleEdge ? '#2b0c16' : '#0f172a'}
                                                stroke={isCycleEdge ? '#f43f5e' : '#334155'}
                                                strokeWidth="1"
                                            />
                                            <text
                                                x="0"
                                                y="3.5"
                                                textAnchor="middle"
                                                fontSize="9.5"
                                                fontWeight="600"
                                                fontFamily="monospace"
                                                fill={isCycleEdge ? '#fda4af' : '#94a3b8'}
                                            >
                                                ${edge.amount >= 1000 ? `${(edge.amount / 1000).toFixed(1)}k` : edge.amount.toFixed(0)}
                                            </text>
                                        </g>
                                    </g>
                                );
                            })}
                        </g>

                        {/* Render Graph Nodes (Accounts) */}
                        <g className="nodes-layer">
                            {vertices.map(accNo => {
                                const coord = nodeCoords[accNo];
                                if (!coord) return null;

                                const isCycleNode = cycleNodeSet.has(accNo);
                                const isSelected = selectedItem === accNo;

                                return (
                                    <g
                                        key={`node-${accNo}`}
                                        className="graph-node"
                                        transform={`translate(${coord.x}, ${coord.y})`}
                                        onClick={() => setSelectedItem(isSelected ? null : accNo)}
                                    >
                                        {/* Glowing halo for cycle nodes */}
                                        {isCycleNode && (
                                            <circle
                                                className="aml-cycle-pulse"
                                                r="34"
                                                fill="none"
                                                stroke="#f43f5e"
                                                strokeWidth="1.5"
                                                strokeDasharray="4 2"
                                            />
                                        )}
                                        {/* Main node circle */}
                                        <circle
                                            r="26"
                                            fill={isCycleNode ? 'url(#node-gradient-cycle)' : 'url(#node-gradient-normal)'}
                                            stroke={isCycleNode ? '#f43f5e' : (isSelected ? '#38bdf8' : '#475569')}
                                            strokeWidth={isCycleNode ? 2.5 : (isSelected ? 2.5 : 1.8)}
                                            style={{
                                                filter: isCycleNode 
                                                    ? 'drop-shadow(0 0 8px rgba(244,63,94,0.4))' 
                                                    : 'drop-shadow(0 2px 6px rgba(0,0,0,0.4))'
                                            }}
                                        />
                                        {/* Account Status Mini dot */}
                                        <circle
                                            cx="0"
                                            cy="-12"
                                            r="3"
                                            fill={isCycleNode ? '#f43f5e' : '#10b981'}
                                        />
                                        {/* Account Number Text */}
                                        <text
                                            x="0"
                                            y="5"
                                            textAnchor="middle"
                                            fontSize="9"
                                            fontWeight="700"
                                            fontFamily="monospace"
                                            fill={isCycleNode ? '#fff' : '#f1f5f9'}
                                        >
                                            {accNo}
                                        </text>
                                    </g>
                                );
                            })}
                        </g>
                    </svg>
                )}
            </div>

            {/* Fallback / Detail Transfer Ledger */}
            <div style={{ marginTop: '1rem' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '0.75rem' }}>
                    <h3 style={{ fontSize: '1rem', fontWeight: '600', color: '#e2e8f0' }}>
                        Directed Transfer Ledger & Forensic Flow Details
                    </h3>
                    <span style={{ fontSize: '0.8rem', color: '#94a3b8' }}>
                        {edges.length} Total Transfer Relationship(s)
                    </span>
                </div>

                {edges.length === 0 ? (
                    <div style={{ padding: '1.5rem', textAlign: 'center', color: '#94a3b8', background: 'rgba(15,23,42,0.4)', borderRadius: '8px' }}>
                        No fund transfer relationships currently recorded in graph.
                    </div>
                ) : (
                    <table>
                        <thead>
                            <tr>
                                <th>Source Account</th>
                                <th>Direction</th>
                                <th>Destination Account</th>
                                <th>Amount</th>
                                <th>Transaction ID</th>
                                <th>AML Status</th>
                            </tr>
                        </thead>
                        <tbody>
                            {edges.map((edge, idx) => {
                                const isCycle = cycleEdgeSet.has(`${edge.from}->${edge.to}`);
                                const rowKey = `${edge.from}-${edge.to}-${edge.txnId || idx}`;
                                const isRowSelected = selectedItem === `${edge.from}->${edge.to}` || selectedItem === edge.from || selectedItem === edge.to;

                                return (
                                    <tr 
                                        key={rowKey}
                                        style={{ 
                                            background: isRowSelected ? 'rgba(56, 189, 248, 0.08)' : (isCycle ? 'rgba(244, 63, 94, 0.05)' : 'transparent'),
                                            cursor: 'pointer'
                                        }}
                                        onClick={() => setSelectedItem(selectedItem === `${edge.from}->${edge.to}` ? null : `${edge.from}->${edge.to}`)}
                                    >
                                        <td style={{ fontFamily: 'monospace', fontWeight: '600' }}>
                                            {edge.from}
                                        </td>
                                        <td style={{ color: isCycle ? '#f43f5e' : '#38bdf8', fontWeight: '700' }}>
                                            ➔
                                        </td>
                                        <td style={{ fontFamily: 'monospace', fontWeight: '600' }}>
                                            {edge.to}
                                        </td>
                                        <td style={{ fontWeight: '700', color: isCycle ? '#fda4af' : '#10b981' }}>
                                            ${edge.amount.toFixed(2)}
                                        </td>
                                        <td style={{ fontFamily: 'monospace', fontSize: '0.8rem', color: '#94a3b8' }}>
                                            {edge.txnId || 'N/A'}
                                        </td>
                                        <td>
                                            {isCycle ? (
                                                <span className="badge badge-cycle">
                                                    ⚠️ CIRCULAR LOOP
                                                </span>
                                            ) : (
                                                <span className="badge badge-active">
                                                    ✓ NORMAL
                                                </span>
                                            )}
                                        </td>
                                    </tr>
                                );
                            })}
                        </tbody>
                    </table>
                )}
            </div>
        </div>
    );
}

function App() {
    const [currentPortal, setCurrentPortal] = useState('customer'); // 'customer' or 'admin'
    
    // Authentication & Customer Session State
    const [loggedInCustomer, setLoggedInCustomer] = useState(() => {
        try {
            const saved = localStorage.getItem('smart_banking_customer');
            return saved ? JSON.parse(saved) : null;
        } catch {
            return null;
        }
    });
    const [authMode, setAuthMode] = useState('login'); // 'login' or 'register'
    const [loginForm, setLoginForm] = useState({ customerId: '', pin: '' });
    const [registerForm, setRegisterForm] = useState({ fullName: '', email: '', phone: '', pin: '', confirmPin: '' });
    const [authLoading, setAuthLoading] = useState(false);
    const [authError, setAuthError] = useState('');
    const [authSuccess, setAuthSuccess] = useState('');

    // Banking State
    const [accounts, setAccounts] = useState([]);
    const [selectedAccount, setSelectedAccount] = useState(null);
    const [transactions, setTransactions] = useState([]);
    const [healthReport, setHealthReport] = useState(null);
    const [categories, setCategories] = useState({});
    const [adminRequests, setAdminRequests] = useState([]);
    const [networkGraph, setNetworkGraph] = useState(null);

    // Modal & Toast State
    const [activeModal, setActiveModal] = useState(null); // 'deposit', 'withdraw', 'transfer'
    const [modalData, setModalData] = useState({ amount: '', description: '', toAccount: '' });
    const [toast, setToast] = useState(null);
    const [backendOnline, setBackendOnline] = useState(true);

    const showToast = (message, isError = false) => {
        setToast({ message, isError });
        setTimeout(() => setToast(null), 4000);
    };

    // Load initial accounts according to portal and session
    const refreshData = async () => {
        try {
            if (currentPortal === 'customer') {
                if (loggedInCustomer) {
                    const accRes = await api.getAccounts(loggedInCustomer.customerId);
                    const userAccs = accRes.data || [];
                    setAccounts(userAccs);
                    if (userAccs.length > 0) {
                        if (!selectedAccount || !userAccs.some(a => a.accountNumber === selectedAccount.accountNumber)) {
                            setSelectedAccount(userAccs[0]);
                        } else {
                            const updated = userAccs.find(a => a.accountNumber === selectedAccount.accountNumber);
                            if (updated) setSelectedAccount(updated);
                        }
                    } else {
                        setSelectedAccount(null);
                    }
                } else {
                    setAccounts([]);
                    setSelectedAccount(null);
                }
            } else {
                // Admin Portal loads all system accounts
                const accRes = await api.getAccounts();
                if (accRes.data && accRes.data.length > 0) {
                    setAccounts(accRes.data);
                }
            }
            setBackendOnline(true);
        } catch (err) {
            setBackendOnline(false);
            showToast("Cannot connect to C++ REST Backend on port 8080", true);
        }
    };

    useEffect(() => {
        refreshData();
    }, [loggedInCustomer, currentPortal]);

    // Load transactions and health whenever selected account changes
    useEffect(() => {
        if (selectedAccount) {
            loadAccountDetails(selectedAccount.accountNumber);
        }
    }, [selectedAccount]);

    const loadAccountDetails = async (accNo) => {
        try {
            const txRes = await api.getTransactions(accNo);
            setTransactions(txRes.data || []);

            try {
                const healthRes = await api.getFinancialHealth(accNo);
                setHealthReport(healthRes.data);
            } catch (hErr) {
                setHealthReport(null);
            }
        } catch (err) {
            console.error("Failed loading account details", err);
        }
    };

    // Load Admin Data
    const loadAdminData = async () => {
        try {
            const [reqRes, catRes, graphRes] = await Promise.all([
                api.getAdminRequests(),
                api.getCategories(),
                api.getNetworkGraph()
            ]);
            setAdminRequests(reqRes.data || []);
            setCategories(catRes.data || {});
            setNetworkGraph(graphRes.data || null);
        } catch (err) {
            console.error("Error loading admin data", err);
            if (err.message && (err.message.includes('401') || err.message.includes('Unauthorized') || err.message.includes('UNAUTHORIZED'))) {
                showToast("Admin API Access Denied (401): Invalid or missing X-Admin-Key", true);
            }
        }
    };

    useEffect(() => {
        if (currentPortal === 'admin') {
            loadAdminData();
        }
    }, [currentPortal]);

    // Authentication Handlers
    const handleLogin = async (e) => {
        e.preventDefault();
        setAuthError('');
        setAuthSuccess('');

        const cid = loginForm.customerId.trim();
        const pin = loginForm.pin.trim();

        if (!cid || !pin) {
            setAuthError('Customer ID and PIN are required.');
            return;
        }

        setAuthLoading(true);
        try {
            const res = await api.login(cid, pin);
            const customer = res.data;
            setLoggedInCustomer(customer);
            localStorage.setItem('smart_banking_customer', JSON.stringify(customer));
            showToast(`Welcome back, ${customer.fullName}!`);
        } catch (err) {
            setAuthError(err.message || 'Login failed. Please verify your Customer ID and PIN.');
        } finally {
            setAuthLoading(false);
        }
    };

    const handleRegister = async (e) => {
        e.preventDefault();
        setAuthError('');
        setAuthSuccess('');

        const { fullName, email, phone, pin, confirmPin } = registerForm;

        if (!fullName.trim() || !email.trim() || !phone.trim() || !pin) {
            setAuthError('All fields are required.');
            return;
        }

        if (pin !== confirmPin) {
            setAuthError('PIN and Confirm PIN do not match.');
            return;
        }

        if (pin.length < 4) {
            setAuthError('PIN must be at least 4 characters/digits.');
            return;
        }

        setAuthLoading(true);
        try {
            const res = await api.registerCustomer(fullName.trim(), email.trim(), phone.trim(), pin.trim());
            const newCust = res.data;
            setAuthSuccess(`Registration successful! Your Customer ID is: ${newCust.customerId}. You can now sign in.`);
            showToast(`Account created for ${newCust.fullName}! Please log in.`);
            setLoginForm({ customerId: newCust.customerId, pin: '' });
            setRegisterForm({ fullName: '', email: '', phone: '', pin: '', confirmPin: '' });
            setAuthMode('login');
        } catch (err) {
            setAuthError(err.message || 'Registration failed. Email or phone number may already be registered.');
        } finally {
            setAuthLoading(false);
        }
    };

    const handleLogout = () => {
        setLoggedInCustomer(null);
        localStorage.removeItem('smart_banking_customer');
        setAccounts([]);
        setSelectedAccount(null);
        setTransactions([]);
        setHealthReport(null);
        setLoginForm({ customerId: '', pin: '' });
        setAuthError('');
        setAuthSuccess('');
        showToast('You have been logged out.');
    };

    const fillDemoCredentials = (cid, pin) => {
        setLoginForm({ customerId: cid, pin });
        setAuthError('');
        setAuthSuccess('');
    };

    // Transaction Actions
    const handleDeposit = async (e) => {
        e.preventDefault();
        try {
            await api.deposit(selectedAccount.accountNumber, modalData.amount, modalData.description);
            showToast(`Deposit of $${modalData.amount} successful!`);
            setActiveModal(null);
            setModalData({ amount: '', description: '', toAccount: '' });
            await refreshData();
            loadAccountDetails(selectedAccount.accountNumber);
        } catch (err) {
            showToast(err.message, true);
        }
    };

    const handleWithdraw = async (e) => {
        e.preventDefault();
        try {
            await api.withdraw(selectedAccount.accountNumber, modalData.amount, modalData.description);
            showToast(`Withdrawal of $${modalData.amount} completed.`);
            setActiveModal(null);
            setModalData({ amount: '', description: '', toAccount: '' });
            await refreshData();
            loadAccountDetails(selectedAccount.accountNumber);
        } catch (err) {
            showToast(err.message, true);
        }
    };

    const handleTransfer = async (e) => {
        e.preventDefault();
        try {
            await api.transfer(selectedAccount.accountNumber, modalData.toAccount, modalData.amount, modalData.description);
            showToast(`Transferred $${modalData.amount} to ${modalData.toAccount}!`);
            setActiveModal(null);
            setModalData({ amount: '', description: '', toAccount: '' });
            await refreshData();
            loadAccountDetails(selectedAccount.accountNumber);
            loadAdminData();
        } catch (err) {
            showToast(err.message, true);
        }
    };

    // Admin Handlers
    const handleProcessRequest = async (approve) => {
        try {
            const res = await api.processAdminRequest(approve);
            showToast(res.message);
            loadAdminData();
            refreshData();
        } catch (err) {
            showToast(err.message, true);
        }
    };

    const handleUndo = async () => {
        try {
            const res = await api.undoAdminTransaction();
            showToast(res.message);
            refreshData();
            loadAdminData();
            if (selectedAccount) loadAccountDetails(selectedAccount.accountNumber);
        } catch (err) {
            showToast(err.message, true);
        }
    };

    return (
        <div>
            {/* Top Navigation */}
            <header className="navbar">
                <div className="nav-brand">
                    <span className="brand-icon">C++</span>
                    <span>Smart Banking System</span>
                </div>
                <div className="nav-tabs">
                    <button 
                        className={`nav-btn ${currentPortal === 'customer' ? 'active' : ''}`}
                        onClick={() => setCurrentPortal('customer')}
                    >
                        Customer Portal
                    </button>
                    <button 
                        className={`nav-btn ${currentPortal === 'admin' ? 'active' : ''}`}
                        onClick={() => setCurrentPortal('admin')}
                    >
                        Admin Dashboard
                    </button>
                </div>
                <div style={{ display: 'flex', alignItems: 'center', gap: '1rem' }}>
                    {currentPortal === 'customer' && loggedInCustomer && (
                        <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem' }}>
                            <div className="user-profile-badge">
                                <span>👤</span>
                                <span>{loggedInCustomer.fullName}</span>
                                <span style={{ color: '#94a3b8', fontSize: '0.8rem' }}>({loggedInCustomer.customerId})</span>
                            </div>
                            <button className="btn-logout" onClick={handleLogout}>
                                Sign Out
                            </button>
                        </div>
                    )}
                    <div className="nav-status">
                        <span className="status-dot"></span>
                        <span>{backendOnline ? 'C++ Core Engine Online' : 'Backend Disconnected'}</span>
                    </div>
                </div>
            </header>

            {/* Main Content Area */}
            <main className="container">
                {currentPortal === 'customer' ? (
                    !loggedInCustomer ? (
                        /* Customer Login / Registration Screen */
                        <div className="auth-wrapper">
                            <div className="auth-card">
                                <div className="auth-header">
                                    <h2>Secure Banking Portal</h2>
                                    <p>Sign in to your account or register as a new banking customer</p>
                                </div>

                                <div className="auth-tabs">
                                    <button 
                                        className={`auth-tab-btn ${authMode === 'login' ? 'active' : ''}`}
                                        onClick={() => { setAuthMode('login'); setAuthError(''); setAuthSuccess(''); }}
                                    >
                                        Customer Login
                                    </button>
                                    <button 
                                        className={`auth-tab-btn ${authMode === 'register' ? 'active' : ''}`}
                                        onClick={() => { setAuthMode('register'); setAuthError(''); setAuthSuccess(''); }}
                                    >
                                        New Registration
                                    </button>
                                </div>

                                {authError && (
                                    <div className="auth-alert error">
                                        <span>⚠️</span>
                                        <span>{authError}</span>
                                    </div>
                                )}

                                {authSuccess && (
                                    <div className="auth-alert success">
                                        <span>✓</span>
                                        <span>{authSuccess}</span>
                                    </div>
                                )}

                                {authMode === 'login' ? (
                                    <div>
                                        {/* Demo Quick-Fill Credentials */}
                                        <div className="demo-box">
                                            <div className="demo-title">Quick Demo Logins (Pre-seeded Accounts)</div>
                                            <div className="demo-chips">
                                                <button 
                                                    type="button" 
                                                    className="demo-chip"
                                                    onClick={() => fillDemoCredentials('CUST-101', '1234')}
                                                >
                                                    Alice Vance (CUST-101 / 1234)
                                                </button>
                                                <button 
                                                    type="button" 
                                                    className="demo-chip"
                                                    onClick={() => fillDemoCredentials('CUST-102', '4321')}
                                                >
                                                    Bob Builder (CUST-102 / 4321)
                                                </button>
                                            </div>
                                        </div>

                                        <form onSubmit={handleLogin}>
                                            <div className="form-group">
                                                <label className="form-label">Customer ID</label>
                                                <input 
                                                    type="text" 
                                                    className="form-input"
                                                    placeholder="e.g. CUST-101"
                                                    required
                                                    value={loginForm.customerId}
                                                    onChange={(e) => setLoginForm({ ...loginForm, customerId: e.target.value })}
                                                />
                                            </div>
                                            <div className="form-group">
                                                <label className="form-label">Security PIN</label>
                                                <input 
                                                    type="password" 
                                                    className="form-input"
                                                    placeholder="Enter your PIN"
                                                    required
                                                    value={loginForm.pin}
                                                    onChange={(e) => setLoginForm({ ...loginForm, pin: e.target.value })}
                                                />
                                            </div>
                                            <button 
                                                type="submit" 
                                                className="btn btn-primary" 
                                                style={{ width: '100%', marginTop: '0.5rem' }}
                                                disabled={authLoading}
                                            >
                                                {authLoading ? 'Authenticating...' : 'Sign In to Banking'}
                                            </button>
                                        </form>

                                        <div style={{ textAlign: 'center', marginTop: '1.5rem', fontSize: '0.85rem', color: '#94a3b8' }}>
                                            Don't have a banking profile?{' '}
                                            <a 
                                                href="#register" 
                                                style={{ color: '#10b981', fontWeight: '600', textDecoration: 'none' }}
                                                onClick={(e) => { e.preventDefault(); setAuthMode('register'); setAuthError(''); }}
                                            >
                                                Register Here
                                            </a>
                                        </div>
                                    </div>
                                ) : (
                                    /* Registration Form */
                                    <div>
                                        <form onSubmit={handleRegister}>
                                            <div className="form-group">
                                                <label className="form-label">Full Name</label>
                                                <input 
                                                    type="text" 
                                                    className="form-input"
                                                    placeholder="e.g. Diana Prince"
                                                    required
                                                    value={registerForm.fullName}
                                                    onChange={(e) => setRegisterForm({ ...registerForm, fullName: e.target.value })}
                                                />
                                            </div>
                                            <div className="form-group">
                                                <label className="form-label">Email Address</label>
                                                <input 
                                                    type="email" 
                                                    className="form-input"
                                                    placeholder="e.g. diana@domain.com"
                                                    required
                                                    value={registerForm.email}
                                                    onChange={(e) => setRegisterForm({ ...registerForm, email: e.target.value })}
                                                />
                                            </div>
                                            <div className="form-group">
                                                <label className="form-label">Phone Number</label>
                                                <input 
                                                    type="text" 
                                                    className="form-input"
                                                    placeholder="e.g. +1-555-7777"
                                                    required
                                                    value={registerForm.phone}
                                                    onChange={(e) => setRegisterForm({ ...registerForm, phone: e.target.value })}
                                                />
                                            </div>
                                            <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '1rem' }}>
                                                <div className="form-group">
                                                    <label className="form-label">Create PIN</label>
                                                    <input 
                                                        type="password" 
                                                        className="form-input"
                                                        placeholder="4-digit PIN"
                                                        required
                                                        value={registerForm.pin}
                                                        onChange={(e) => setRegisterForm({ ...registerForm, pin: e.target.value })}
                                                    />
                                                </div>
                                                <div className="form-group">
                                                    <label className="form-label">Confirm PIN</label>
                                                    <input 
                                                        type="password" 
                                                        className="form-input"
                                                        placeholder="Re-enter PIN"
                                                        required
                                                        value={registerForm.confirmPin}
                                                        onChange={(e) => setRegisterForm({ ...registerForm, confirmPin: e.target.value })}
                                                    />
                                                </div>
                                            </div>
                                            <button 
                                                type="submit" 
                                                className="btn btn-primary" 
                                                style={{ width: '100%', marginTop: '0.5rem' }}
                                                disabled={authLoading}
                                            >
                                                {authLoading ? 'Registering Account...' : 'Open Customer Account'}
                                            </button>
                                        </form>

                                        <div style={{ textAlign: 'center', marginTop: '1.5rem', fontSize: '0.85rem', color: '#94a3b8' }}>
                                            Already an existing customer?{' '}
                                            <a 
                                                href="#login" 
                                                style={{ color: '#10b981', fontWeight: '600', textDecoration: 'none' }}
                                                onClick={(e) => { e.preventDefault(); setAuthMode('login'); setAuthError(''); }}
                                            >
                                                Sign In
                                            </a>
                                        </div>
                                    </div>
                                )}
                            </div>
                        </div>
                    ) : (
                        /* Logged-in Customer Banking Dashboard */
                        <div>
                            {/* Portal Header */}
                            <div className="portal-header">
                                <div className="portal-title">
                                    <h1>Customer Banking Dashboard</h1>
                                    <p>Welcome back, <strong>{loggedInCustomer.fullName}</strong> (ID: {loggedInCustomer.customerId})</p>
                                </div>
                                {accounts.length > 0 && (
                                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem' }}>
                                        <span style={{ fontSize: '0.9rem', color: '#94a3b8' }}>Select Account:</span>
                                        <select 
                                            className="form-select"
                                            style={{ width: 'auto', padding: '0.5rem 1rem' }}
                                            value={selectedAccount ? selectedAccount.accountNumber : ''}
                                            onChange={(e) => {
                                                const acc = accounts.find(a => a.accountNumber === e.target.value);
                                                if (acc) setSelectedAccount(acc);
                                            }}
                                        >
                                            {accounts.map(a => (
                                                <option key={a.accountNumber} value={a.accountNumber}>
                                                    {a.accountNumber} ({a.accountType})
                                                </option>
                                            ))}
                                        </select>
                                    </div>
                                )}
                            </div>

                            {accounts.length === 0 ? (
                                <div className="card" style={{ textAlign: 'center', padding: '3rem 2rem' }}>
                                    <div style={{ fontSize: '2.5rem', marginBottom: '1rem' }}>🏦</div>
                                    <h2 style={{ marginBottom: '0.5rem' }}>No Active Bank Accounts Linked</h2>
                                    <p style={{ color: '#94a3b8', maxWidth: '500px', margin: '0 auto 1.5rem' }}>
                                        Your customer profile is registered, but no checking or savings accounts are currently associated with your ID. 
                                        Please contact bank administration to open an account.
                                    </p>
                                    <button className="btn btn-secondary" onClick={handleLogout}>
                                        Switch Account / Log Out
                                    </button>
                                </div>
                            ) : (
                                <div>
                                    {/* Metric Cards */}
                                    {selectedAccount && (
                                        <div className="stats-grid">
                                            <div className="card">
                                                <div className="card-header">
                                                    <span className="card-label">Current Balance</span>
                                                    <span className={`badge ${selectedAccount.isActive ? 'badge-active' : 'badge-blocked'}`}>
                                                        {selectedAccount.isActive ? 'ACTIVE' : 'BLOCKED'}
                                                    </span>
                                                </div>
                                                <div className="card-value">${selectedAccount.balance.toFixed(2)}</div>
                                                <div className="card-subtext">
                                                    {selectedAccount.accountType === 'Savings Account' 
                                                        ? `Interest Rate: ${selectedAccount.interestRate}% (Min: $${selectedAccount.minimumBalance})`
                                                        : `Overdraft Limit: $${selectedAccount.overdraftLimit}`}
                                                </div>
                                            </div>

                                            <div className="card">
                                                <div className="card-header">
                                                    <span className="card-label">Total Purchasing Power</span>
                                                    <span className="badge badge-deposit">Liquid</span>
                                                </div>
                                                <div className="card-value">${selectedAccount.availableCredit.toFixed(2)}</div>
                                                <div className="card-subtext">Balance + Available Overdraft Credit</div>
                                            </div>

                                            {healthReport && (
                                                <div className="card">
                                                    <div className="card-header">
                                                        <span className="card-label">Smart Financial Wellness</span>
                                                        <span className="badge badge-active">{healthReport.statusRating}</span>
                                                    </div>
                                                    <div className="card-value">{healthReport.savingsRate.toFixed(1)}%</div>
                                                    <div className="card-subtext">{healthReport.recommendation}</div>
                                                </div>
                                            )}
                                        </div>
                                    )}

                                    {/* Action Buttons */}
                                    <div className="actions-bar">
                                        <button className="btn btn-primary" onClick={() => setActiveModal('deposit')}>
                                            + Deposit Money
                                        </button>
                                        <button className="btn btn-secondary" onClick={() => setActiveModal('withdraw')}>
                                            - Withdraw Funds
                                        </button>
                                        <button className="btn btn-secondary" onClick={() => setActiveModal('transfer')}>
                                            ⇄ Transfer to Account
                                        </button>
                                    </div>

                                    {/* Transaction Statement Table */}
                                    <div className="table-container">
                                        <div className="table-header">
                                            <h2>Account Transaction Statement</h2>
                                            <span style={{ color: '#94a3b8', fontSize: '0.85rem' }}>
                                                {transactions.length} Recorded Ledger Event(s)
                                            </span>
                                        </div>
                                        {transactions.length === 0 ? (
                                            <div style={{ padding: '2rem', textAlign: 'center', color: '#94a3b8' }}>
                                                No transactions found for this account.
                                            </div>
                                        ) : (
                                            <table>
                                                <thead>
                                                    <tr>
                                                        <th>Transaction ID</th>
                                                        <th>Date & Time</th>
                                                        <th>Type</th>
                                                        <th>Counterparty</th>
                                                        <th>Amount</th>
                                                        <th>Description</th>
                                                    </tr>
                                                </thead>
                                                <tbody>
                                                    {transactions.map(t => (
                                                        <tr key={t.transactionId}>
                                                            <td style={{ fontFamily: 'monospace', fontWeight: '600' }}>{t.transactionId}</td>
                                                            <td style={{ color: '#94a3b8' }}>{t.timestamp}</td>
                                                            <td>
                                                                <span className={`badge badge-${t.type.toLowerCase().includes('deposit') ? 'deposit' : t.type.toLowerCase().includes('withdraw') ? 'withdrawal' : 'transfer'}`}>
                                                                    {t.type}
                                                                </span>
                                                            </td>
                                                            <td>{t.fromAccount === (selectedAccount ? selectedAccount.accountNumber : '') ? t.toAccount : t.fromAccount}</td>
                                                            <td style={{ fontWeight: '700', color: t.type.includes('DEPOSIT') || t.type.includes('IN') ? '#10b981' : '#f43f5e' }}>
                                                                {t.type.includes('DEPOSIT') || t.type.includes('IN') ? '+' : '-'}${t.amount.toFixed(2)}
                                                            </td>
                                                            <td>{t.description}</td>
                                                        </tr>
                                                    ))}
                                                </tbody>
                                            </table>
                                        )}
                                    </div>
                                </div>
                            )}
                        </div>
                    )
                ) : (
                    /* Admin Dashboard View */
                    <div>
                        <div className="portal-header">
                            <div className="portal-title">
                                <div style={{ display: 'flex', alignItems: 'center', gap: '0.75rem', flexWrap: 'wrap' }}>
                                    <h1 style={{ margin: 0 }}>Admin & Operations Dashboard</h1>
                                    <span className="badge badge-active" style={{ fontSize: '0.75rem', fontWeight: '600' }}>
                                        🔒 Protected API (X-Admin-Key)
                                    </span>
                                </div>
                                <p style={{ marginTop: '0.35rem' }}>C++ Engine Monitoring: FIFO Queue, LIFO Stack, and AML Graph Network (Authorized Session)</p>
                            </div>
                            <button className="btn btn-danger" onClick={handleUndo}>
                                ↺ Undo Most Recent Transaction (LIFO Stack)
                            </button>
                        </div>

                        {/* Accounts Overview */}
                        <div className="table-container">
                            <div className="table-header">
                                <h2>All Registered System Accounts</h2>
                            </div>
                            <table>
                                <thead>
                                    <tr>
                                        <th>Account No</th>
                                        <th>Account Type</th>
                                        <th>Holder Name</th>
                                        <th>Balance</th>
                                        <th>Status</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    {accounts.map(a => (
                                        <tr key={a.accountNumber}>
                                            <td style={{ fontFamily: 'monospace', fontWeight: '600' }}>{a.accountNumber}</td>
                                            <td>{a.accountType}</td>
                                            <td>{a.holderName}</td>
                                            <td style={{ fontWeight: '700' }}>${a.balance.toFixed(2)}</td>
                                            <td>
                                                <span className={`badge ${a.isActive ? 'badge-active' : 'badge-blocked'}`}>
                                                    {a.isActive ? 'ACTIVE' : 'BLOCKED'}
                                                </span>
                                            </td>
                                        </tr>
                                    ))}
                                </tbody>
                            </table>
                        </div>

                        {/* Pending FIFO Queue */}
                        <div className="table-container">
                            <div className="table-header">
                                <h2>Pending Customer Request Queue (FIFO DSA)</h2>
                                <span style={{ color: '#94a3b8', fontSize: '0.85rem' }}>
                                    {adminRequests.length} Pending Item(s)
                                </span>
                            </div>
                            {adminRequests.length === 0 ? (
                                <div style={{ padding: '2rem', textAlign: 'center', color: '#94a3b8' }}>
                                    No requests currently pending in FIFO queue.
                                </div>
                            ) : (
                                <table>
                                    <thead>
                                        <tr>
                                            <th>Request ID</th>
                                            <th>Customer</th>
                                            <th>Account</th>
                                            <th>Type</th>
                                            <th>Amount</th>
                                            <th>Notes</th>
                                            <th>Actions</th>
                                        </tr>
                                    </thead>
                                    <tbody>
                                        {adminRequests.map((r, idx) => (
                                            <tr key={r.requestId}>
                                                <td style={{ fontFamily: 'monospace' }}>{r.requestId}</td>
                                                <td>{r.customerId}</td>
                                                <td>{r.accountNumber}</td>
                                                <td><span className="badge badge-transfer">{r.type}</span></td>
                                                <td>${r.amount.toFixed(2)}</td>
                                                <td>{r.notes}</td>
                                                <td>
                                                    {idx === 0 ? (
                                                        <div style={{ display: 'flex', gap: '0.5rem' }}>
                                                            <button 
                                                                className="btn btn-primary" 
                                                                style={{ padding: '0.35rem 0.75rem', fontSize: '0.8rem' }}
                                                                onClick={() => handleProcessRequest(true)}
                                                            >
                                                                Approve
                                                            </button>
                                                            <button 
                                                                className="btn btn-danger" 
                                                                style={{ padding: '0.35rem 0.75rem', fontSize: '0.8rem' }}
                                                                onClick={() => handleProcessRequest(false)}
                                                            >
                                                                Reject
                                                            </button>
                                                        </div>
                                                    ) : (
                                                        <span style={{ color: '#94a3b8', fontSize: '0.8rem' }}>Waiting in Queue</span>
                                                    )}
                                                </td>
                                            </tr>
                                        ))}
                                    </tbody>
                                </table>
                            )}
                        </div>

                        {/* AML Transaction Network (Graph & DFS Cycle Detection) */}
                        <AmlNetworkVisualizer 
                            networkGraph={networkGraph} 
                            onRefresh={loadAdminData} 
                        />

                        {/* Category Spending Summary (STL map) */}
                        <div className="table-container">
                            <div className="table-header">
                                <h2>Category Spending Summary (Red-Black Tree STL map)</h2>
                            </div>
                            <table>
                                <thead>
                                    <tr>
                                        <th>Category (Sorted In-Order)</th>
                                        <th>Aggregated Volume</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    {Object.entries(categories).map(([cat, total]) => (
                                        <tr key={cat}>
                                            <td style={{ fontWeight: '600' }}>{cat}</td>
                                            <td style={{ fontWeight: '700', color: '#10b981' }}>${total.toFixed(2)}</td>
                                        </tr>
                                    ))}
                                </tbody>
                            </table>
                        </div>
                    </div>
                )}
            </main>

            {/* Modals */}
            {activeModal && (
                <div className="modal-overlay" onClick={() => setActiveModal(null)}>
                    <div className="modal-content" onClick={(e) => e.stopPropagation()}>
                        <div className="modal-header">
                            <h3>
                                {activeModal === 'deposit' && 'Deposit Funds'}
                                {activeModal === 'withdraw' && 'Withdraw Funds'}
                                {activeModal === 'transfer' && 'Inter-Account Transfer'}
                            </h3>
                            <button className="close-btn" onClick={() => setActiveModal(null)}>&times;</button>
                        </div>
                        <form onSubmit={
                            activeModal === 'deposit' ? handleDeposit :
                            activeModal === 'withdraw' ? handleWithdraw : handleTransfer
                        }>
                            {activeModal === 'transfer' && (
                                <div className="form-group">
                                    <label className="form-label">Destination Account Number</label>
                                    <input 
                                        type="text" 
                                        className="form-input"
                                        placeholder="e.g. CUR-1002"
                                        required
                                        value={modalData.toAccount}
                                        onChange={(e) => setModalData({...modalData, toAccount: e.target.value})}
                                    />
                                </div>
                            )}
                            <div className="form-group">
                                <label className="form-label">Amount ($)</label>
                                <input 
                                    type="number" 
                                    step="0.01"
                                    min="0.01"
                                    className="form-input"
                                    placeholder="0.00"
                                    required
                                    value={modalData.amount}
                                    onChange={(e) => setModalData({...modalData, amount: e.target.value})}
                                />
                            </div>
                            <div className="form-group">
                                <label className="form-label">Description / Note</label>
                                <input 
                                    type="text" 
                                    className="form-input"
                                    placeholder="Optional note"
                                    value={modalData.description}
                                    onChange={(e) => setModalData({...modalData, description: e.target.value})}
                                />
                            </div>
                            <button type="submit" className="btn btn-primary" style={{ width: '100%', marginTop: '1rem' }}>
                                Confirm {activeModal.toUpperCase()}
                            </button>
                        </form>
                    </div>
                </div>
            )}

            {/* Toast Notifications */}
            {toast && (
                <div className={`toast ${toast.isError ? 'error' : ''}`}>
                    {toast.message}
                </div>
            )}
        </div>
    );
}

ReactDOM.createRoot(document.getElementById('root')).render(<App />);
