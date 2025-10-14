
/**
 * Webserver-Implementierung
 */

#include "webserver.h"

WebServerManager::WebServerManager() {
  server = nullptr;
  mouseHandler = nullptr;
  networkManager = nullptr;
}

bool WebServerManager::begin(MouseHandler* mouse, NetworkManager* network) {
  mouseHandler = mouse;
  networkManager = network;
  
  server = new AsyncWebServer(80);
  
  // ========== Routes ==========
  
  // Hauptseite
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
    request->send(200, "text/html", getIndexHTML());
  });
  
  // Status-API
  server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleStatus(request);
  });
  
  // BLE-Scan
  server->on("/api/scan/ble", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleScanBLE(request);
  });
  
  // BT-Classic-Scan
  server->on("/api/scan/bt", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleScanBT(request);
  });
  
  // USB-Scan
  server->on("/api/scan/usb", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleScanUSB(request);
  });
  
  // WiFi-Scan
  server->on("/api/scan/wifi", HTTP_GET, [this](AsyncWebServerRequest* request) {
    handleScanWiFi(request);
  });
  
  // Maus verbinden
  server->on("/api/connect/mouse", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleConnectMouse(request);
  });
  
  // WiFi verbinden
  server->on("/api/connect/wifi", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleConnectWiFi(request);
  });
  
  // Trennen
  server->on("/api/disconnect", HTTP_POST, [this](AsyncWebServerRequest* request) {
    handleDisconnect(request);
  });
  
  // OTA-Upload
  server->on("/api/ota", HTTP_POST,
    [](AsyncWebServerRequest* request) {
      request->send(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
      delay(100);
      ESP.restart();
    },
    [this](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
      handleOTAUpload(request, filename, index, data, len, final);
    }
  );
  
  server->begin();
  Serial.println("[WEBSERVER] Server gestartet auf Port 80");
  
  return true;
}

void WebServerManager::handleStatus(AsyncWebServerRequest* request) {
  StaticJsonDocument<512> doc;
  
  // Maus-Status
  doc["mouseConnected"] = mouseHandler->isMouseConnected();
  
  if (mouseHandler->isMouseConnected()) {
    MouseData data = mouseHandler->getMouseData();
    doc["mouseX"] = data.x;
    doc["mouseY"] = data.y;
    doc["leftButton"] = data.leftButton;
    doc["rightButton"] = data.rightButton;
    doc["speed"] = data.speed;
  }
  
  // Netzwerk-Status
  doc["apSSID"] = networkManager->getAPSSID();
  doc["apIP"] = networkManager->getAPIP().toString();
  doc["stationConnected"] = networkManager->isStationConnected();
  
  if (networkManager->isStationConnected()) {
    doc["stationIP"] = networkManager->getStationIP().toString();
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleScanBLE(AsyncWebServerRequest* request) {
  StaticJsonDocument<2048> doc;
  JsonArray devices = doc.createNestedArray("devices");
  
  mouseHandler->scanBLEMice([&devices](BLEMouseDevice device) {
    JsonObject dev = devices.createNestedObject();
    dev["name"] = device.name;
    dev["address"] = device.address;
    dev["rssi"] = device.rssi;
  });
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleScanBT(AsyncWebServerRequest* request) {
  StaticJsonDocument<2048> doc;
  JsonArray devices = doc.createNestedArray("devices");
  
  mouseHandler->scanBTClassicMice([&devices](BTClassicDevice device) {
    JsonObject dev = devices.createNestedObject();
    dev["name"] = device.name;
    dev["address"] = device.address;
    dev["rssi"] = device.rssi;
  });
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleScanUSB(AsyncWebServerRequest* request) {
  StaticJsonDocument<2048> doc;
  JsonArray devices = doc.createNestedArray("devices");
  
  mouseHandler->scanUSBMice([&devices](USBMouseDevice device) {
    JsonObject dev = devices.createNestedObject();
    dev["name"] = device.name;
    dev["vendorId"] = device.vendorId;
    dev["productId"] = device.productId;
  });
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleScanWiFi(AsyncWebServerRequest* request) {
  StaticJsonDocument<2048> doc;
  JsonArray networks = doc.createNestedArray("networks");
  
  int n = networkManager->scanNetworks();
  
  for (int i = 0; i < n; i++) {
    JsonObject net = networks.createNestedObject();
    net["ssid"] = networkManager->getScannedSSID(i);
    net["rssi"] = networkManager->getScannedRSSI(i);
    net["encrypted"] = networkManager->getScannedEncryption(i);
  }
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleConnectMouse(AsyncWebServerRequest* request) {
  if (!request->hasParam("address", true)) {
    request->send(400, "text/plain", "Missing address parameter");
    return;
  }
  
  String address = request->getParam("address", true)->value();
  bool success = mouseHandler->connectBLEMouse(address.c_str());
  
  StaticJsonDocument<128> doc;
  doc["success"] = success;
  doc["message"] = success ? "Verbunden" : "Verbindung fehlgeschlagen";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleConnectWiFi(AsyncWebServerRequest* request) {
  if (!request->hasParam("ssid", true) || !request->hasParam("password", true)) {
    request->send(400, "text/plain", "Missing parameters");
    return;
  }
  
  String ssid = request->getParam("ssid", true)->value();
  String password = request->getParam("password", true)->value();
  
  bool success = networkManager->connectToWiFi(ssid.c_str(), password.c_str());
  
  StaticJsonDocument<128> doc;
  doc["success"] = success;
  doc["message"] = success ? "Verbunden" : "Verbindung fehlgeschlagen";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleDisconnect(AsyncWebServerRequest* request) {
  mouseHandler->disconnectMouse();
  
  StaticJsonDocument<128> doc;
  doc["success"] = true;
  doc["message"] = "Getrennt";
  
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}

void WebServerManager::handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
  if (!index) {
    Serial.printf("[OTA] Update Start: %s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  }
  
  if (len) {
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }
  }
  
  if (final) {
    if (Update.end(true)) {
      Serial.printf("[OTA] Update erfolgreich: %u bytes\n", index + len);
    } else {
      Update.printError(Serial);
    }
  }
}

const char* WebServerManager::getIndexHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>LilyGoMaus Control</title>
<style>
body{font-family:Arial,sans-serif;max-width:800px;margin:20px auto;padding:20px;background:#1a1a1a;color:#fff}
h1,h2{color:#4CAF50}
button{background:#4CAF50;color:#fff;border:none;padding:10px 20px;margin:5px;cursor:pointer;border-radius:4px}
button:hover{background:#45a049}
button:disabled{background:#666;cursor:not-allowed}
.status{background:#2a2a2a;padding:15px;border-radius:8px;margin:10px 0}
.device-list{background:#333;padding:10px;border-radius:4px;max-height:300px;overflow-y:auto}
.device-item{padding:8px;margin:5px 0;background:#444;border-radius:4px;display:flex;justify-content:space-between;align-items:center}
input{padding:8px;margin:5px;border:1px solid #666;border-radius:4px;background:#2a2a2a;color:#fff;min-width:200px}
.error{color:#f44336}
.success{color:#4CAF50}
#mouseStatus{font-size:18px;font-weight:bold}
</style>
</head>
<body>
<h1>🖱️ LilyGoMaus Control</h1>

<div class="status">
  <h2>Status</h2>
  <div id="mouseStatus">Maus: Nicht verbunden</div>
  <div>AP: <span id="apInfo">-</span></div>
  <div>Station: <span id="stationInfo">Nicht verbunden</span></div>
</div>

<h2>🔍 BLE-Mäuse scannen</h2>
<button onclick="scanBLE()">Scan starten</button>
<div id="bleDevices" class="device-list"></div>

<h2>📶 WiFi-Netzwerke</h2>
<button onclick="scanWiFi()">Scan starten</button>
<div id="wifiNetworks" class="device-list"></div>

<h2>🔌 Maus trennen</h2>
<button onclick="disconnect()">Trennen</button>

<h2>📦 Firmware-Update (OTA)</h2>
<input type="file" id="otaFile" accept=".bin">
<button onclick="uploadOTA()">Upload</button>
<div id="otaStatus"></div>

<script>
let statusInterval;

async function updateStatus(){
  try{
    const res=await fetch('/api/status');
    const data=await res.json();
    
    document.getElementById('mouseStatus').innerHTML=
      'Maus: '+(data.mouseConnected?
        '<span class="success">Verbunden ('+data.mouseX+','+data.mouseY+')</span>':
        '<span class="error">Nicht verbunden</span>');
    
    document.getElementById('apInfo').textContent=data.apSSID+' ('+data.apIP+')';
    document.getElementById('stationInfo').textContent=
      data.stationConnected?'Verbunden ('+data.stationIP+')':'Nicht verbunden';
  }catch(e){console.error(e)}
}

async function scanBLE(){
  document.getElementById('bleDevices').innerHTML='<div>Scanne...</div>';
  try{
    const res=await fetch('/api/scan/ble');
    const data=await res.json();
    let html='';
    data.devices.forEach(d=>{
      html+=`<div class="device-item">
        <span>${d.name} (${d.address}) RSSI:${d.rssi}</span>
        <button onclick="connectMouse('${d.address}')">Verbinden</button>
      </div>`;
    });
    document.getElementById('bleDevices').innerHTML=html||'<div>Keine Geräte gefunden</div>';
  }catch(e){
    document.getElementById('bleDevices').innerHTML='<div class="error">Fehler beim Scan</div>';
  }
}

async function scanWiFi(){
  document.getElementById('wifiNetworks').innerHTML='<div>Scanne...</div>';
  try{
    const res=await fetch('/api/scan/wifi');
    const data=await res.json();
    let html='';
    data.networks.forEach(n=>{
      html+=`<div class="device-item">
        <span>${n.ssid} (RSSI:${n.rssi}) ${n.encrypted?'🔒':''}</span>
        <button onclick="connectWiFi('${n.ssid}')">Verbinden</button>
      </div>`;
    });
    document.getElementById('wifiNetworks').innerHTML=html||'<div>Keine Netzwerke gefunden</div>';
  }catch(e){
    document.getElementById('wifiNetworks').innerHTML='<div class="error">Fehler beim Scan</div>';
  }
}

async function connectMouse(addr){
  try{
    const form=new FormData();
    form.append('address',addr);
    const res=await fetch('/api/connect/mouse',{method:'POST',body:form});
    const data=await res.json();
    alert(data.message);
    updateStatus();
  }catch(e){alert('Fehler: '+e)}
}

async function connectWiFi(ssid){
  const pass=prompt('Passwort für '+ssid+':');
  if(!pass)return;
  try{
    const form=new FormData();
    form.append('ssid',ssid);
    form.append('password',pass);
    const res=await fetch('/api/connect/wifi',{method:'POST',body:form});
    const data=await res.json();
    alert(data.message);
    updateStatus();
  }catch(e){alert('Fehler: '+e)}
}

async function disconnect(){
  try{
    const res=await fetch('/api/disconnect',{method:'POST'});
    const data=await res.json();
    alert(data.message);
    updateStatus();
  }catch(e){alert('Fehler: '+e)}
}

async function uploadOTA(){
  const file=document.getElementById('otaFile').files[0];
  if(!file){alert('Keine Datei ausgewählt');return;}
  
  const form=new FormData();
  form.append('file',file);
  
  document.getElementById('otaStatus').innerHTML='<div>Uploading...</div>';
  
  try{
    const res=await fetch('/api/ota',{method:'POST',body:form});
    const text=await res.text();
    document.getElementById('otaStatus').innerHTML=
      text==='OK'?'<div class="success">Update erfolgreich! ESP startet neu...</div>':
      '<div class="error">Update fehlgeschlagen</div>';
  }catch(e){
    document.getElementById('otaStatus').innerHTML='<div class="error">Fehler: '+e+'</div>';
  }
}

updateStatus();
statusInterval=setInterval(updateStatus,2000);
</script>
</body>
</html>
)rawliteral";
}
