/**
 * ESP32 LilyGo T-Display Mouse Tracker
 * 
 * Hauptprogramm - Koordiniert alle Module:
 * - Display-Verwaltung und Grafikdarstellung
 * - Maus-Input-Verarbeitung (USB/BT/BLE)
 * - Webserver und OTA-Updates
 * - Netzwerk-Management (AP + Station)
 * 
 * @file main.cpp
 * @author rigr
 * @version 1.0.0
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "display.h"
#include "mouse_handler.h"
#include "webserver.h"
#include "network.h"

// ========== Globale Variablen ==========

DisplayManager displayManager;
MouseHandler mouseHandler;
WebServerManager webServer;
NetworkManager networkManager;

// Timing für verschiedene Tasks
unsigned long lastDisplayUpdate = 0;
unsigned long lastMousePoll = 0;
unsigned long lastNetworkCheck = 0;

const unsigned long DISPLAY_UPDATE_INTERVAL = 16;  // ~60 FPS
const unsigned long MOUSE_POLL_INTERVAL = 10;      // 100 Hz Maus-Polling
const unsigned long NETWORK_CHECK_INTERVAL = 5000; // 5 Sekunden

// ========== Setup-Funktion ==========

void setup() {
  // Serielle Kommunikation initialisieren
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║  ESP32 LilyGo T-Display Mouse Tracker ║");
  Serial.println("║         Version 1.0.0                  ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();

  // 1. Display initialisieren (erstes visuelles Feedback)
  Serial.println("[SETUP] Initialisiere Display...");
  if (displayManager.begin()) {
    Serial.println("[OK] Display bereit");
    displayManager.showBootScreen("Starte System...");
  } else {
    Serial.println("[ERROR] Display-Initialisierung fehlgeschlagen!");
  }
  delay(500);

  // 2. Netzwerk starten (Access Point + optional Station)
  Serial.println("[SETUP] Initialisiere Netzwerk...");
  displayManager.showBootScreen("Starte WLAN...");
  if (networkManager.begin()) {
    Serial.println("[OK] Netzwerk bereit");
    Serial.printf("[INFO] Access Point: %s\n", networkManager.getAPSSID());
    Serial.printf("[INFO] AP-IP: %s\n", networkManager.getAPIP().toString().c_str());
  } else {
    Serial.println("[ERROR] Netzwerk-Initialisierung fehlgeschlagen!");
  }
  delay(500);

  // 3. Webserver starten
  Serial.println("[SETUP] Initialisiere Webserver...");
  displayManager.showBootScreen("Starte Webserver...");
  if (webServer.begin(&mouseHandler, &networkManager)) {
    Serial.println("[OK] Webserver bereit");
    Serial.printf("[INFO] Webinterface: http://%s\n", 
                  networkManager.getAPIP().toString().c_str());
  } else {
    Serial.println("[ERROR] Webserver-Initialisierung fehlgeschlagen!");
  }
  delay(500);

  // 4. Maus-Handler initialisieren
  Serial.println("[SETUP] Initialisiere Maus-Handler...");
  displayManager.showBootScreen("Suche Maus...");
  if (mouseHandler.begin()) {
    Serial.println("[OK] Maus-Handler bereit");
  } else {
    Serial.println("[ERROR] Maus-Handler-Initialisierung fehlgeschlagen!");
  }
  delay(500);

  // 5. Setup abgeschlossen - Hauptbildschirm anzeigen
  Serial.println("\n[SETUP] ✓ Initialisierung abgeschlossen\n");
  Serial.println("════════════════════════════════════════");
  Serial.println("System bereit!");
  Serial.println("════════════════════════════════════════");
  Serial.printf("Verbinde mit: %s\n", networkManager.getAPSSID());
  Serial.printf("Passwort:     %s\n", networkManager.getAPPassword());
  Serial.printf("Webinterface: http://%s\n", networkManager.getAPIP().toString().c_str());
  Serial.println("════════════════════════════════════════\n");

  displayManager.clearScreen();
  displayManager.showConnectionInfo(
    networkManager.getAPSSID(),
    networkManager.getAPPassword(),
    networkManager.getAPIP().toString().c_str()
  );

  // Initialen Status aktualisieren
  lastDisplayUpdate = millis();
  lastMousePoll = millis();
  lastNetworkCheck = millis();
}

// ========== Loop-Funktion ==========

void loop() {
  unsigned long currentMillis = millis();

  // ========== Maus-Polling ==========
  // Hohe Frequenz (100 Hz) für flüssige Mausbewegung
  if (currentMillis - lastMousePoll >= MOUSE_POLL_INTERVAL) {
    lastMousePoll = currentMillis;
    
    mouseHandler.update();
    
    // Bei Statusänderung (Maus verbunden/getrennt) Display aktualisieren
    static bool lastMouseConnected = false;
    bool currentMouseConnected = mouseHandler.isMouseConnected();
    
    if (currentMouseConnected != lastMouseConnected) {
      lastMouseConnected = currentMouseConnected;
      
      if (currentMouseConnected) {
        Serial.println("[INFO] Maus verbunden!");
        displayManager.clearScreen();
      } else {
        Serial.println("[INFO] Maus getrennt!");
        displayManager.clearScreen();
        displayManager.showConnectionInfo(
          networkManager.getAPSSID(),
          networkManager.getAPPassword(),
          networkManager.getAPIP().toString().c_str()
        );
      }
    }
  }

  // ========== Display-Update ==========
  // 60 FPS für flüssige Animationen
  if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = currentMillis;
    
    if (mouseHandler.isMouseConnected()) {
      // Maus-Daten abrufen
      MouseData mouseData = mouseHandler.getMouseData();
      
      // Cursor zeichnen (mit geschwindigkeitsbasierter Helligkeit)
      displayManager.drawCursor(
        mouseData.x, 
        mouseData.y, 
        mouseData.speed
      );
      
      // Klick-Animationen zeichnen
      if (mouseData.leftButton && mouseData.rightButton) {
        // Beide Tasten: Kombination
        displayManager.drawClickAnimation(
          mouseData.x, 
          mouseData.y, 
          CLICK_BOTH
        );
      } else if (mouseData.leftButton) {
        // Linksklick: Konzentrische Kreise
        displayManager.drawClickAnimation(
          mouseData.x, 
          mouseData.y, 
          CLICK_LEFT
        );
      } else if (mouseData.rightButton) {
        // Rechtsklick: Strahlen
        displayManager.drawClickAnimation(
          mouseData.x, 
          mouseData.y, 
          CLICK_RIGHT
        );
      }
      
      // Status am unteren Rand
      displayManager.showMouseStatus("Maus verbunden");
      
      // Debug-Ausgabe bei Klicks
      static bool lastLeftButton = false;
      static bool lastRightButton = false;
      
      if (mouseData.leftButton && !lastLeftButton) {
        Serial.printf("[MOUSE] Linksklick bei (%d, %d)\n", 
                     mouseData.x, mouseData.y);
      }
      if (mouseData.rightButton && !lastRightButton) {
        Serial.printf("[MOUSE] Rechtsklick bei (%d, %d)\n", 
                     mouseData.x, mouseData.y);
      }
      
      lastLeftButton = mouseData.leftButton;
      lastRightButton = mouseData.rightButton;
    }
    
    // Animationen updaten (Kreise/Strahlen ausblenden)
    displayManager.updateAnimations();
  }

  // ========== Netzwerk-Check ==========
  // Niedrige Frequenz (alle 5 Sekunden)
  if (currentMillis - lastNetworkCheck >= NETWORK_CHECK_INTERVAL) {
    lastNetworkCheck = currentMillis;
    
    // Netzwerkstatus prüfen
    networkManager.update();
    
    // Optional: Reconnect-Logik für Station Mode
    if (networkManager.isStationEnabled() && !networkManager.isStationConnected()) {
      static int reconnectAttempts = 0;
      reconnectAttempts++;
      
      if (reconnectAttempts <= 3) {
        Serial.printf("[WIFI] Reconnect-Versuch %d/3...\n", reconnectAttempts);
        networkManager.reconnectStation();
      } else {
        // Nach 3 Versuchen: Aufgeben bis zum nächsten manuellen Versuch
        reconnectAttempts = 0;
      }
    }
  }

  // ========== Webserver-Tasks ==========
  // Asynchroner Webserver läuft im Hintergrund
  // Keine explizite Verarbeitung nötig
  
  // Kleine Verzögerung für Watchdog
  delay(1);
}

// ========== Hilfs-Funktionen ==========

/**
 * Wird bei kritischen Fehlern aufgerufen
 * Zeigt Fehler auf Display und Serial an
 */
void handleCriticalError(const char* errorMessage) {
  Serial.printf("[CRITICAL ERROR] %s\n", errorMessage);
  displayManager.showError(errorMessage);
  
  // System anhalten
  while (true) {
    delay(1000);
  }
}

/**
 * Wird bei OTA-Start aufgerufen
 * Zeigt Fortschritt auf Display
 */
void onOTAStart() {
  Serial.println("[OTA] Update gestartet");
  displayManager.showOTAProgress(0);
}

/**
 * Wird während OTA-Update aufgerufen
 */
void onOTAProgress(unsigned int progress, unsigned int total) {
  int percentage = (progress * 100) / total;
  Serial.printf("[OTA] Fortschritt: %d%%\n", percentage);
  displayManager.showOTAProgress(percentage);
}

/**
 * Wird bei OTA-Ende aufgerufen
 */
void onOTAEnd() {
  Serial.println("[OTA] Update abgeschlossen, Neustart...");
  displayManager.showBootScreen("Neustart...");
}

/**
 * Wird bei OTA-Fehler aufgerufen
 */
void onOTAError(ota_error_t error) {
  const char* errorMsg = "OTA-Fehler";
  
  switch (error) {
    case OTA_AUTH_ERROR:
      errorMsg = "OTA: Auth fehlgeschlagen";
      break;
    case OTA_BEGIN_ERROR:
      errorMsg = "OTA: Begin fehlgeschlagen";
      break;
    case OTA_CONNECT_ERROR:
      errorMsg = "OTA: Connect fehlgeschlagen";
      break;
    case OTA_RECEIVE_ERROR:
      errorMsg = "OTA: Receive fehlgeschlagen";
      break;
    case OTA_END_ERROR:
      errorMsg = "OTA: End fehlgeschlagen";
      break;
  }
  
  Serial.printf("[OTA ERROR] %s\n", errorMsg);
  displayManager.showError(errorMsg);
}
