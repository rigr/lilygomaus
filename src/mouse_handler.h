/**
 * Mouse Handler für USB, Bluetooth Classic und BLE-Mäuse
 * Vollständige Implementierung für alle drei Typen
 */

#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <USB.h>
#include <USBHID.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_hid_gap.h"

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
  
  // BT-Classic-spezifisch
  bool btClassicInitialized;
  bool btClassicConnected;
  esp_bd_addr_t btClassicAddress;
  
  // USB-spezifisch
  bool usbConnected;
  USBHID* usbHID;
  
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
  
  // Private Methoden - BT Classic
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
  void scanBLEMice(void (*callback)(BLEMouseDevice device));
  
  // BT-Classic-Funktionen
  bool connectBTClassicMouse(const char* address);
  void scanBTClassicMice(void (*callback)(BTClassicDevice device));
  
  // USB-Funktionen
  bool connectUSBMouse();
  void scanUSBMice(void (*callback)(USBMouseDevice device));
  
  // Trennen
  void disconnectMouse();
};

#endif
