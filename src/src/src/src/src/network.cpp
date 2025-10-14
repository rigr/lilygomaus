/**
 * Netzwerk-Management Implementierung
 * 
 * @file network.cpp
 * @author rigr
 */

#include "network.h"

// ========== Konstruktor ==========

NetworkManager::NetworkManager() 
  : apIP(AP_IP),
    stationEnabled(false),
    lastReconnectAttempt(0) {
}

// ========== Öffentliche Methoden ==========

bool NetworkManager::begin() {
  Serial.println("[NETWORK] Starte Access Point...");
  
  // WiFi-Modus: AP + Station
  WiFi.mode(WIFI_AP_STA);
  
  // Access Point konfigurieren
  if (!WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET)) {
    Serial.println("[ERROR] AP-Konfiguration fehlgeschlagen!");
    return false;
  }
  
  // Access Point starten
  if (!WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, 0, AP_MAX_CONNECTIONS)) {
    Serial.println("[ERROR] AP-Start fehlgeschlagen!");
    return false;
  }
  
  apIP = WiFi.softAPIP();
  Serial.printf("[NETWORK] Access Point gestartet\n");
  Serial.printf("  SSID: %s\n", AP_SSID);
  Serial.printf("  Password: %s\n", AP_PASSWORD);
  Serial.printf("  IP: %s\n", apIP.toString().c_str());
  Serial.printf("  Channel: %d\n", AP_CHANNEL);
  
  return true;
}

void NetworkManager::update() {
  // Prüfe Station-Verbindung
  if (stationEnabled && !isStationConnected()) {
    // Reconnect-Logik wird in main.cpp gehandhabt
  }
  
  // Optional: Client-Count loggen
  static int lastClientCount = 0;
  int currentClientCount = getAPClients();
  if (currentClientCount != lastClientCount) {
    Serial.printf("[NETWORK] AP Clients: %d\n", currentClientCount);
    lastClientCount = currentClientCount;
  }
}

bool NetworkManager::connectToWiFi(const char* ssid, const char* password) {
  Serial.printf("[NETWORK] Verbinde mit WLAN: %s\n", ssid);
  
  stationSSID = ssid;
  stationPassword = password;
  stationEnabled = true;
  
  WiFi.begin(ssid, password);
  
  // Warte bis zu 10 Sekunden auf Verbindung
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[NETWORK] WLAN verbunden!\n");
    Serial.printf("  IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    Serial.println("[NETWORK] WLAN-Verbindung fehlgeschlagen!");
    return false;
  }
}

void NetworkManager::disconnectWiFi() {
  if (stationEnabled) {
    Serial.println("[NETWORK] Trenne WLAN...");
    WiFi.disconnect();
    stationEnabled = false;
  }
}

void NetworkManager::reconnectStation() {
  if (!stationEnabled) return;
  
  unsigned long now = millis();
  if (now - lastReconnectAttempt < 30000) return;  // Max. alle 30 Sekunden
  
  lastReconnectAttempt = now;
  Serial.println("[NETWORK] Versuche Reconnect...");
  
  WiFi.disconnect();
  delay(100);
  WiFi.begin(stationSSID.c_str(), stationPassword.c_str());
}

String NetworkManager::scanWiFiNetworks() {
  Serial.println("[NETWORK] Scanne WLAN-Netze...");
  
  String result = "[";
  int n = WiFi.scanNetworks();
  
  for (int i = 0; i < n; i++) {
    if (i > 0) result += ",";
    
    result += "{";
    result += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    result += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    result += "\"encryption\":\"" + String(WiFi.encryptionType(i)) + "\",";
    result += "\"channel\":" + String(WiFi.channel(i));
    result += "}";
  }
  
  result += "]";
  
  WiFi.scanDelete();
  Serial.printf("[NETWORK] %d Netzwerke gefunden\n", n);
  
  return result;
}
