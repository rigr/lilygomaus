/**
 * Maus-Input-Handler Implementierung
 * 
 * @file mouse_handler.cpp
 * @author rigr
 */

#include "mouse_handler.h"
#include "display.h"

// BLE HID Mouse Service UUID
static BLEUUID serviceUUID("00001812-0000-1000-8000-00805f9b34fb");
static BLEUUID charUUID("00002a4d-0000-1000-8000-00805f9b34fb");

// ========== Konstruktor ==========

MouseHandler::MouseHandler() 
  : mouseConnected(false),
    lastX(120), lastY(67),
    lastUpdateTime(0),
    pClient(nullptr),
    pMouseCharacteristic(nullptr) {
  
  currentData.x = 120;  // Bildschirmmitte
  currentData.y = 67;
  currentData.speed = 0;
  currentData.leftButton = false;
  currentData.rightButton = false;
  currentData.middleButton = false;
}

// ========== Öffentliche Methoden ==========

bool MouseHandler::begin() {
  Serial.println("[MOUSE] Initialisiere BLE...");
  
  // BLE initialisieren
  NimBLEDevice::init("ESP32Mouse");
  
  Serial.println("[MOUSE] Bereit für Maus-Verbindungen");
  return true;
}

void MouseHandler::update() {
  // Versuche verschiedene Input-Quellen
  updateBLE();
  updateUSB();
  updateClassicBT();
  
  // Geschwindigkeit berechnen
  calculateSpeed();
}

String MouseHandler::scanBLEMice() {
  String result = "[";
  
  Serial.println("[BLE] Starte Scan...");
  
  NimBLEScan* pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  NimBLEScanResults foundDevices = pBLEScan->start(5);
  
  for (int i = 0; i < foundDevices.getCount(); i++) {
    NimBLEAdvertisedDevice device = foundDevices.getDevice(i);
    
    // Nur HID-Geräte (Mäuse/Tastaturen)
    if (device.isAdvertisingService(serviceUUID)) {
      if (result.length() > 1) result += ",";
      result += "{\"name\":\"" + String(device.getName().c_str()) + 
                "\",\"address\":\"" + String(device.getAddress().toString().c_str()) + 
                "\",\"rssi\":" + String(device.getRSSI()) + "}";
    }
  }
  
  result += "]";
  Serial.printf("[BLE] Gefunden: %s\n", result.c_str());
  return result;
}

String MouseHandler::scanBluetoothMice() {
  // Placeholder - Bluetooth Classic Scan
  // Benötigt esp_bt_gap.h für Classic Bluetooth
  String result = "[";
  result += "{\"name\":\"BT Classic Scan\",\"info\":\"Noch nicht implementiert\"}";
  result += "]";
  return result;
}

String MouseHandler::scanUSBMice() {
  // Placeholder - USB-HID Scan
  String result = "[";
  result += "{\"name\":\"USB Scan\",\"info\":\"Noch nicht implementiert\"}";
  result += "]";
  return result;
}

// ========== Private Methoden ==========

void MouseHandler::updateBLE() {
  // Wenn nicht verbunden, versuche zu verbinden
  if (!pClient || !pClient->isConnected()) {
    mouseConnected = false;
    
    // Auto-Connect zum ersten gefundenen HID-Gerät
    NimBLEScan* pBLEScan = NimBLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    NimBLEScanResults results = pBLEScan->start(1, false);
    
    for (int i = 0; i < results.getCount(); i++) {
      NimBLEAdvertisedDevice device = results.getDevice(i);
      
      if (device.isAdvertisingService(serviceUUID)) {
        Serial.printf("[BLE] Verbinde mit: %s\n", device.getName().c_str());
        
        pClient = NimBLEDevice::createClient();
        if (pClient->connect(&device)) {
          NimBLERemoteService* pService = pClient->getService(serviceUUID);
          if (pService) {
            pMouseCharacteristic = pService->getCharacteristic(charUUID);
            if (pMouseCharacteristic && pMouseCharacteristic->canNotify()) {
              mouseConnected = true;
              Serial.println("[BLE] Maus verbunden!");
              break;
            }
          }
        }
      }
    }
    
    pBLEScan->clearResults();
    return;
  }
  
  // Maus-Daten lesen
  if (pMouseCharacteristic && pMouseCharacteristic->canRead()) {
    std::string value = pMouseCharacteristic->readValue();
    
    if (value.length() >= 4) {
      // HID Mouse Report Format: [Buttons, X-Delta, Y-Delta, Wheel]
      uint8_t buttons = value[0];
      int8_t deltaX = value[1];
      int8_t deltaY = value[2];
      
      // Buttons auswerten
      currentData.leftButton = (buttons & 0x01) != 0;
      currentData.rightButton = (buttons & 0x02) != 0;
      currentData.middleButton = (buttons & 0x04) != 0;
      
      // Position aktualisieren
      currentData.x += deltaX;
      currentData.y += deltaY;
      
      // Auf Bildschirmgrenzen beschränken
      currentData.x = constrain(currentData.x, 0, 239);
      currentData.y = constrain(currentData.y, 0, 134);
    }
  }
}

void MouseHandler::updateUSB() {
  // USB-HID wird in zukünftiger Version implementiert
  // Benötigt ESP32-S2/S3 mit nativer USB-Unterstützung
  // oder externe USB-Host-Library
}

void MouseHandler::updateClassicBT() {
  // Bluetooth Classic wird in zukünftiger Version implementiert
  // Benötigt esp_bt_gap.h und SPP/HID Profile
}

void MouseHandler::calculateSpeed() {
  unsigned long now = millis();
  unsigned long deltaTime = now - lastUpdateTime;
  
  if (deltaTime > 0) {
    // Distanz berechnen
    int deltaX = currentData.x - lastX;
    int deltaY = currentData.y - lastY;
    float distance = sqrt(deltaX * deltaX + deltaY * deltaY);
    
    // Geschwindigkeit = Distanz / Zeit
    currentData.speed = (distance / deltaTime) * 1000.0f;  // Pixel pro Sekunde
    
    // Auf Maximum begrenzen
    if (currentData.speed > MAX_MOUSE_SPEED) {
      currentData.speed = MAX_MOUSE_SPEED;
    }
  }
  
  // Position und Zeit speichern
  lastX = currentData.x;
  lastY = currentData.y;
  lastUpdateTime = now;
}
