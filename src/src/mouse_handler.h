/**
 * Maus-Input-Handler für USB, Bluetooth Classic und BLE
 * 
 * @file mouse_handler.h
 * @author rigr
 */

#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <Arduino.h>
#include <NimBLEDevice.h>

// ========== Konstanten ==========

#define MAX_MOUSE_SPEED 50.0f  // Maximum für Speed-Berechnung

// ========== Strukturen ==========

/**
 * Maus-Daten für Display-Rendering
 */
struct MouseData {
  int x;              // X-Position (Bildschirm-Koordinaten)
  int y;              // Y-Position
  float speed;        // Bewegungsgeschwindigkeit (für Helligkeit)
  bool leftButton;    // Linke Maustaste gedrückt
  bool rightButton;   // Rechte Maustaste gedrückt
  bool middleButton;  // Mittlere Maustaste gedrückt
};

// ========== Klasse: MouseHandler ==========

/**
 * Verwaltet Maus-Input von verschiedenen Quellen
 */
class MouseHandler {
public:
  MouseHandler();
  
  bool begin();
  void update();
  
  bool isMouseConnected() const { return mouseConnected; }
  MouseData getMouseData() const { return currentData; }
  
  // Scan-Funktionen für Webinterface
  String scanBLEMice();
  String scanBluetoothMice();
  String scanUSBMice();

private:
  MouseData currentData;
  bool mouseConnected;
  
  // Letzte Position für Geschwindigkeitsberechnung
  int lastX, lastY;
  unsigned long lastUpdateTime;
  
  // BLE
  NimBLEClient* pClient;
  NimBLERemoteCharacteristic* pMouseCharacteristic;
  
  void updateBLE();
  void updateUSB();
  void updateClassicBT();
  
  void calculateSpeed();
};

#endif // MOUSE_HANDLER_H
