/**
 * Webserver Implementierung
 * 
 * @file webserver.cpp
 * @author rigr
 */

#include "webserver.h"

// HTML-Interface (eingebettet)
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Maus-Tracker</title>
<style>
body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#1a1a1a;color:#fff}
.container{max-width:800px;margin:0 auto}
h1{text-align:center;color:#00d4ff}
.status-box{background:#2a2a2a;border-radius:8px;padding:20px;margin:20px 0;border-left:4px solid #00d4ff}
.status-item{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid #3a3a3a}
.status-item:last-child{border-bottom:none}
.status-label{font-weight:bold}
.status-value{color:#00d4ff}
.connected{color:#00ff00}
.disconnected{color:#ff4444}
.btn-group{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin:20px 0}
.btn{background:#00d4ff;color:#000;border:none;padding:15px;border-radius:6px;font-size:16px;cursor:pointer;font-weight:bold;transition:all 0.3s}
.btn:hover{background:#00a8cc;transform:translateY(-2px)}
.btn:active{transform:translateY(0)}
.results{background:#2a2a2a;border-radius:8px;padding:15px;margin:10px 0;max-height:300px;overflow-y:auto}
.result-item{padding:8px;margin:5px 0;background:#3a3a3a;border-radius:4px;cursor:pointer;transition:background 0.2s}
.result-item:hover{background:#4a4a4a}
.ota-section{background:#2a2a2a;border-radius:8px;padding:20px;margin:20px 0;border-left:4px solid #ff9500}
.file-input{display:none}
.file-label{display:block;background:#ff9500;color:#000;padding:15px;border-radius:6px;text-align:center;cursor:pointer;font-weight:bold;transition:all 0.3s}
.file-label:hover{background:#cc7700}
.progress-bar{width:100%;height:30px;background:#3a3a3a;border-radius:6px;overflow:hidden;margin:15px 0;display:none}
.progress-fill{height:100%;background:#00ff00;transition:width 0.3s;display:flex;align-items:center;justify-content:center;color:#000;font-weight:bold}
</style>
</head>
<body>
<div class="container">
<h1>🖱️ ESP32 Maus-Tracker</h1>

<div class="status-box">
<h2>📊 Status</h2>
<div class="status-item">
<span class="status-label">Maus:</span>
<span id="mouseStatus" class="status-value disconnected">Getrennt</span>
</div>
<div class="status-item">
<span class="status-label">Position:</span>
<span id="mousePos" class="status-value">0, 0</span>
</div>
<div class="status-item">
<span class="status-label">Tasten:</span>
<span id="mouseButtons" class="status-value">Keine</span>
</div>
<div class="status-item">
<span class="status-label">WLAN Clients:</span>
<span id="wifiClients" class="status-value">0</span>
</div>
</div>

<div class="status-box">
<h2>🔍 Geräte scannen</h2>
<div class="btn-group">
<button class="btn" onclick="scanBLE()">🔵 BLE-Mäuse</button>
<button class="btn" onclick="scanBT()">🔵 Bluetooth</button>
<button class="btn" onclick="scanUSB()">🔌 USB-Mäuse</button>
<button class="btn" onclick="scanWiFi()">📶 WLAN</button>
</div>
<div id="scanResults" class="results" style="display:none"></div>
</div>

<div class="ota-section">
<h2>🔄 Firmware-Update (OTA)</h2>
<form id="otaForm" enctype="multipart/form-data">
<input type="file" id="firmwareFile" name="firmware" accept=".bin" class="file-input" onchange="updateFileName()">
<label for="firmwareFile" class="file-label" id="fileLabel">📁 Firmware auswählen (.bin)</label>
<div class="progress-bar" id="progressBar">
<div class="progress-fill" id="progressFill">0%</div>
</div>
<button type="submit" class="btn" style="width:100%;margin-top:15px;display:none" id="uploadBtn">⬆️ Upload starten</button>
</form>
</div>

</div>

<script>
// Status-Update alle 2 Sekunden
setInterval(updateStatus, 2000);
updateStatus();

async function updateStatus() {
  try {
    const res = await fetch('/status');
    const data = await res.json();
    document.getElementById('mouseStatus').textContent = data.connected ? 'Verbunden' : 'Getrennt';
    document.getElementById('mouseStatus').className = 'status-value ' + (data.connected ? 'connected' : 'disconnected');
    document.getElementById('mousePos').textContent = data.x + ', ' + data.y;
    
    let buttons = [];
    if (data.leftButton) buttons.push('Links');
    if (data.rightButton) buttons.push('Rechts');
    if (data.middleButton) buttons.push('Mitte');
    document.getElementById('mouseButtons').textContent = buttons.length ? buttons.join(' + ') : 'Keine';
    document.getElementById('wifiClients').textContent = data.clients || 0;
  } catch(e) {
    console.error('Status-Update fehlgeschlagen:', e);
  }
}

async function scanBLE() {
  showResults('Scanne BLE-Geräte...');
  try {
    const res = await fetch('/scan/ble');
    const data = await res.json();
    displayResults(data, 'BLE-Mäuse');
  } catch(e) {
    showResults('Fehler: ' + e.message);
  }
}

async function scanBT() {
  showResults('Scanne Bluetooth-Geräte...');
  try {
    const res = await fetch('/scan/bt');
    const data = await res.json();
    displayResults(data, 'Bluetooth-Geräte');
  } catch(e) {
    showResults('Fehler: ' + e.message);
  }
}

async function scanUSB() {
  showResults('Scanne USB-Geräte...');
  try {
    const res = await fetch('/scan/usb');
    const data = await res.json();
    displayResults(data, 'USB-Geräte');
  } catch(e) {
    showResults('Fehler: ' + e.message);
  }
}

async function scanWiFi() {
  showResults('Scanne WLAN-Netze...');
  try {
    const res = await fetch('/scan/wifi');
    const data = await res.json();
    displayWiFiResults(data);
  } catch(e) {
    showResults('Fehler: ' + e.message);
  }
}

function showResults(text) {
  const div = document.getElementById('scanResults');
  div.style.display = 'block';
  div.innerHTML = '<p>' + text + '</p>';
}

function displayResults(devices, title) {
  const div = document.getElementById('scanResults');
  div.style.display = 'block';
  
  if (!devices || devices.length === 0) {
    div.innerHTML = '<p>Keine ' + title + ' gefunden</p>';
    return;
  }
  
  let html = '<h3>' + title + ' (' + devices.length + ')</h3>';
  devices.forEach(d => {
    html += '<div class="result-item">';
    html += '<strong>' + (d.name || 'Unbekannt') + '</strong><br>';
    if (d.address) html += 'Adresse: ' + d.address + '<br>';
    if (d.rssi) html += 'Signal: ' + d.rssi + ' dBm';
    if (d.info) html += d.info;
    html += '</div>';
  });
  div.innerHTML = html;
}

function displayWiFiResults(networks) {
  const div = document.getElementById('scanResults');
  div.style.display = 'block';
  
  if (!networks || networks.length === 0) {
    div.innerHTML = '<p>Keine WLAN-Netze gefunden</p>';
    return;
  }
  
  let html = '<h3>WLAN-Netze (' + networks.length + ')</h3>';
  networks.forEach(n => {
    html += '<div class="result-item" onclick="connectWiFi(\'' + n.ssid + '\')">';
    html += '<strong>' + n.ssid + '</strong><br>';
    html += 'Signal: ' + n.rssi + ' dBm | Kanal: ' + n.channel;
    html += '</div>';
  });
  div.innerHTML = html;
}

function connectWiFi(ssid) {
  const password = prompt('Passwort für ' + ssid + ':');
  if (password) {
    fetch('/wifi/connect?ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password))
      .then(res => res.json())
      .then(data => alert(data.success ? 'Verbunden!' : 'Fehler: ' + data.error))
      .catch(e => alert('Verbindung fehlgeschlagen: ' + e.message));
  }
}

function updateFileName() {
  const input = document.getElementById('firmwareFile');
  const label = document.getElementById('fileLabel');
  const btn = document.getElementById('uploadBtn');
  
  if (input.files.length > 0) {
    label.textContent = '✅ ' + input.files[0].name;
    btn.style.display = 'block';
  }
}

document.getElementById('otaForm').onsubmit = async function(e) {
  e.preventDefault();
  
  const file = document.getElementById('firmwareFile').files[0];
  if (!file) {
    alert('Bitte Firmware-Datei auswählen');
    return;
  }
  
  const progressBar = document.getElementById('progressBar');
  const progressFill = document.getElementById('progressFill');
  progressBar.style.display = 'block';
  
  const formData = new FormData();
  formData.append('firmware', file);
  
  try {
    const xhr = new XMLHttpRequest();
    
    xhr.upload.onprogress = function(e) {
      if (e.lengthComputable) {
        const percent = Math.round((e.loaded / e.total) * 100);
        progressFill.style.width = percent + '%';
        progressFill.textContent = percent + '%';
      }
    };
    
    xhr.onload = function() {
      if (xhr.status === 200) {
        progressFill.textContent = '✅ Upload erfolgreich! Neustart...';
        setTimeout(() => location.reload(), 10000);
      } else {
        alert('Upload fehlgeschlagen: ' + xhr.statusText);
        progressBar.style.display = 'none';
      }
    };
    
    xhr.onerror = function() {
      alert('Upload-Fehler');
      progressBar.style.display = 'none';
    };
    
    xhr.open('POST', '/ota/upload');
    xhr.send(formData);
  } catch(e) {
    alert('Fehler: ' + e.message);
    progressBar.style.display = 'none';
  }
};
</script>
</body>
</html>
)rawliteral";

// ========== Konstruktor ==========

WebServerManager::WebServerManager() 
  : server(nullptr),
    pMouseHandler(nullptr),
    pNetworkManager(nullptr) {
}

// ========== Öffentliche Methoden ==========

bool WebServerManager::begin(MouseHandler* mouseHandler, NetworkManager* networkManager) {
  pMouseHandler = mouseHandler;
  pNetworkManager = networkManager;
  
  server = new AsyncWebServer(80);
  setupRoutes();
  server->begin();
  
  Serial.println("[WEB] Server gestartet auf Port 80");
  return true;
}

void WebServerManager::handleClient() {
  // AsyncWebServer läuft automatisch im Hintergrund
}

// ========== Private Methoden ==========

void WebServerManager::setupRoutes() {
  // Root-Seite
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest* req) {
    req->send_P(200, "text/html", HTML_PAGE);
  });
  
  // Status-API
  server->on("/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
    handleStatus(req);
  });
  
  // Scan-APIs
  server->on("/scan/ble", HTTP_GET, [this](AsyncWebServerRequest* req) {
    handleScanBLE(req);
  });
  
  server->on("/scan/bt", HTTP_GET, [this](AsyncWebServerRequest* req) {
    handleScanBT(req);
  });
  
  server->on("/scan/usb", HTTP_GET, [this](AsyncWebServerRequest* req) {
    handleScanUSB(req);
  });
  
  server->on("/scan/wifi", HTTP_GET, [this](AsyncWebServerRequest* req) {
    handleScanWiFi(req);
  });
  
  // WiFi-Verbindung
  server->on("/wifi/connect", HTTP_GET, [this](AsyncWebServerRequest* req) {
    handleConnectWiFi(req);
  });
  
  // OTA-Upload
  server->on("/ota/upload", HTTP_POST, 
    [](AsyncWebServerRequest* req) {
      req->send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
      delay(1000);
      ESP.restart();
    },
    [this](AsyncWebServerRequest* req, String filename, size_t index, uint8_t* data, size_t len, bool final) {
      handleOTAUpload(req, filename, index, data, len, final);
    }
  );
}

void WebServerManager::handleStatus(AsyncWebServerRequest* request) {
  MouseData data = pMouseHandler->getMouseData();
  
  StaticJsonDocument<256> doc;
  doc["connected"] = pMouseHandler->isMouseConnected();
  doc["x"] = data.x;
  doc["y"] = data.y;
  doc["leftButton"] = data.leftButton;
  doc["rightButton"] = data.rightButton;
  doc["middleButton"] = data.middleButton;
  doc["clients"] = pNetworkManager->getAPClients();
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleScanBLE(AsyncWebServerRequest* request) {
  String result = pMouseHandler->scanBLEMice();
  request->send(200, "application/json", result);
}

void WebServerManager::handleScanBT(AsyncWebServerRequest* request) {
  String result = pMouseHandler->scanBluetoothMice();
  request->send(200, "application/json", result);
}

void WebServerManager::handleScanUSB(AsyncWebServerRequest* request) {
  String result = pMouseHandler->scanUSBMice();
  request->send(200, "application/json", result);
}

void WebServerManager::handleScanWiFi(AsyncWebServerRequest* request) {
  String result = pNetworkManager->scanWiFiNetworks();
  request->send(200, "application/json", result);
}

void WebServerManager::handleConnectWiFi(AsyncWebServerRequest* request) {
  if (!request->hasParam("ssid") || !request->hasParam("password")) {
    request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
    return;
  }
  
  String ssid = request->getParam("ssid")->value();
  String password = request->getParam("password")->value();
  
  bool success = pNetworkManager->connectToWiFi(ssid.c_str(), password.c_str());
  
  StaticJsonDocument<128> doc;
  doc["success"] = success;
  if (success) {
    doc["ip"] = pNetworkManager->getStationIP().toString();
  } else {
    doc["error"] = "Connection failed";
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleOTAUpload(AsyncWebServerRequest* request, String filename, 
                                        size_t index, uint8_t* data, size_t len, bool final) {
  if (!index) {
    Serial.printf("[OTA] Update Start: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  }
  
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }
  
  if (final) {
    if (Update.end(true)) {
      Serial.printf("[OTA] Update Success: %u bytes\n", index + len);
    } else {
      Update.printError(Serial);
    }
  }
}
