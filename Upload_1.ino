#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>

const char* ssid = "CBOS-PROPAGATION-NODE";
const char* password = ""; 

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

File uploadFile;

// Complete Integrated Dashboard UI with File Drop & Onboard Directory Browser
const char html_portal[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CBOS - Soft AP Forensic Drop Box</title>
    <script src="https://tailwindcss.com"></script>
    <link rel="stylesheet" href="https://cloudflare.com">
    <script>
        tailwind.config = { theme: { extend: { colors: { apBg: "#040a14", panelBg: "#0a1526", accentBlue: "#0091ff", accentGreen: "#00e676", borderBlue: "#122b4f" } } } }
    </script>
    <style>
        body { background-color: #040a14; font-family: ui-sans-serif, sans-serif; }
        .dashed-contour { background-image: url("data:image/svg+xml,%3csvg width='100%25' height='100%25' xmlns='http://w3.org width='100%25' height='100%25' fill='none' rx='8' ry='8' stroke='%23122B4FFF' stroke-width='2' stroke-dasharray='8%2c 8' stroke-dashoffset='0' stroke-linecap='square'/%3e%3c/svg%3e"); }
    </style>
</head>
<body class="text-slate-200 min-h-screen flex flex-col p-4 md:p-6 space-y-6 max-w-xl mx-auto">

    <!-- ================= CAPTIVE PORTAL HEADER ================= -->
    <header class="w-full bg-panelBg border border-borderBlue rounded-xl p-4 flex justify-between items-center shadow-lg">
        <div class="flex items-center gap-2">
            <i class="fa-solid fa-wifi text-accentBlue animate-pulse"></i>
            <div>
                <h1 class="text-xs font-bold tracking-widest text-slate-100">SOFT AP PORTAL</h1>
                <p class="text-[10px] text-slate-400 font-mono">HOST: ESP32 CORE AIRGAP</p>
            </div>
        </div>
        <span class="bg-accentGreen/10 border border-accentGreen/30 text-accentGreen text-[9px] font-mono px-2 py-0.5 rounded font-bold">MUTABLE_STORAGE_ON</span>
    </header>

    <!-- ================= INGESTION CONTROLLER DROP BOX ================= -->
    <main class="w-full bg-panelBg border border-borderBlue rounded-xl p-6 shadow-xl">
        <div class="text-center mb-4">
            <h2 class="text-xs font-bold tracking-widest text-accentBlue uppercase">Local Artifact Drop Box</h2>
            <p class="text-xs text-slate-400 mt-1">Select evidence files to transmit them to the flash chip bank arrays.</p>
        </div>
        
        <form method="POST" action="/upload" enctype="multipart/form-data" class="space-y-4">
            <input type="file" name="update" id="fileSelector" accept="image/*" class="hidden" onchange="processSelectedImage(this)">
            <div onclick="document.getElementById('fileSelector').click()" class="dashed-contour rounded-lg p-6 flex flex-col items-center justify-center cursor-pointer min-h-[120px] hover:border-accentBlue group transition-all">
                <i class="fa-solid fa-cloud-arrow-up text-2xl text-slate-500 group-hover:text-accentBlue mb-2"></i>
                <span class="text-xs font-medium text-slate-300">Click to browse your local device</span>
            </div>
            
            <div id="previewContainer" class="hidden border border-borderBlue bg-black/30 rounded-lg p-4">
                <div class="grid grid-cols-12 gap-4 items-center">
                    <div class="col-span-4 bg-apBg border border-borderBlue rounded p-1 flex items-center justify-center aspect-square overflow-hidden">
                        <img id="imageMock" src="" class="w-full h-full object-contain">
                    </div>
                    <div class="col-span-8 space-y-1 text-[11px] font-mono">
                        <div class="truncate"><span class="text-slate-500">STAGED:</span> <span id="metaName" class="text-slate-300 font-bold"></span></div>
                        <div><span class="text-slate-500">CAPACITY:</span> <span id="metaSize" class="text-slate-300"></span></div>
                    </div>
                </div>
                <button type="submit" class="w-full mt-4 bg-accentBlue hover:bg-blue-600 text-white font-bold text-xs py-2 rounded transition-all">
                    Transmit Block Payload to Flash Partition
                </button>
            </div>
        </form>
    </main>

    <!-- ================= HARDWARE EVIDENCE STORAGE MATRIX ================= -->
    <section class="w-full bg-panelBg border border-borderBlue rounded-xl p-6 shadow-xl flex flex-col">
        <div class="border-b border-borderBlue pb-2 mb-4 flex justify-between items-center">
            <h2 class="text-xs font-bold tracking-widest text-accentGreen uppercase"><i class="fa-solid fa-box-archive mr-1.5"></i>Onboard Evidence Locker</h2>
            <span class="text-[10px] font-mono text-slate-400">PARTITION: LITTLEFS</span>
        </div>

        <!-- Storage Bar Allocation Metrics -->
        <div class="mb-4">
            <div class="flex justify-between text-[10px] font-mono mb-1 text-slate-400">
                <span>Onboard Flash Capacity Allocation</span>
                <span id="storagePercent">Calculating...</span>
            </div>
            <div class="h-2 bg-black/40 rounded-full border border-borderBlue overflow-hidden">
                <div id="storageBar" class="h-full bg-gradient-to-r from-accentBlue to-accentGreen w-0 transition-all duration-500"></div>
            </div>
        </div>

        <!-- Real-Time Directory Grid List -->
        <div id="directoryContainer" class="space-y-2 max-h-[220px] overflow-y-auto pr-1">
            <div class="text-center py-4 text-xs text-slate-500 font-mono">Scanning storage filesystems...</div>
        </div>
    </section>

    <footer class="text-center text-[10px] text-slate-500 font-mono">
        Isolated Cryptographic Node File Browser Environment — CBOS v3.0
    </footer>

    <script>
        function processSelectedImage(input) {
            if (input.files && input.files[0]) {
                const file = input.files[0];
                const reader = new FileReader();
                reader.onload = function(e) { document.getElementById('imageMock').src = e.target.result; }
                reader.readAsDataURL(file);
                document.getElementById('metaName').innerText = file.name;
                document.getElementById('metaSize').innerText = (file.size / 1024).toFixed(2) + " KB";
                document.getElementById('previewContainer').classList.remove('hidden');
            }
        }

        // Fetch running workspace array structures directly from the ESP32 endpoint json loops
        async function refreshStorageDirectory() {
            try {
                const response = await fetch('/list-files');
                const data = await response.json();
                
                // Render storage allocation status progress bar
                const used = data.usedBytes;
                const total = data.totalBytes;
                const pct = ((used / total) * 100).toFixed(1);
                document.getElementById('storagePercent').innerText = pct + "% (" + (used/1024).toFixed(0) + "KB / " + (total/1024).toFixed(0) + "KB)";
                document.getElementById('storageBar').style.width = pct + "%";

                const container = document.getElementById('directoryContainer');
                container.innerHTML = "";

                if (data.files.length === 0) {
                    container.innerHTML = `<div class="text-center py-4 text-xs text-slate-500 font-mono">[Locker Empty - No Forensic Evidence Staged]</div>`;
                    return;
                }

                // Loop layout injection for array entries
                data.files.forEach(file => {
                    const row = document.createElement('div');
                    row.className = "flex justify-between items-center bg-black/30 border border-borderBlue/50 p-2.5 rounded text-xs font-mono";
                    row.innerHTML = `
                        <div class="flex items-center gap-2 truncate max-w-[70%]">
                            <i class="fa-solid fa-file-image text-purple-400"></i>
                            <span class="text-slate-300 truncate" title="${file.name}">${file.name}</span>
                        </div>
                        <div class="flex items-center gap-3">
                            <span class="text-slate-500 text-[10px]">${(file.size / 1024).toFixed(1)} KB</span>
                            <a href="/download?file=${encodeURIComponent(file.name)}" download class="text-accentCyan hover:text-accentGreen transition-colors"><i class="fa-solid fa-download"></i></a>
                            <button onclick="deleteEvidenceFile('${file.name}')" class="text-slate-500 hover:text-red-400 transition-colors"><i class="fa-solid fa-trash-can"></i></button>
                        </div>
                    `;
                    container.appendChild(row);
                });
            } catch (err) {
                console.error("Directory synchronizer execution failure:", err);
            }
        }

        async function deleteEvidenceFile(filename) {
            if (confirm("Purge selected forensic artifact permanently from hardware arrays?")) {
                await fetch(`/delete?file=${encodeURIComponent(filename)}`, { method: 'DELETE' });
                refreshStorageDirectory();
            }
        }

        // Initialize synchronization run loop paths
        window.addEventListener('DOMContentLoaded', refreshStorageDirectory);
    </script>
</body>
</html>
)rawliteral";

void handleRootPortal() {
  server.send(200, "text/html", html_portal);
}



// REST Endpoint: Returns filesystem status layout arrays as JSON objects
void handleListFiles() {
  String json = "{";
  json += "\"totalBytes\":" + String(LittleFS.totalBytes()) + ",";
  json += "\"usedBytes\":" + String(LittleFS.usedBytes()) + ",";
  json += "\"files\":[";

  File root = LittleFS.open("/");
  File file = root.openNextFile();
  bool first = true;
  while(file) {
    if (!first) json += ",";
    json += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
    first = false;
    file = root.openNextFile();
  }
  json += "]}";
  server.send(200, "application/json", json);
}

// REST Endpoint: Downloads selected files from the filesystem core
void handleDownloadFile() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "BAD REQUEST: Missing file reference target argument.");
    return;
  }
  String filename = server.arg("file");
  if (!filename.startsWith("/")) filename = "/" + filename;

  if (LittleFS.exists(filename)) {
    File file = LittleFS.open(filename, "r");
    server.streamFile(file, "application/octet-stream");
    file.close();
  } else {
    server.send(404, "text/plain", "NOT FOUND: Evidence block mapping target does not exist.");
  }
}

// REST Endpoint: Deletes an asset block permanently from the flash grid arrays
void handleDeleteFile() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "BAD REQUEST");
    return;
  }
  String filename = server.arg("file");
  if (!filename.startsWith("/")) filename = "/" + filename;

  if (LittleFS.exists(filename)) {
    LittleFS.remove(filename);
    server.send(200, "text/plain", "DELETED");
  } else {
    server.send(404, "text/plain", "NOT FOUND");
  }
}

void handleUploadExecution() {
  // Automatically redirect back to the home layout menu to show the updated file list immediately
  server.sendHeader("Location", "http://192.168.4", true);
  server.send(302, "text/plain", "");
}

void handleFileUploadProcessing() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("Target destination mapping initializing: "); Serial.println(filename);
    uploadFile = LittleFS.open(filename, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      Serial.print("Ingestion payload success volume: "); Serial.println(upload.totalSize);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  
  if(!LittleFS.begin(true)){
    Serial.println("An error occurred while mounting LittleFS partition.");
    return;
  }

  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid, password);
  
  Serial.print("Access Point Active. IP Address: "); Serial.println(WiFi.softAPIP());

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // Web Routing Architecture Mapping Endpoints
  server.on("/", HTTP_GET, handleRootPortal);
  server.on("/list-files", HTTP_GET, handleListFiles);
  server.on("/download", HTTP_GET, handleDownloadFile);
  server.on("/delete", HTTP_DELETE, handleDeleteFile);
  server.on("/upload", HTTP_POST, handleUploadExecution, handleFileUploadProcessing);
  
  // Captive Portal Hijack Rules Map
  server.on("/generate_204", HTTP_GET, handleRootPortal); 
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("HTTP Core Engine and File Management Node System Online.");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
