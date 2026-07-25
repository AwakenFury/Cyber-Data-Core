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
<head>...[HTML Content from https://github.com/AwakenFury/Cyber-Data-Core/blob/main/Data_Core.html]...</head>
<body>...[Dashboard UI elements]...</body>
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
