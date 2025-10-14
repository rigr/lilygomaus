
/**
 * Netzwerk-Implementierung
 */

#include "network.h"

NetworkManager::NetworkManager() {
  apEnabled = false;
  stationEnabled = false;
  stationConnected = false;
  apIP = IPAddress(192, 168, 4, 1);
}

bool NetworkManager::begin() {
  WiFi.mode(WIFI_AP_STA); // Dual-Mode: AP + Station
  
  // Access Point starten
  return startAccessPoint();
}

bool NetworkManager::startAccessPoint() {
  Serial.println("[NETWORK] Starte Access Point...");
  
  if (!WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0))) {
    Serial.println("[NETWORK] AP-Konfiguration fehlgeschlagen");
    return false;
  }
  
  if (!WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONNECTIONS)) {
    Serial.println("[NETWORK] AP-Start fehlgeschlagen");
    return false;
  }
  
  apEnabled = true;
  Serial.printf("[NETWORK] AP gestartet: %s\n", AP_SSID);
  Serial.printf("[NETWORK] IP: %s\n", apIP.toString().c_str());
  
  return true;
}

void NetworkManager::update() {
  // Check station connection status
  if (stationEnabled) {
    stationConnected = WiFi.status() == WL_CONNECTED;
  }
}

const char* NetworkManager::getAPSSID() {
  return AP_SSID;
}

const char* NetworkManager::getAPPassword() {
  return AP_PASSWORD;
}

IPAddress NetworkManager::getAPIP() {
  return apIP;
}

bool NetworkManager::connectToWiFi(const char* ssid, const char* password) {
  Serial.printf("[NETWORK] Verbinde mit %s...\n", ssid);
  
  stationSSID = ssid;
  stationPassword = password;
  
  WiFi.begin(ssid, password);
  
  // Warte max. 10 Sekunden
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    stationEnabled = true;
    stationConnected = true;
    Serial.printf("\n[NETWORK] Verbunden! IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    Serial.println("\n[NETWORK] Verbindung fehlgeschlagen");
    return false;
  }
}

void NetworkManager::disconnectWiFi() {
  WiFi.disconnect();
  stationEnabled = false;
  stationConnected = false;
  Serial.println("[NETWORK] WiFi getrennt");
}

bool NetworkManager::reconnectStation() {
  if (!stationEnabled || stationSSID.length() == 0) {
    return false;
  }
  
  return connectToWiFi(stationSSID.c_str(), stationPassword.c_str());
}

bool NetworkManager::isStationEnabled() {
  return stationEnabled;
}

bool NetworkManager::isStationConnected() {
  return stationConnected;
}

IPAddress NetworkManager::getStationIP() {
  return WiFi.localIP();
}

int NetworkManager::scanNetworks() {
  Serial.println("[NETWORK] Scanne WiFi-Netzwerke...");
  return WiFi.scanNetworks();
}

String NetworkManager::getScannedSSID(int index) {
  return WiFi.SSID(index);
}

int NetworkManager::getScannedRSSI(int index) {
  return WiFi.RSSI(index);
}

bool NetworkManager::getScannedEncryption(int index) {
  return WiFi.encryptionType(index) != WIFI_AUTH_OPEN;
}
