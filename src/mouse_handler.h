
/**
 * Mouse Handler für USB, Bluetooth Classic und BLE-Mäuse
 */

#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

// Maus-Daten Struktur
struct MouseData {
  int x;
  int y;
  bool leftButton;
  bool rightButton;
  float speed;
};

// BLE-Scan-Ergebnis
struct BLEMouseDevice {
  String name;
  String address;
  int rssi;
};

class MouseHandler {
private:
  // BLE-spezifisch
  NimBLEClient* bleClient;
  NimBLERemoteCharacteristic* bleMouseCharacteristic;
  bool bleConnected;
  
  // Maus-Daten
  MouseData currentData;
  int lastX, lastY;
  unsigned long lastUpdateTime;
  
  // BLE HID Report Descriptor IDs
  static const uint8_t HID_REPORT_ID_MOUSE = 0x02;
  
  // Private Methoden
  bool connectBLE(const char* address);
  void disconnectBLE();
  void processBLEMouseReport(uint8_t* data, size_t length);
  float calculateSpeed();
  
  // BLE Callbacks
  static void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);

public:
  MouseHandler();
  
  bool begin();
  void update();
  
  // Status
  bool isMouseConnected();
  MouseData getMouseData();
  
  // BLE-Funktionen
  bool connectBLEMouse(const char* address);
  void scanBLEMice(void (*callback)(BLEMouseDevice device));
  void disconnectMouse();
  
  // USB und BT-Classic Platzhalter (für zukünftige Implementierung)
  void scanUSBMice(void (*callback)(const char* deviceName));
  void scanBTClassicMice(void (*callback)(const char* deviceName, const char* address));
};

#endif
