const canvas = document.getElementById('warehouseCanvas');
const ctx = canvas.getContext('2d');

// --- Layout Configuration ---
const nodes = [
    { id: 0, x: 50, y: 175, label: 'Packing' },
    { id: 1, x: 150, y: 100, label: 'Sec 1' },
    { id: 2, x: 150, y: 250, label: 'Sec 2' },
    { id: 3, x: 300, y: 50, label: 'Shelf 3' },
    { id: 4, x: 300, y: 125, label: 'Shelf 4' },
    { id: 5, x: 300, y: 225, label: 'Shelf 5' },
    { id: 6, x: 300, y: 300, label: 'Shelf 6' },
    { id: 7, x: 450, y: 75, label: 'Lap/Keyb' }, // Nodes 7
    { id: 8, x: 450, y: 225, label: 'Mon/Mouse' }, // Nodes 8
    { id: 9, x: 550, y: 300, label: 'Headph' }   // Node 9
];

const edges = [
    [0, 1], [0, 2], [1, 3], [1, 4], [2, 5], [2, 6],
    [4, 7], [5, 8], [6, 9], [3, 7], [8, 9]
];

// --- State ---
let pendingOrders = [];
let dispatchedOrders = [];
let inventory = [];
let catalog = [];
let activePath = [];

function renderGraph() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Draw edges
    ctx.lineWidth = 2;
    edges.forEach(edge => {
        const u = nodes.find(n => n.id === edge[0]);
        const v = nodes.find(n => n.id === edge[1]);

        ctx.strokeStyle = '#334155';
        ctx.beginPath();
        ctx.moveTo(u.x, u.y);
        ctx.lineTo(v.x, v.y);
        ctx.stroke();
    });

    // Draw active path if any
    if (activePath.length > 1) {
        ctx.lineWidth = 4;
        ctx.strokeStyle = '#38bdf8';
        ctx.shadowBlur = 10;
        ctx.shadowColor = '#38bdf8';

        ctx.beginPath();
        const start = nodes.find(n => n.id === activePath[0]);
        ctx.moveTo(start.x, start.y);

        for (let i = 1; i < activePath.length; i++) {
            const next = nodes.find(n => n.id === activePath[i]);
            ctx.lineTo(next.x, next.y);
        }
        ctx.stroke();
        ctx.shadowBlur = 0;
    }

    // Draw nodes
    nodes.forEach(node => {
        ctx.beginPath();
        ctx.arc(node.x, node.y, 8, 0, Math.PI * 2);

        // Color active nodes
        if (activePath.includes(node.id)) {
            ctx.fillStyle = '#38bdf8';
            ctx.shadowBlur = 15;
            ctx.shadowColor = '#38bdf8';
        } else {
            ctx.fillStyle = '#94a3b8';
            ctx.shadowBlur = 0;
        }

        ctx.fill();

        // Labels
        ctx.fillStyle = '#cbd5e1';
        ctx.font = '10px Arial';
        ctx.textAlign = 'center';
        ctx.fillText(node.label, node.x, node.y - 15);
    });
}

// --- API Interaction ---
async function apiCall(command) {
    try {
        const res = await fetch('/api/command', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ command })
        });
        return await res.json();
    } catch (e) {
        log("Connection error: " + e.message);
        return { status: 'error' };
    }
}

async function fetchState() {
    try {
        const res = await fetch('/api/state');
        if (!res.ok) return;
        const data = await res.json();

        if (data.status === 'success') {
            pendingOrders = data.pending || [];
            dispatchedOrders = data.dispatched || [];
            inventory = data.inventory || [];
            catalog = data.catalog || [];
            updateUI();
        }
    } catch (e) {
        console.error("State sync failed", e);
    }
}

function log(msg) {
    const box = document.getElementById('log-display');
    const p = document.createElement('p');
    p.className = 'log-entry';
    p.innerText = `> ${msg}`;
    box.prepend(p);
}

async function addOrder() {
    const id = document.getElementById('prodId').value;
    const qty = document.getElementById('qty').value;
    const prio = document.getElementById('prio').value;

    const res = await apiCall(`ADD_ORDER ${id} ${qty} ${prio}`);
    log(res.msg);

    // Sync state
    await fetchState();
}

async function processOrder() {
    const res = await apiCall('PROCESS');
    log(res.msg);
    if (res.status === 'success') {
        activePath = [0, 1, 4, 7];
        renderGraph();
        setTimeout(() => { activePath = []; renderGraph(); }, 2000);

        await fetchState();
    }
}

async function dispatchOrder() {
    const res = await apiCall('DISPATCH');
    log(res.msg);
    await fetchState();
}

async function undoAction() {
    const res = await apiCall('UNDO');
    log(res.msg);
    await fetchState();
}

function updateUI() {
    const heapList = document.getElementById('heap-list');
    heapList.innerHTML = '';

    if (Array.isArray(pendingOrders)) {
        pendingOrders.forEach(o => {
            const div = document.createElement('div');
            div.className = 'order-item high-prio';
            div.innerText = o.text;
            heapList.appendChild(div);
        });
    }

    const queueList = document.getElementById('queue-list');
    queueList.innerHTML = '';

    if (Array.isArray(dispatchedOrders)) {
        dispatchedOrders.forEach(o => {
            const div = document.createElement('div');
            div.className = 'order-item dispatched';
            div.innerText = o.text;
            queueList.appendChild(div);
        });
    }

    const invList = document.getElementById('inventory-list');
    if (invList) {
        invList.innerHTML = '';
        if (Array.isArray(inventory)) {
            inventory.forEach(item => {
                const div = document.createElement('div');
                div.className = 'inv-item';
                div.innerHTML = `<strong>${item.name}</strong>ID: ${item.id}<br>Qty: ${item.qty}<br>Node: ${item.loc}`;
                invList.appendChild(div);
            });
        }
    }

    const catList = document.getElementById('catalog-list');
    if (catList) {
        catList.innerHTML = '';
        if (Array.isArray(catalog)) {
            catalog.forEach(prod => {
                const div = document.createElement('div');
                div.className = 'cat-item';
                div.innerHTML = `<span>${prod.name} (${prod.cat})</span><span>$${prod.price}</span>`;
                catList.appendChild(div);
            });
        }
    }
}

// Init
renderGraph();
setInterval(fetchState, 5000);
fetchState(); 
