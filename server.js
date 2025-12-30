const express = require('express');
const { spawn } = require('child_process');
const path = require('path');
const bodyParser = require('body-parser');
const readline = require('readline');

const app = express();
const PORT = 3000;

app.use(express.static('public'));
app.use(bodyParser.json());

// Spawn C++ Backend
const warehouseProc = spawn(path.join(__dirname, '../warehouse_v2.exe'), ['--api']);

// Create Readable Interface for stdout
const rl = readline.createInterface({
    input: warehouseProc.stdout,
    terminal: false
});

let pendingResponseResolver = null;

const commandQueue = [];
let isProcessing = false;

// Listen for JSON lines from C++
rl.on('line', (line) => {
    line = line.trim();
    if (!line) return;

    console.log(`C++ Output: ${line}`);

    // Check if it looks like JSON
    if (line.startsWith('{')) {
        try {
            const data = JSON.parse(line);
            if (pendingResponseResolver) {
                pendingResponseResolver(data);
                // pendingResponseResolver = null; // Don't clear here, let processQueue handle it
            }
        } catch (e) {
            console.error('JSON Parse Error:', e);
        }
    }
});

warehouseProc.stderr.on('data', (data) => {
    console.error(`C++ Error: ${data}`);
});

warehouseProc.on('close', (code) => {
    console.log(`C++ process exited with code ${code}`);
});

// Process the next command in the queue
function processQueue() {
    if (isProcessing || commandQueue.length === 0) return;

    isProcessing = true;
    const { cmd, resolve, reject, expectResponse } = commandQueue.shift();

    // Set the resolver for the incoming line
    pendingResponseResolver = (data) => {
        resolve(data);
        pendingResponseResolver = null;
        isProcessing = false;
        setTimeout(processQueue, 10); // Check next
    };

    // Send to C++
    try {
        warehouseProc.stdin.write(cmd + '\n');

        if (!expectResponse) {
            pendingResponseResolver = null;
            isProcessing = false;
            resolve({});
            setTimeout(processQueue, 10);
        }
    } catch (e) {
        reject(e);
        isProcessing = false;
        processQueue();
    }
}

// Helper to send command to C++
function sendCommand(cmd, expectResponse = true) {
    return new Promise((resolve, reject) => {
        commandQueue.push({ cmd, resolve, reject, expectResponse });
        processQueue();
    });
}

app.post('/api/command', async (req, res) => {
    const { command } = req.body;
    console.log(`Run: ${command}`);

    try {
        const response = await sendCommand(command);
        res.json(response);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

app.get('/api/state', async (req, res) => {
    try {
        const response = await sendCommand("GET_STATE");
        res.json(response);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

app.listen(PORT, () => {
    console.log(`Server v3 (Queued) running at http://localhost:${PORT}`);
    console.log(`Connected to Smart Warehouse C++ Backend`);
});
