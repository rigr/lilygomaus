/**
 * Webserver und OTA-Management
 * 
 * @file webserver.h
 * @author rigr
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <ArduinoJson.h>
#include "mouse_handler.h"
#include "network.h"

// ========== Klasse: WebServerManager ==========

class WebServerManager {
public:
  WebServerManager();
  
  bool begin(MouseHandler* mouseHandler, NetworkManager* networkManager);
  void handleClient();

private:
  AsyncWebServer* server;
  MouseHandler* pMouseHandler;
  NetworkManager* pNetworkManager;
  
  void setupRoutes();
  void handleRoot(AsyncWebServerRequest* request);
  void handleStatus(AsyncWebServerRequest* request);
  void handleScanBLE(AsyncWebServerRequest* request);
  void handleScanBT(AsyncWebServerRequest* request);
  void handleScanUSB(AsyncWebServerRequest* request);
  void handleScanWiFi(AsyncWebServerRequest* request);
  void handleConnectWiFi(AsyncWebServerRequest* request);
  void handleOTAUpload(AsyncWebServerRequest* request, String filename, 
                       size_t index, uint8_t* data, size_t len, bool final);
};

#endif // WEBSERVER_H
