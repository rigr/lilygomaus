/**
 * Mouse Handler für USB, Bluetooth Classic und BLE-Mäuse
 * Vereinfachte Version ohne ESP-HID Dependencies
 */

#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <Arduino.h>
#include <functional>
#include <NimBLEDevice.h>

// Bluetooth Classic (nur Basic-APIs, kein HID-Host)
#ifdef CONFIG_BT_ENABLED
  #include "esp_bt_main.h"
  #include "esp_bt_device.h"
  #include "esp_gap_bt_api.h"
#endif

// Maus-Typen
enum MouseType {
  MOUSE_NONE,
  MOUSE_BLE,
  MOUSE_BT_CLASSIC,
  MOUSE_USB
};

// Maus-Daten Struktur
struct MouseData {
  int x;
  int y;
  bool leftButton;
  bool rightButton;
  float speed;
  MouseType type;
};

// BLE-Scan-Ergebnis
struct BLEMouseDevice {
  String name;
  String address;
  int rssi;
};

// BT-Classic-Scan-Ergebnis
struct BTClassicDevice {
  String name;
  String address;
  int rssi;
};

// USB-Device-Info
struct USBMouseDevice {
  String name;
  uint16_t vendorId;
  uint16_t productId;
};

class MouseHandler {
private:
  // BLE-spezifisch
  NimBLEClient* bleClient;
  NimBLERemoteCharacteristic* bleMouseCharacteristic;
  bool bleConnected;
  
  // BT-Classic-spezifisch (vereinfacht ohne HID-Host)
  bool btClassicInitialized;
  bool btClassicConnected;
  uint8_t btClassicAddress[6];
  
  // USB-spezifisch
  bool usbConnected;
  
  // Aktueller Maus-Typ
  MouseType currentMouseType;
  
  // Maus-Daten
  MouseData currentData;
  int lastX, lastY;
  unsigned long lastUpdateTime;
  
  // Private Methoden - BLE
  bool connectBLE(const char* address);
  void disconnectBLE();
  void processBLEMouseReport(uint8_t* data, size_t length);
  static void notifyCallback(NimBLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify);
  
  // Private Methoden - BT Classic (vereinfacht)
  bool initBTClassic();
  bool connectBTClassic(const char* address);
  void disconnectBTClassic();
  static void btClassicGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param);
  void processBTClassicData(uint8_t* data, size_t length);
  
  // Private Methoden - USB
  bool initUSB();
  void disconnectUSB();
  void processUSBMouseReport(uint8_t* data, size_t length);
  static void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
  
  // Gemeinsame Hilfsfunktionen
  float calculateSpeed();
  void updateMousePosition(int8_t dx, int8_t dy);
  void updateMouseButtons(bool left, bool right);

public:
  MouseHandler();
  
  bool begin();
  void update();
  
  // Status
  bool isMouseConnected();
  MouseData getMouseData();
  MouseType getMouseType();
  
  // BLE-Funktionen
  bool connectBLEMouse(const char* address);
  void scanBLEMice(std::function<void(BLEMouseDevice)> callback);
  
  // BT-Classic-Funktionen (vereinfacht)
  bool connectBTClassicMouse(const char* address);
  void scanBTClassicMice(std::function<void(BTClassicDevice)> callback);
  
  // USB-Funktionen (Stubs)
  bool connectUSBMouse();
  void scanUSBMice(std::function<void(USBMouseDevice)> callback);
  
  // Trennen
  void disconnectMouse();
};

#endif
