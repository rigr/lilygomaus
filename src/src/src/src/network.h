/**
 * Netzwerk-Management für Access Point und Station Mode
 * 
 * @file network.h
 * @author rigr
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>

// ========== Netzwerk-Konstanten ==========

#define AP_SSID "ESPMAUS"
#define AP_PASSWORD "MausMaus"
#define AP_CHANNEL 6
#define AP_MAX_CONNECTIONS 4
#define AP_IP IPAddress(192, 168, 4, 1)
#define AP_GATEWAY IPAddress(192, 168, 4, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)

// ========== Klasse: NetworkManager ==========

class NetworkManager {
public:
  NetworkManager();
  
  bool begin();
  void update();
  
  // Access Point
  const char* getAPSSID() const { return AP_SSID; }
  const char* getAPPassword() const { return AP_PASSWORD; }
  IPAddress getAPIP() const { return apIP; }
  int getAPClients() const { return WiFi.softAPgetStationNum(); }
  
  // Station Mode
  bool connectToWiFi(const char* ssid, const char* password);
  void disconnectWiFi();
  bool isStationEnabled() const { return stationEnabled; }
  bool isStationConnected() const { return WiFi.status() == WL_CONNECTED; }
  IPAddress getStationIP() const { return WiFi.localIP(); }
  void reconnectStation();
  
  // Scanning
  String scanWiFiNetworks();

private:
  IPAddress apIP;
  bool stationEnabled;
  String stationSSID;
  String stationPassword;
  
  unsigned long lastReconnectAttempt;
};

#endif // NETWORK_H
