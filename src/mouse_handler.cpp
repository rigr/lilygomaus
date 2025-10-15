/**
 * Mouse Handler Implementierung
 * Unterstützt USB, Bluetooth Classic und BLE-Mäuse
 * 
 * Priorität: 1. BT Classic, 2. USB, 3. BLE
 */

#include "mouse_handler.h"
#include <math.h>

// ========== Globale Variablen für Callbacks ==========

static MouseHandler* g_mouseHandlerInstance = nullptr;
static void (*g_bleDeviceCallback)(BLEMouseDevice) = nullptr;
static void (*g_btClassicDeviceCallback)(BTClassicDevice) = nullptr;
static void (*g_usbDeviceCallback)(USBMouseDevice) = nullptr;

// ========== Konstruktor ==========

MouseHandler::MouseHandler() {
  bleClient = nullptr;
  bleMouseCharacteristic = nullptr;
  bleConnected = false;
  
  btClassicInitialized = false;
  btClassicConnected = false;
  memset(btClassicAddress, 0, sizeof(btClassicAddress));
  
  usbConnected = false;
  usbHID = nullptr;
  
  currentMouseType = MOUSE_NONE;
  
  currentData.x = 120; // Display-Mitte
  currentData.y = 67;
  currentData.leftButton = false;
  currentData.rightButton = false;
  currentData.speed = 0.0f;
  currentData.type = MOUSE_NONE;
  
  lastX = currentData.x;
  lastY = currentData.y;
  lastUpdateTime = millis();
  
  g_mouseHandlerInstance = this;
}

// ========== Initialisierung ==========

bool MouseHandler::begin() {
  Serial.println("[MouseHandler] Initialisiere Maus-Handler...");
  
  // Vorbereitung aller Systeme (werden bei Bedarf aktiviert)
  Serial.println("[MouseHandler] Bereit für BT-Classic, USB und BLE-Mäuse");
  
  return true;
}

// ========== Update-Loop ==========

void MouseHandler::update() {
  // Geschwindigkeit berechnen
  unsigned long now = millis();
  if (now - lastUpdateTime > 100) {
    currentData.speed = calculateSpeed();
    lastX = currentData.x;
    lastY = currentData.y;
    lastUpdateTime = now;
  }
  
  // USB-Polling (falls USB-Maus verbunden)
  // TODO: Wird implementiert wenn USB-Support aktiv ist
}

// ========== Status-Funktionen ==========

bool MouseHandler::isMouseConnected() {
  return currentMouseType != MOUSE_NONE;
}

MouseData MouseHandler::getMouseData() {
  currentData.type = currentMouseType;
  return currentData;
}

MouseType MouseHandler::getMouseType() {
  return currentMouseType;
}

// ========== Disconnect ==========

void MouseHandler::disconnectMouse() {
  Serial.println("[MouseHandler] Trenne Maus...");
  
  switch(currentMouseType) {
    case MOUSE_BLE:
      disconnectBLE();
      break;
    case MOUSE_BT_CLASSIC:
      disconnectBTClassic();
      break;
    case MOUSE_USB:
      disconnectUSB();
      break;
    default:
      break;
  }
  
  currentMouseType = MOUSE_NONE;
  Serial.println("[MouseHandler] Maus getrennt");
}

// ========== Hilfsfunktionen ==========

float MouseHandler::calculateSpeed() {
  unsigned long now = millis();
  unsigned long deltaTime = now - lastUpdateTime;
  
  if (deltaTime == 0) return currentData.speed;
  
  int dx = currentData.x - lastX;
  int dy = currentData.y - lastY;
  float distance = sqrt(dx * dx + dy * dy);
  
  // Pixel pro Sekunde
  float speed = distance / (deltaTime / 1000.0f);
  
  // Sanfte Geschwindigkeitsänderung (Low-Pass-Filter)
  return currentData.speed * 0.7f + speed * 0.3f;
}

void MouseHandler::updateMousePosition(int8_t dx, int8_t dy) {
  currentData.x += dx;
  currentData.y += dy;
  
  // Display-Grenzen (T-Display: 240x135)
  currentData.x = constrain(currentData.x, 0, 240);
  currentData.y = constrain(currentData.y, 0, 135);
}

void MouseHandler::updateMouseButtons(bool left, bool right) {
  // Nur bei Änderung ausgeben
  if (left != currentData.leftButton || right != currentData.rightButton) {
    Serial.printf("[MouseHandler] Buttons: L=%d R=%d\n", left, right);
  }
  
  currentData.leftButton = left;
  currentData.rightButton = right;
}


// ============================================================================
// BLUETOOTH CLASSIC - PRIORITÄT 1
// ============================================================================

#ifdef CONFIG_BT_ENABLED

bool MouseHandler::initBTClassic() {
  if (btClassicInitialized) {
    Serial.println("[BT-Classic] Bereits initialisiert");
    return true;
  }
  
  Serial.println("[BT-Classic] Initialisiere Bluetooth Classic...");
  
  // BLE-Speicher freigeben (BT-Classic und BLE können nicht gleichzeitig aktiv sein!)
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));
  
  // Bluetooth Controller konfigurieren
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t ret = esp_bt_controller_init(&bt_cfg);
  if (ret != ESP_OK) {
    Serial.printf("[BT-Classic] Controller init fehlgeschlagen: %s\n", esp_err_to_name(ret));
    return false;
  }
  
  // Bluetooth Controller aktivieren (nur Classic)
  ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
  if (ret != ESP_OK) {
    Serial.printf("[BT-Classic] Controller enable fehlgeschlagen: %s\n", esp_err_to_name(ret));
    return false;
  }
  
  // Bluedroid Stack initialisieren
  ret = esp_bluedroid_init();
  if (ret != ESP_OK) {
    Serial.printf("[BT-Classic] Bluedroid init fehlgeschlagen: %s\n", esp_err_to_name(ret));
    return false;
  }
  
  ret = esp_bluedroid_enable();
  if (ret != ESP_OK) {
    Serial.printf("[BT-Classic] Bluedroid enable fehlgeschlagen: %s\n", esp_err_to_name(ret));
    return false;
  }
  
  // GAP Callback registrieren
  esp_bt_gap_register_callback(btClassicGapCallback);
  
  // Device als connectable und discoverable setzen
  esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
  
  // HID Host Profile initialisieren
  esp_hidh_config_t config = {
    .callback = btClassicHIDCallback,
    .event_stack_size = 4096,
    .callback_arg = this,
  };
  
  ret = esp_hidh_init(&config);
  if (ret != ESP_OK) {
    Serial.printf("[BT-Classic] HID Host init fehlgeschlagen: %s\n", esp_err_to_name(ret));
    return false;
  }
  
  btClassicInitialized = true;
  Serial.println("[BT-Classic] ✓ Initialisierung erfolgreich");
  
  return true;
}

void MouseHandler::scanBTClassicMice(void (*callback)(BTClassicDevice device)) {
  if (!btClassicInitialized) {
    if (!initBTClassic()) {
      Serial.println("[BT-Classic] Kann nicht scannen - Initialisierung fehlgeschlagen");
      return;
    }
  }
  
  Serial.println("[BT-Classic] Starte Scan (10 Sekunden)...");
  g_btClassicDeviceCallback = callback;
  
  // Discovery starten (10 Sekunden)
  esp_err_t ret = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
  if (ret != ESP_OK) {
    Serial.printf("[BT-Classic] Scan-Start fehlgeschlagen: %s\n", esp_err_to_name(ret));
  }
}

void MouseHandler::btClassicGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param) {
  switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT: {
      // Device gefunden
      esp_bt_gap_dev_prop_t* prop = param->disc_res.prop;
      
      // Prüfe Class of Device (HID-Devices haben 0x2580 oder 0x0580)
      uint32_t cod = 0;
      bool isHID = false;
      String deviceName = "";
      
      for (int i = 0; i < param->disc_res.num_prop; i++) {
        if (prop[i].type == ESP_BT_GAP_DEV_PROP_COD) {
          cod = *(uint32_t*)prop[i].val;
          // Major Device Class: Peripheral (0x05), Minor: Pointing device (0x02)
          // CoD Format: 0xMMmmmm (MM=Major, mmmm=Minor+Service)
          if ((cod & 0x1F00) == 0x0500) { // Major: Peripheral
            if ((cod & 0x00C0) == 0x0080) { // Minor: Pointing device (Maus)
              isHID = true;
            }
          }
        }
        else if (prop[i].type == ESP_BT_GAP_DEV_PROP_BDNAME) {
          deviceName = String((char*)prop[i].val);
        }
      }
      
      if (isHID && g_btClassicDeviceCallback != nullptr) {
        BTClassicDevice device;
        device.name = deviceName;
        
        // BT-Adresse als String
        char addr_str[18];
        sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X",
                param->disc_res.bda[0], param->disc_res.bda[1],
                param->disc_res.bda[2], param->disc_res.bda[3],
                param->disc_res.bda[4], param->disc_res.bda[5]);
        device.address = String(addr_str);
        device.rssi = param->disc_res.rssi[0];
        
        Serial.printf("[BT-Classic] HID-Device gefunden: %s (%s) RSSI: %d\n",
                     device.name.c_str(), device.address.c_str(), device.rssi);
        
        g_btClassicDeviceCallback(device);
      }
      break;
    }
    
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: {
      if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
        Serial.println("[BT-Classic] Scan abgeschlossen");
      }
      break;
    }
    
    default:
      break;
  }
}

bool MouseHandler::connectBTClassic(const char* address) {
  if (!btClassicInitialized) {
    if (!initBTClassic()) {
      return false;
    }
  }
  
  Serial.printf("[BT-Classic] Verbinde mit %s...\n", address);
  
  // String-Adresse zu esp_bd_addr_t konvertieren
  sscanf(address, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &btClassicAddress[0], &btClassicAddress[1], &btClassicAddress[2],
         &btClassicAddress[3], &btClassicAddress[4], &btClassicAddress[5]);
  
  // HID Host verbindet sich automatisch bei esp_hidh_dev_open
  esp_hidh_dev_t* dev = esp_hidh_dev_open(btClassicAddress, ESP_HID_TRANSPORT_BT, 0);
  
  if (dev == nullptr) {
    Serial.println("[BT-Classic] Verbindung fehlgeschlagen");
    return false;
  }
  
  btClassicConnected = true;
  currentMouseType = MOUSE_BT_CLASSIC;
  
  Serial.println("[BT-Classic] ✓ Verbunden!");
  return true;
}

bool MouseHandler::connectBTClassicMouse(const char* address) {
  return connectBTClassic(address);
}

void MouseHandler::disconnectBTClassic() {
  if (!btClassicConnected) return;
  
  Serial.println("[BT-Classic] Trenne Verbindung...");
  
  // TODO: esp_hidh_dev_close() mit Device-Handle
  // (Device-Handle müsste gespeichert werden)
  
  btClassicConnected = false;
  currentMouseType = MOUSE_NONE;
}

void MouseHandler::btClassicHIDCallback(void* handler_args, esp_event_base_t base,
                                       int32_t id, void* event_data) {
  esp_hidh_event_t event = (esp_hidh_event_t)id;
  esp_hidh_event_data_t* param = (esp_hidh_event_data_t*)event_data;
  
  MouseHandler* handler = (MouseHandler*)handler_args;
  
  switch (event) {
    case ESP_HIDH_OPEN_EVENT: {
      Serial.println("[BT-Classic HID] Verbindung geöffnet");
      break;
    }
    
    case ESP_HIDH_INPUT_EVENT: {
      // HID Mouse Report Format (Standard):
      // Byte 0: Buttons (Bit 0=Left, Bit 1=Right, Bit 2=Middle, ...)
      // Byte 1: X-Movement (int8_t)
      // Byte 2: Y-Movement (int8_t)
      // Byte 3: Wheel (int8_t, optional)
      
      if (param->input.length >= 3) {
        uint8_t* data = param->input.data;
        
        bool leftButton = data[0] & 0x01;
        bool rightButton = data[0] & 0x02;
        int8_t dx = (int8_t)data[1];
        int8_t dy = (int8_t)data[2];
        
        if (handler) {
          handler->updateMousePosition(dx, dy);
          handler->updateMouseButtons(leftButton, rightButton);
        }
      }
      break;
    }
    
    case ESP_HIDH_CLOSE_EVENT: {
      Serial.println("[BT-Classic HID] Verbindung geschlossen");
      if (handler) {
        handler->btClassicConnected = false;
        handler->currentMouseType = MOUSE_NONE;
      }
      break;
    }
    
    default:
      break;
  }
}

void MouseHandler::processBTClassicData(uint8_t* data, size_t length) {
  // Diese Funktion wird vom HID-Callback aufgerufen
  if (length < 3) return;
  
  bool leftButton = data[0] & 0x01;
  bool rightButton = data[0] & 0x02;
  int8_t dx = (int8_t)data[1];
  int8_t dy = (int8_t)data[2];
  
  updateMousePosition(dx, dy);
  updateMouseButtons(leftButton, rightButton);
}

#else
// Bluetooth nicht aktiviert
bool MouseHandler::initBTClassic() {
  Serial.println("[BT-Classic] Bluetooth nicht in SDK aktiviert!");
  return false;
}

void MouseHandler::scanBTClassicMice(void (*callback)(BTClassicDevice device)) {
  Serial.println("[BT-Classic] Bluetooth nicht verfügbar");
}

bool MouseHandler::connectBTClassic(const char* address) {
  Serial.println("[BT-Classic] Bluetooth nicht verfügbar");
  return false;
}

bool MouseHandler::connectBTClassicMouse(const char* address) {
  return false;
}

void MouseHandler::disconnectBTClassic() {}
void MouseHandler::btClassicGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t* param) {}
void MouseHandler::btClassicHIDCallback(void* handler_args, esp_event_base_t base, int32_t id, void* event_data) {}
void MouseHandler::processBTClassicData(uint8_t* data, size_t length) {}
#endif


// ============================================================================
// USB - PRIORITÄT 2
// ============================================================================

bool MouseHandler::initUSB() {
  Serial.println("[USB] USB-Host-Unterstützung noch nicht implementiert");
  Serial.println("[USB] TODO: TinyUSB-Host-Integration für USB-Mäuse");
  
  // TODO: USB-Host initialisieren
  // #include "tinyusb.h"
  // tuh_init(BOARD_TUD_RHPORT);
  
  return false;
}

bool MouseHandler::connectUSBMouse() {
  Serial.println("[USB] USB-Maus-Unterstützung in Entwicklung");
  return false;
}

void MouseHandler::scanUSBMice(void (*callback)(USBMouseDevice device)) {
  Serial.println("[USB] USB-Scan noch nicht implementiert");
}

void MouseHandler::disconnectUSB() {
  if (usbConnected) {
    Serial.println("[USB] Trenne USB-Maus...");
    usbConnected = false;
  }
}

void MouseHandler::processUSBMouseReport(uint8_t* data, size_t length) {
  // TODO: USB HID Report verarbeiten
}

void MouseHandler::usbEventCallback(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data) {
  // TODO: USB-Event-Handler
}


// ============================================================================
// BLE - PRIORITÄT 3
// ============================================================================

bool MouseHandler::connectBLE(const char* address) {
  Serial.println("[BLE] BLE-Maus-Unterstützung noch nicht implementiert");
  Serial.println("[BLE] TODO: NimBLE HID-over-GATT Implementation");
  
  // TODO: BLE-Verbindung aufbauen
  // NimBLEDevice::init("");
  // bleClient = NimBLEDevice::createClient();
  // bleClient->connect(NimBLEAddress(address));
  
  return false;
}

bool MouseHandler::connectBLEMouse(const char* address) {
  return connectBLE(address);
}

void MouseHandler::scanBLEMice(void (*callback)(BLEMouseDevice device)) {
  Serial.println("[BLE] BLE-Scan noch nicht implementiert");
}

void MouseHandler::disconnectBLE() {
  if (bleConnected && bleClient) {
    Serial.println("[BLE] Trenne BLE-Maus...");
    bleClient->disconnect();
    bleConnected = false;
  }
}

void MouseHandler::processBLEMouseReport(uint8_t* data, size_t length) {
  // TODO: BLE HID Report verarbeiten
}

void MouseHandler::notifyCallback(NimBLERemoteCharacteristic* pChar,
                                  uint8_t* pData, size_t length, bool isNotify) {
  // TODO: BLE-Notification-Handler
  if (g_mouseHandlerInstance) {
    g_mouseHandlerInstance->processBLEMouseReport(pData, length);
  }
}
