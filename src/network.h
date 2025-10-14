/**
 * Netzwerk-Management für Access Point und Station Mode
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <WiFi.h>
#include <Arduino.h>

// AP-Konfiguration
#define AP_SSID "ESPMAUS"
#define AP_PASSWORD "MausMaus"
#define AP_CHANNEL 6
#define AP_MAX_CONNECTIONS 4

class NetworkManager {
private:
  bool apEnabled;
  bool stationEnabled;
  bool stationConnected;
  
  String stationSSID;
  String stationPassword;
  
  IPAddress apIP;

public:
  NetworkManager();
  
  bool begin();
  void update();
  
  // Access Point
  bool startAccessPoint();
  const char* getAPSSID();
  const char* getAPPassword();
  IPAddress getAPIP();
  
  // Station Mode
  bool connectToWiFi(const char* ssid, const char* password);
  void disconnectWiFi();
  bool reconnectStation();
  bool isStationEnabled();
  bool isStationConnected();
  IPAddress getStationIP();
  
  // WiFi-Scanning
  int scanNetworks();
  String getScannedSSID(int index);
  int getScannedRSSI(int index);
  bool getScannedEncryption(int index);
};

#endif

