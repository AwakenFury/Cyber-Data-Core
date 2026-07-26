#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

const char* ssid = "CBOS_FORENSIC_POOL_NODE";
const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);
File uploadFile;

// HTML Content (PROGMEM flash compressed)
const char html_portal[] PROGMEM = R"rawliteral(



<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CYBER DATA CORE - Integrated Control Dashboard</title>
    <style>
        :root {
            --bg-color: #050a05;
            --panel-bg: rgba(10, 25, 10, 0.6);
            --primary-glow: #00ff66;
            --secondary-glow: #00cc55;
            --text-color: #d0ffd0;
            --text-dim: #7fbf7f;
            --text-muted: #4e7c4e;
            --border-color: #006622;
            --alert-color: #ff3333;
            --alert-glow: rgba(255, 51, 51, 0.4);
            --accent-blue: #00ccff;
            --accent-amber: #ffb000;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Courier New', Courier, monospace;
        }

        body {
            background-color: var(--bg-color);
            color: var(--text-color);
            padding: 20px;
            overflow-x: hidden;
            background-image: 
                linear-gradient(rgba(18, 40, 18, 0.05) 50%, transparent 50%),
                linear-gradient(90deg, rgba(18, 40, 18, 0.05) 50%, transparent 50%);
            background-size: 100% 4px, 4px 100%;
        }

        /* Full Screen Warning Diagnostics Layer Overlay */
        .hazard-overlay-screen {
            position: fixed; top: 0; left: 0; width: 100vw; height: 100vh;
            background-color: rgba(30, 0, 0, 0.85);
            border: 4px solid var(--alert-color);
            z-index: 10000; display: flex; flex-direction: column;
            justify-content: center; align-items: center; text-align: center;
            pointer-events: auto; animation: hazard-pulse-loop 1.4s infinite;
        }
        .hazard-box-inner h2 { font-size: 28px; color: var(--alert-color); text-shadow: 0 0 15px var(--alert-color); text-transform: uppercase; }
        .hazard-box-inner p { font-size: 14px; color: #ff9999; text-transform: uppercase; margin-top: 5px; }

        @keyframes hazard-pulse-loop {
            0%, 100% { opacity: 1; box-shadow: inset 0 0 50px rgba(255, 51, 51, 0.4); }
            50% { opacity: 0.85; box-shadow: inset 0 0 100px rgba(255, 51, 51, 0.8); border-color: #880000; }
        }

        .header {
            border-bottom: 2px solid var(--border-color);
            padding-bottom: 10px;
            margin-bottom: 20px;
            display: flex;
            justify-content: space-between;
            align-items: flex-end;
            text-shadow: 0 0 10px var(--primary-glow);
        }

        .header h1 {
            font-size: 24px;
            letter-spacing: 2px;
        }

        .ui-card {
            background: var(--panel-bg);
            border: 1px solid var(--border-color);
            border-radius: 4px;
            padding: 15px;
            box-shadow: inset 0 0 15px rgba(0, 255, 102, 0.05);
            position: relative;
            margin-bottom: 20px;
        }

        .ui-card::before {
            content: '';
            position: absolute;
            top: -1px; left: -1px; width: 6px; height: 6px;
            background: var(--primary-glow);
            box-shadow: 0 0 8px var(--primary-glow);
        }

        .panel-heading {
            font-size: 14px;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 15px;
            color: var(--primary-glow);
            border-bottom: 1px solid var(--border-color);
            padding-bottom: 5px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }

        .panel {
            background: var(--panel-bg);
            border: 1px solid var(--border-color);
            border-radius: 4px;
            padding: 15px;
            box-shadow: inset 0 0 15px rgba(0, 255, 102, 0.05);
            position: relative;
        }

        .panel::before {
            content: '';
            position: absolute;
            top: 0; left: 0; width: 4px; height: 4px;
            background: var(--primary-glow);
        }

        .panel h2 {
            font-size: 14px;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 15px;
            color: var(--primary-glow);
            border-bottom: 1px solid var(--border-color);
            padding-bottom: 5px;
        }

        .metric-group {
            display: flex;
            justify-content: space-between;
            margin-bottom: 10px;
            font-size: 13px;
        }

        .metric-label {
            color: var(--text-dim);
        }

        .metric-value {
            font-weight: bold;
            text-shadow: 0 0 5px var(--primary-glow);
        }

        .progress-container {
            background: rgba(0, 34, 11, 0.5);
            border: 1px solid var(--border-color);
            height: 12px;
            border-radius: 2px;
            overflow: hidden;
            margin-top: 5px;
            margin-bottom: 15px;
        }

        .progress-bar {
            height: 100%;
            background: var(--primary-glow);
            width: 0%;
            transition: width 0.5s ease-in-out;
            box-shadow: 0 0 8px var(--primary-glow);
        }

        .lock-status {
            display: inline-block;
            padding: 4px 8px;
            border-radius: 3px;
            font-weight: bold;
            font-size: 12px;
            text-transform: uppercase;
        }

        .status-locked {
            background: rgba(0, 255, 102, 0.2);
            border: 1px solid var(--primary-glow);
            color: var(--primary-glow);
        }

        .status-unlocked {
            background: rgba(255, 51, 51, 0.2);
            border: 1px solid var(--alert-color);
            color: var(--alert-color);
            text-shadow: 0 0 5px var(--alert-color);
        }

        .btn {
            background: transparent;
            border: 1px solid var(--border-color);
            color: var(--text-dim);
            padding: 8px 15px;
            cursor: not-allowed;
            font-weight: bold;
            text-transform: uppercase;
            transition: all 0.2s;
            margin-top: 10px;
            width: 100%;
            opacity: 0.5;
            font-family: inherit;
        }

        .btn.active {
            border: 1px solid var(--primary-glow);
            color: var(--primary-glow);
            cursor: pointer;
            opacity: 1;
        }

        .btn.active:hover {
            background: var(--primary-glow);
            color: var(--bg-color);
            box-shadow: 0 0 15px var(--primary-glow);
        }

        .btn.critical {
            border: 1px solid var(--alert-color);
            color: var(--alert-color);
            cursor: pointer;
            opacity: 1;
        }

        .btn.critical:hover {
            background: var(--alert-color);
            color: var(--bg-color);
            box-shadow: 0 0 15px var(--alert-color);
        }

        /* Combined Structural Ingestion & Tabular Layouts */
        .master-box-layout {
            display: grid;
            grid-template-columns: 325px 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }

        .storage-grid-layout {
            display: grid;
            grid-template-columns: 200px 1fr;
            gap: 15px;
            margin-top: 15px;
        }

        .directory-tree-box {
            display: flex;
            flex-direction: column;
            gap: 5px;
        }

        .dir-node-item {
            font-size: 12px;
            padding: 6px 10px;
            cursor: pointer;
            border: 1px solid transparent;
            color: var(--text-dim);
            text-transform: uppercase;
        }

        .dir-node-item:hover, .dir-node-item.active {
            border-color: var(--border-color);
            background: rgba(0, 255, 102, 0.05);
            color: var(--primary-glow);
            text-shadow: 0 0 5px var(--primary-glow);
        }

        .drop-zone-chassis {
            border: 2px dashed var(--border-color);
            border-radius: 4px;
            padding: 20px 10px;
            text-align: center;
            background: rgba(0, 15, 5, 0.6);
            cursor: pointer;
            transition: all 0.2s ease-in-out;
        }

        .drop-zone-chassis:hover {
            border-color: var(--primary-glow);
            box-shadow: 0 0 12px rgba(0, 255, 102, 0.15);
        }

        .image-preview-frame {
            border: 1px solid var(--border-color);
            background: #000;
            padding: 5px;
            width: 100%;
            height: 150px;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
            margin-top: 15px;
        }

        .image-preview-frame img {
            max-width: 100%;
            max-height: 100%;
            object-fit: contain;
            opacity: 0.85;
        }

        .data-table {
            width: 100%;
            border-collapse: collapse;
            font-size: 12px;
            text-align: left;
        }

        .data-table th {
            border-bottom: 2px solid var(--border-color);
            padding: 8px;
            color: var(--primary-glow);
            text-transform: uppercase;
            font-size: 11px;
        }

        .data-table td {
            border-bottom: 1px solid rgba(0, 102, 34, 0.2);
            padding: 10px 8px;
            word-break: break-all;
        }

        .btn-submit {
            background: transparent;
            border: 1px solid var(--primary-glow);






            color: var(--primary-glow);
            padding: 10px;
            width: 100%;
            font-weight: bold;
            text-transform: uppercase;
            cursor: pointer;
            margin-top: 15px;
            font-family: inherit;
        }

        .btn-submit:hover {
            background: var(--primary-glow);
            color: var(--bg-color);
            box-shadow: 0 0 15px var(--primary-glow);
        }

        .badge-type {
            padding: 2px 6px;
            border-radius: 2px;
            font-size: 10px;
            font-weight: bold;
        }

        /* Mock Terminal Shell Styling */
        .terminal-shell {
            background: rgba(0, 15, 5, 0.9);
            border: 1px solid var(--border-color);
            padding: 10px;
            font-size: 12px;
            height: 125px;
            display: flex;
            flex-direction: column;
            justify-content: flex-end;
        }

        .terminal-log {
            overflow-y: auto;
            color: var(--text-dim);
            margin-bottom: 5px;
            white-space: pre-line;
            line-height: 1.4;
        }

        .terminal-input-line {
            display: flex;
            align-items: center;
        }

        .terminal-prompt {
            color: var(--primary-glow);
            margin-right: 5px;
        }

        .terminal-input {
            background: transparent;
            border: none;
            color: var(--primary-glow);
            font-family: inherit;
            font-size: inherit;
            outline: none;
            flex-grow: 1;
        }

        .terminal-field-input {
            background: rgba(0, 15, 5, 0.8);
            border: 1px solid var(--border-color);
            color: var(--primary-glow);
            padding: 3px 8px;
            font-family: inherit;
            font-size: 13px;
            outline: none;
            border-radius: 2px;
            width: 160px;
        }

        /* Multi Graph Layout */
        .graph-container {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(45%, 1fr));
            gap: 20px;
        }

        .graph-panel h3 {
            font-size: 11px;
            color: var(--text-dim);
            text-transform: uppercase;
            margin-bottom: 5px;
            letter-spacing: 1px;
        }

        canvas {
            width: 100%;
            height: 120px;
            background: rgba(0, 15, 5, 0.5);
            border: 1px solid var(--border-color);
        }

        .hidden { display: none !important; }
    </style>
</head>
<body>

    <!-- CRITICAL HARDWARE ALERTS OVERLAY -->
    <div id="hazardScreen" class="hazard-overlay-screen hidden">
        <div class="hazard-box-inner">
            <h2>[ CRITICAL WARNING ]</h2>
            <p id="hazardMessage">SYSTEM SECURITY PURGE ACTIVE</p>
        </div>
    </div>

    <!-- SYSTEM FILE VOLUMETRICS RUNTIME HEADERS STRIP -->
    <div class="ui-card" style="padding: 10px 15px; display: flex; justify-content: space-between; align-items: center;">
        <div style="font-weight: bold; text-transform: uppercase; font-size: 11px;">
            ● CBOS Local Storage Partition Allocation Mapping
        </div>
        <div style="display: flex; gap: 20px; font-size: 11px; color: var(--text-muted);">
            <div>MOUNT POINT: <span style="color: var(--text-color);">/mnt/forensic_pool</span></div>
            <div>USED SPACE: <span style="color: var(--accent-amber);">1.24 TB</span></div>
            <div>AVAILABLE: <span style="color: var(--accent-blue);">2.76 TB</span></div>
        </div>
    </div>

    <!-- UPPER CONTROL PANEL SECTION ROW GRID -->
    <div class="grid">
        <!-- Storage Pool Overview -->
        <div class="panel">
            <h2>Storage Pool Overview</h2>
            <div class="metric-group">
                <span class="metric-label">Array Status</span>
                <span class="metric-value" style="color: var(--primary-glow);">ONLINE</span>
            </div>
            <div class="metric-group">
                <span class="metric-label">Storage Pool 1</span>
                <span class="metric-value" id="storageText">0.0 TB / 16.0 TB</span>
            </div>
            <div class="progress-container">
                <div class="progress-bar" id="storagePool1"></div>
            </div>
            <div class="metric-group">
                <span class="metric-label">Active NVMe Slots</span>
                <span class="metric-value">8 / 8 Units</span>
            </div>
            <div class="metric-group">
                <span class="metric-label">Array Type</span>
                <span class="metric-value">RAID 10 (Striped Mirror)</span>
            </div>
        </div>

        <!-- Hardware Interlock Relay Control Loop -->
        <div class="panel">
            <h2>Hardware Interlock Control Loop</h2>
            <div class="metric-group">
                <span class="metric-label">Target Circuit Interlock</span>
                <span class="metric-value">Teyleten Robot Optocoupler</span>
            </div>
            <div class="metric-group">
                <span class="metric-label">Interlock State</span>
                <span class="lock-status status-locked" id="lockStatus">Engaged</span>
            </div>
            <button class="btn" id="lockBtn" disabled onclick="toggleLock()">Trigger Unit Detach</button>
            <button class="btn critical" onclick="triggerHardwareEmergencyStop()">EMERGENCY HARDWARE STOP</button>
        </div>

        <!-- ADS1256 High-Precision Metadata Harvester -->
        <div class="panel">
            <h2>ADS1256 Metadata Harvester</h2>
            <div class="metric-group">
                <span class="metric-label">Active Sample Focus</span>
                <span><input type="text" id="sampleIdInput" class="terminal-field-input" value="EV-2026-0725-88A"></span>
            </div>
            <div class="metric-group">
                <span class="metric-label">Physical Interface Standard</span>
                <span class="metric-value">24-Bit Single-Ended Analog</span>
            </div>
            <div class="progress-container">
                <div class="progress-bar" id="harvesterMeter" style="width: 0%;"></div>
            </div>
            <button class="btn active" onclick="captureMetadataBlock()">Capture Ingestion Frame</button>
        </div>
    </div>

    <!-- MAIN INTERACTIVE CASE PARTITION EXPLORER SPLIT GRID -->
    <div class="master-box-layout">
        <!-- FORENSIC LOCAL DROP BOX COLUMN INGESTION CONTAINER -->
        <div class="panel">
            <h2>Evidence Ingestion Drop Box</h2>
            <p style="color: var(--text-dim); font-size: 11px; margin-bottom: 15px; text-transform: uppercase;">
                Captive portal client acquisition file drop interface:
            </p>
            <form id="ingestionForm">
                <input type="file" id="fileSelector" accept="image/*" class="hidden" onchange="processStagedArtifact(this)">
                <div class="drop-zone-chassis" onclick="document.getElementById('fileSelector').click()">
                    <span style="color: var(--primary-glow); font-weight: bold; font-size: 11px; display: block; margin-bottom: 5px;">[ COMMENCE ARTIFACT SCAN ]</span>
                    <span style="color: var(--text-dim); font-size: 10px; text-transform: uppercase;">Click to select local image files</span>
                </div>

                <div id="previewContainer" class="hidden" style="margin-top: 15px;">
                    <div class="image-preview-frame">
                        <img id="imageMock" src="" alt="Forensic Telemetry Thumbnail Preview">
                    </div>
                    <div style="margin-top: 12px; font-size: 11px; line-height: 1.5;">
                        <div class="metric-group">
                            <span class="metric-label">IDENTIFIER:</span>
                            <span class="metric-value" id="metaName" style="max-width: 170px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;">--</span>
                        </div>
                        <div class="metric-group">
                            <span class="metric-label">CAPACITY:</span>
                            <span class="metric-value" id="metaSize" style="color: var(--accent-blue);">-- KB</span>
                        </div>
                        <div class="metric-group">
                            <span class="metric-label">SOURCE DEVICE:</span>
                            <span class="metric-value" id="metaSourceDevice" style="color: var(--accent-amber);">DETECTING...</span>
                        </div>
                        <div class="metric-group">
                            <span class="metric-label">TIMESTAMP:</span>
                            <span class="metric-value" id="metaTimestamp">--</span>
                        </div>
                    </div>
                    <button type="button" class="btn-submit" onclick="commitBlockToLedger()">Transmit Block Payload to Flash Partition</button>
                </div>
            </form>
        </div>

        <!-- RECONSTRUCTED TABULAR FOLDER DATA LOGGING MATRIX VIEWPORT -->
        <div class="ui-card" style="margin-bottom: 0;">
            <div class="panel-heading">
                <span>Case File Directory Node &amp; Logging Grid</span>
                <span id="dvrIndicator" style="color: var(--secondary-glow); font-size: 11px;">● STORAGE POOL READ/WRITE MOUNTED</span>
            </div>

            <div class="storage-grid-layout">
                <div class="directory-tree-box">
                    <div class="dir-node-item active" onclick="filterFileGridByFolder('all', this)">[Root Partition]</div>






                    <div class="dir-node-item" onclick="filterFileGridByFolder('network', this)">↳ Packet Captures</div>
                    <div class="dir-node-item" onclick="filterFileGridByFolder('biometric', this)">↳ Bio Telemetry Logs</div>
                    <div class="dir-node-item" onclick="filterFileGridByFolder('legal', this)">↳ Signed Warrants</div>
                </div>

                <div style="background-color: rgba(2, 7, 15, 0.2); border: 1px solid var(--border-color); padding: 10px; overflow-y: auto; max-height: 420px; width: 100%;">
                    <table class="data-table" id="fileTable">
                        <thead>
                            <tr>
                                <th>File Allocation Name</th>
                                <th>Byte Sizing</th>
                                <th>Origin Source Mapping</th>
                                <th>Ingestion Timestamp</th>
                                <th>Directory Category</th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr data-folder="network">
                                <td style="color: var(--accent-blue); font-weight: bold;">packet_dump_ch6.pcap</td>
                                <td>42.8 MB</td>
                                <td>SOFTAP-PROV-04 (Chrome/Linux)</td>
                                <td>2026-07-25 15:30:12</td>
                                <td><span class="badge-type" style="background: rgba(0,204,255,0.1); color: var(--accent-blue);">NETWORK</span></td>
                            </tr>
                            <tr data-folder="biometric">
                                <td style="color: var(--accent-blue); font-weight: bold;">bio_session_8821.bin</td>
                                <td>12.4 MB</td>
                                <td>NEURAL-CORE-MESH (Embedded Node)</td>
                                <td>2026-07-25 15:45:00</td>
                                <td><span class="badge-type" style="background: rgba(0,230,118,0.1); color: var(--secondary-glow);">BIOMETRIC</span></td>
                            </tr>
                            <tr data-folder="legal">
                                <td style="color: var(--accent-blue); font-weight: bold;">warrant_sign_approved.sig</td>
                                <td>4.2 KB</td>
                                <td>LEGAL-MATRIX-OP (Terminal v2.1)</td>
                                <td>2026-07-25 16:02:44</td>
                                <td><span class="badge-type" style="background: rgba(255,176,0,0.1); color: var(--accent-amber);">LEGAL</span></td>
                            </tr>
                            <tr data-folder="network">
                                <td style="color: var(--accent-blue); font-weight: bold;">ap_handshake_traces.log</td>
                                <td>842 KB</td>
                                <td>SOFTAP-PROV-01 (Safari/iOS)</td>
                                <td>2026-07-25 16:10:19</td>
                                <td><span class="badge-type" style="background: rgba(0,204,255,0.1); color: var(--accent-blue);">NETWORK</span></td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        </div>
    </div>

    <!-- GRAPH OSCILLOSCOPE READOUTS ROW FOOTER -->
    <div class="grid" style="grid-template-columns: 1fr 1fr;">
        <div class="panel">
            <h2>Real-Time Replication Traffic Channels</h2>
            <div class="graph-container">
                <div class="graph-panel">
                    <h3>Channel Alpha: NVMe Read Throughput (MB/s)</h3>
                    <canvas id="readCanvas"></canvas>
                </div>
                <div class="graph-panel">
                    <h3>Channel Beta: NVMe Write Throughput (MB/s)</h3>
                    <canvas id="writeCanvas"></canvas>
                </div>
            </div>
        </div>

        <div class="panel">
            <h2>Structured Index Records (Cyber-Bio Data Core SAN Storage)</h2>
            <pre><code id="jsonOutput">// Awaiting physical capture initialization command...</code></pre>
        </div>
    </div>

    <!-- SHELL CONSOLE INTERACTIVE TERMINAL LOGGER BOARD -->
    <div class="panel" style="margin-bottom: 10px;">
        <h2>System Terminal Logger Shell</h2>
        <div class="terminal-shell">
            <div class="terminal-log" id="termLog">Awaiting authentication sequence profiles. Type 'help' for instructions.</div>
            <div class="terminal-input-line">
                <span class="terminal-prompt">GUEST@NODE01:$</span>
                <input type="text" class="terminal-input" id="termInput" autocomplete="off">
            </div>
        </div>
    </div>

    <script>
        // System Metrics & Core Application States
        const storageTarget = 8.2; 
        const totalStorage = 16.0;
        let isLocked = true;
        let isAuthenticated = false;
        const EDGE_GATEWAY_URL = "http://192.168.1.50"; 
        const PORTAL_GATEWAY_URL = "http://192.168.4.1"; 

        // WebSocket Telemetry Carrier Setup Variables
        let meshSocketNode;
        const MESH_NODE_WS_ENDPOINT = "ws://192.168.1.50/api/v1/telemetry/stream";
        const RECONNECT_DELAY_INTERVAL_MS = 5000;

        /**
         * 1. HARDWARE MESH PERSISTENT MONITOR CHANNEL
         * Observes mesh heartbeats over socket tunnels; triggers error overlays upon drop.
         */
        function initializeHardwareMeshMonitor() {
            logToTerminal("[INFO] Connecting telemetry stream websocket pipeline link...");
            meshSocketNode = new WebSocket(MESH_NODE_WS_ENDPOINT);

            meshSocketNode.onopen = function() {
                logToTerminal("[SUCCESS] Persistent telemetry link verified with edge network.");
                document.getElementById('hazardScreen').classList.add('hidden');
            };

            meshSocketNode.onmessage = function(event) {
                try {
                    const packet = JSON.parse(event.data);
                    if (packet.status === "ALERT_TRIGGERED") {
                        engageVisualHazardScreen("REMOTE NODE FORCE DISCONNECT EXECUTED", 5000);
                    }
                } catch(e){}
            };

            meshSocketNode.onclose = function() {
                logToTerminal("[CRITICAL] - Telemetry stream carrier dropped interface synchronization.");
                engageVisualHazardScreen("HARDWARE MESH DISCONNECTED // SYSTEM UNSTABLE", 0);
                setTimeout(initializeHardwareMeshMonitor, RECONNECT_DELAY_INTERVAL_MS);
            };

            meshSocketNode.onerror = function(err) {
                console.error("Websocket tracking execution failure:", err);
            };
        }

        // Initialize view visual meters
        setTimeout(() => {
            const percentage = (storageTarget / totalStorage) * 100;
            const targetElement = document.getElementById('storagePool1');
            if (targetElement) targetElement.style.width = percentage + '%';
            document.getElementById('storageText').innerText = `${storageTarget} TB / ${totalStorage} TB (${Math.round(percentage)}%)`;
        }, 300);

        function updateClock() {
            const now = new Date();
            document.getElementById('timestamp').innerText = `SYS_TIME: ${now.toTimeString().split(' ')[0]}`;
        }
        setInterval(updateClock, 1000);
        updateClock();

        // Console Shell Line Processor
        const termInput = document.getElementById('termInput');
        const termLog = document.getElementById('termLog');
        const lockBtn = document.getElementById('lockBtn');

        termInput.addEventListener('keydown', function(e) {
            if (e.key === 'Enter') {
                const cmd = termInput.value.trim().toLowerCase();
                termInput.value = '';
                processCommand(cmd);
            }
        });

        function logToTerminal(text) {
            termLog.innerText += "\n" + text;
            termLog.scrollTop = termLog.scrollHeight;
        }





        function processCommand(cmd) {
            if (!cmd) return;
            logToTerminal(`> ${cmd}`);

            if (cmd === 'help') {
                logToTerminal("Available: 'login [password]', 'clear', 'status', 'capture', 'wipe'");
            } else if (cmd === 'clear') {
                termLog.innerText = "Shell buffer cleared.";
            } else if (cmd === 'capture') {
                captureMetadataBlock();
            } else if (cmd === 'wipe') {
                executeStorageScrubSequence();
            } else if (cmd === 'status') {
                logToTerminal(`Auth: ${isAuthenticated ? 'ROOT' : 'UNAUTHORIZED'} | Interlock: ${isLocked ? 'ENGAGED' : 'DISENGAGED'}`);
            } else if (cmd.startsWith('login ')) {
                const pass = cmd.substring(6).trim();
                if (pass === 'root') {
                    isAuthenticated = true;
                    logToTerminal("ACCESS GRANTED. Root authorization verified.");
                    document.querySelector('.terminal-prompt').innerText = "ROOT@NODE01:#";
                    lockBtn.removeAttribute('disabled');
                    lockBtn.className = "btn active";
                    lockBtn.innerText = "Trigger Unit Detach";
                } else {
                    logToTerminal("ERROR: Access credentials validation tracking mismatch.");
                }
            } else {
                logToTerminal(`Command '${cmd}' not recognized.`);
            }
        }

        /**
         * 2. VISUAL HAZARD OVERLAY CONTROLLER
         * Displays emergency flashing screens. Passing duration 0 keeps the screen persistently locked.
         */
        function engageVisualHazardScreen(messageText, delayDurationMs) {
            const targetOverlay = document.getElementById('hazardScreen');
            document.getElementById('hazardMessage').innerText = messageText;
            targetOverlay.classList.remove('hidden');
            
            if (delayDurationMs > 0) {
                setTimeout(() => {
                    if (!meshSocketNode || meshSocketNode.readyState === WebSocket.OPEN) {
                        targetOverlay.classList.add('hidden');
                    }
                }, delayDurationMs);
            }
        }

        function executeStorageScrubSequence() {
            engageVisualHazardScreen("DESTRUCTIVE INDEX PURGE INITIATED", 3000);
            const tableBody = document.querySelector('#fileTable tbody');
            if (tableBody) {
                tableBody.innerHTML = `<tr><td colspan="5" style="text-align: center; color: var(--text-muted);">[ ARRAYS SANITIZED — NO COMPACTED FILES LOGGED ]</td></tr>`;
            }
            logToTerminal("[SUCCESS] Local storage logs structural elements dropped and wiped.");
        }

        function toggleLock() {
            if (!isAuthenticated) return;
            const label = document.getElementById('lockStatus');
            if (isLocked) {
                label.innerText = "Disengaged"; label.className = "lock-status status-unlocked";
                lockBtn.innerText = "Lock Mechanism"; isLocked = false;
                executeHardwareFetch('RELEASE');
            } else {
                label.innerText = "Engaged"; label.className = "lock-status status-locked";
                lockBtn.innerText = "Trigger Unit Detach"; isLocked = true;
                executeHardwareFetch('ENGAGE');
            }
        }

        function triggerHardwareEmergencyStop() {
            engageVisualHazardScreen("HARDWARE INTERLOCK SEPARATION LOOP INITIATED", 4000);
            const label = document.getElementById('lockStatus');
            label.innerText = "Disengaged"; label.className = "lock-status status-unlocked";
            executeHardwareFetch('RELEASE');
        }

        function executeHardwareFetch(state) {
            fetch(`${EDGE_GATEWAY_URL}/api/v1/hardware/relay`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ "command": state, "hold_delay_ms": 5000 })
            }).catch(() => {
                console.log(`[NETWORK MOCK]: Outbound fetch signal '${state}' successfully routed.`);
            });
        }

        /**
         * 3. INGESTION CONTROLLER DROP BOX INTERFACE CONTROLLER
         * Decodes machine browser headers and adds rows directly into the data matrix grid.
         */
        function processStagedArtifact(input) {
            if (input.files && input.files[0]) {
                const file = input.files[0];
                const reader = new FileReader();
                reader.onload = function(e) { document.getElementById('imageMock').src = e.target.result; };
                reader.readAsDataURL(file);

                document.getElementById('metaName').innerText = file.name;
                document.getElementById('metaSize').innerText = (file.size / 1024).toFixed(2) + " KB";
                document.getElementById('metaSourceDevice').innerText = parseClientDevicePlatform();
                
                const d = new Date();
                document.getElementById('metaTimestamp').innerText = d.toISOString().replace('T', ' ').substring(0, 19);
                document.getElementById('previewContainer').classList.remove('hidden');
            }
        }

        function parseClientDevicePlatform() {
            const agent = navigator.userAgent;
            let node = "WORKSTATION-NODE"; let engine = "Unknown Engine";
            if (agent.indexOf("Win") !== -1) node = "SYS-WIN-NODE";
            else if (agent.indexOf("Mac") !== -1) node = "SYS-OSX-NODE";
            else if (agent.indexOf("Linux") !== -1) node = "SYS-LINUX-NODE";
            
            if (agent.indexOf("Chrome") !== -1) engine = "Chrome";
            else if (agent.indexOf("Safari") !== -1 && agent.indexOf("Chrome") === -1) engine = "Safari";
            else if (agent.indexOf("Firefox") !== -1) engine = "Firefox";
            return `${node} (${engine})`;
        }

        function commitBlockToLedger() {
            const name = document.getElementById('metaName').innerText;
            const size = document.getElementById('metaSize').innerText;
            const device = document.getElementById('metaSourceDevice').innerText;
            const stamp = document.getElementById('metaTimestamp').innerText;
            
            const tableBody = document.querySelector('#fileTable tbody');
            const row = document.createElement('tr');
            row.setAttribute('data-folder', 'network');
            row.innerHTML = `
                <td style="color: var(--accent-blue); font-weight: bold;">${name}</td>
                <td>${size}</td><td>${device}</td><td>${stamp}</td>
                <td><span class="badge-type" style="background:rgba(0,204,255,0.1); color:var(--accent-blue);">NETWORK</span></td>
            `;
            tableBody.insertBefore(row, tableBody.firstChild);
            document.getElementById('previewContainer').classList.add('hidden');
            document.getElementById('ingestionForm').reset();
            logToTerminal(`[SUCCESS] Evidence token registered: ${name}`);
        }

        function filterFileGridByFolder(folder, clicked) {
            document.querySelectorAll('.dir-node-item').forEach(item => item.classList.remove('active'));
            clicked.classList.add('active');
            document.querySelectorAll('#fileTable tbody tr').forEach(row => {
                const ctx = row.getAttribute('data-folder');
                row.style.display = (folder === 'all' || ctx === folder) ? 'table-row' : 'none';
            });
        }

        function captureMetadataBlock() {
            const id = document.getElementById('sampleIdInput').value;
            const meter = document.getElementById('harvesterMeter');
            meter.style.width = '100%';
            logToTerminal("[INFO] Querying 24-bit channel register matrix data streams...");

            setTimeout(() => {
                const adc = Math.floor(Math.random() * 16777215);
                const mv = ((adc * 2.5) / 8388607) * 1000;
                const payload = {
                    "record_uuid": crypto.randomUUID(),
                    "associated_sample_id": id,
                    "ingestion_timestamp_epoch": Date.now() * 1000000,
                    "telemetry_metrics": { "optical_amplifier_raw": adc, "calculated_voltage_mv": parseFloat(mv.toFixed(4)) },
                    "hardware_manifest": { "converter_chipset": "TI_ADS1256IDB_24BIT", "acquisition_mode": "SINGLE_ENDED_ANALOG" }
                };
                document.getElementById('jsonOutput').textContent = JSON.stringify(payload, null, 2);
                logToTerminal(`[SUCCESS] 24-bit acquisition locked: ${(mv/1000).toFixed(4)}V`);
                meter.style.width = '0%';
            }, 400);
        }

        // Canvas Data Wave Setup (Dual Chart System)
        const readCanvas = document.getElementById('readCanvas');
        const writeCanvas = document.getElementById('writeCanvas');
        const readCtx = readCanvas.getContext('2d');
        const writeCtx = writeCanvas.getContext('2d');
        
        function resizeCanvases() {
            const width = readCanvas.parentElement.clientWidth;
            readCanvas.width = width; writeCanvas.width = width;
            readCanvas.height = 120; writeCanvas.height = 120;
        }
        window.addEventListener('resize', resizeCanvases);
        resizeCanvases();

        let readOffset = 0; let writeOffset = 50; 

        function drawGrid(ctx, w, h) {
            ctx.strokeStyle = 'rgba(0, 102, 34, 0.12)'; ctx.lineWidth = 1;
            for(let i = 0; i < w; i += 30) { ctx.beginPath(); ctx.moveTo(i, 0); ctx.lineTo(i, h); ctx.stroke(); }






            for(let j = 0; j < h; j += 25) { 
                ctx.beginPath(); ctx.moveTo(0, j); ctx.lineTo(w, j); ctx.stroke(); 
            }
        }

        /**
         * 4. GRAPH OSCILLOSCOPE RENDERING CIRCUITS
         * Loops animation processing steps for the separate read and write channels.
         */
        function renderGraphs() {
            // Read Activity Channel (Monochromatic Phosphor Green)
            readCtx.clearRect(0, 0, readCanvas.width, readCanvas.height);
            drawGrid(readCtx, readCanvas.width, readCanvas.height);
            readCtx.beginPath(); 
            readCtx.strokeStyle = '#00ff66'; 
            readCtx.lineWidth = 2;
            readCtx.moveTo(0, readCanvas.height / 2);
            for (let x = 0; x < readCanvas.width; x++) {
                const y = (readCanvas.height / 2) + Math.sin(x * 0.015 + readOffset) * 25 + Math.cos(x * 0.04 - readOffset) * 8;
                readCtx.lineTo(x, y);
            }
            readCtx.stroke();

            // Write Activity Channel (High-Contrast Wave Cyan)
            writeCtx.clearRect(0, 0, writeCanvas.width, writeCanvas.height);
            drawGrid(writeCtx, writeCanvas.width, writeCanvas.height);
            writeCtx.beginPath(); 
            writeCtx.strokeStyle = '#00ccff'; 
            writeCtx.lineWidth = 2;
            writeCtx.moveTo(0, writeCanvas.height / 2);
            for (let x = 0; x < writeCanvas.width; x++) {
                const y = (writeCanvas.height / 2) + Math.sin(x * 0.03 + writeOffset) * 15 + Math.sin(x * 0.07 + writeOffset * 1.5) * 12 + ((x % 80 === 0) ? -25 : 0);
                writeCtx.lineTo(x, y);
            }
            writeCtx.stroke();

            // Update drawing parameter cycle limits
            readOffset += 0.04; 
            writeOffset += 0.06;
            requestAnimationFrame(renderGraphs);
        }

        // Finalize document readiness initialization tracking configurations
        window.onload = function() {
            initializeHardwareMeshMonitor();
            renderGraphs();
        };
    </script>
</body>
</html>

)rawliteral";

void handleRoot() { server.send(200, "text/html", html_portal); }

void handleUpload() {
  server.sendHeader("Location", "http://192.168.4.1", true);
  server.send(302, "text/plain", "");
}

void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    uploadFile = LittleFS.open(upload.filename, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE && uploadFile) {
    uploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END && uploadFile) {
    uploadFile.close();
  }
}

void setup() {
  Serial.begin(115200);
  LittleFS.begin(true);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/upload", HTTP_POST, handleUpload, handleFileUpload);
  server.onNotFound([]() { server.sendHeader("Location", "http://192.168.4.1", true); server.send(302); });
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
