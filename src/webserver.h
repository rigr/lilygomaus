
/**
 * Webserver mit REST-API und OTA-Update
 */

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Update.h>
#include "mouse_handler.h"
#include "network.h"

class WebServerManager {
private:
  AsyncWebServer* server;
  MouseHandler* mouseHandler;
  NetworkManager* networkManager;
  
  // HTML-Interface (inline)
  const char* getIndexHTML();
  
  // Request-Handler
  void handleRoot(AsyncWebServerRequest* request);
  void handleStatus(AsyncWebServerRequest* request);
  void handleScanBLE(AsyncWebServerRequest* request);
  void handleScanBT(AsyncWebServerRequest* request);
  void handleScanUSB(AsyncWebServerRequest* request);
  void handleScanWiFi(AsyncWebServerRequest* request);
  void handleConnectMouse(AsyncWebServerRequest* request);
  void handleConnectWiFi(AsyncWebServerRequest* request);
  void handleDisconnect(AsyncWebServerRequest* request);
  void handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final);

public:
  WebServerManager();
  
  bool begin(MouseHandler* mouseHandler, NetworkManager* networkManager);
};

#endif
