/**
 * Display-Implementierung für ESP32 LilyGo T-Display
 * 
 * @file display.cpp
 * @author rigr
 */

#include "display.h"

// ========== Konstruktor ==========


DisplayManager::DisplayManager() 
  : lastCursorX(SCREEN_WIDTH / 2),
    lastCursorY(SCREEN_HEIGHT / 2) {
  // Alle Animationen als inaktiv markieren
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    animations[i].active = false;
    animations[i].frame = 0;
  }
}

// ========== Öffentliche Methoden ==========

bool DisplayManager::begin() {
  // TFT initialisieren
  tft.init();
  tft.setRotation(SCREEN_ROTATION);
  tft.fillScreen(COLOR_BACKGROUND);
  
  // Hintergrundbeleuchtung einschalten (GPIO 4)
  pinMode(TFT_BL, OUTPUT);
  setBrightness(200);  // 78% Helligkeit
  
  return true;
}

void DisplayManager::clearScreen(uint16_t color) {
  tft.fillScreen(color);
  
  // Cursor-Position zurücksetzen
  lastCursorX = SCREEN_WIDTH / 2;
  lastCursorY = SCREEN_HEIGHT / 2;
  
  // Alle Animationen stoppen
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    animations[i].active = false;
  }
}

void DisplayManager::drawCursor(int x, int y, float speed) {
  // Koordinaten auf Bildschirmgrenzen beschränken
  x = constrain(x, CURSOR_SIZE, SCREEN_WIDTH - CURSOR_SIZE);
  y = constrain(y, CURSOR_SIZE, SCREEN_HEIGHT - CURSOR_SIZE);
  
  // Alten Cursor löschen (schwarzer Kreis)
  tft.fillCircle(lastCursorX, lastCursorY, CURSOR_SIZE, COLOR_BACKGROUND);
  
  // Helligkeit basierend auf Geschwindigkeit berechnen
  uint8_t brightness = calculateBrightness(speed);
  
  // RGB-Farbe mit variabler Helligkeit
  uint16_t cursorColor = tft.color565(brightness, brightness, brightness);
  
  // Neuen Cursor zeichnen
  tft.fillCircle(x, y, CURSOR_SIZE, cursorColor);
  
  // Optional: Outline für bessere Sichtbarkeit
  if (brightness > 200) {
    tft.drawCircle(x, y, CURSOR_SIZE + 1, COLOR_CLICK_LEFT);
  }
  
  // Position speichern
  lastCursorX = x;
  lastCursorY = y;
}

void DisplayManager::drawClickAnimation(int x, int y, ClickType type) {
  // Freien Animations-Slot finden
  int slot = findFreeAnimationSlot();
  if (slot < 0) {
    // Kein freier Slot - älteste Animation überschreiben
    slot = 0;
  }
  
  // Animation initialisieren
  animations[slot].x = x;
  animations[slot].y = y;
  animations[slot].type = type;
  animations[slot].frame = 1;
  animations[slot].active = true;
}

void DisplayManager::updateAnimations() {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (!animations[i].active) continue;
    
    ClickAnimation* anim = &animations[i];
    
    // Animation basierend auf Typ zeichnen
    switch (anim->type) {
      case CLICK_LEFT:
        drawRingAnimation(anim->x, anim->y, anim->frame);
        break;
        
      case CLICK_RIGHT:
        drawRayAnimation(anim->x, anim->y, anim->frame);
        break;
        
      case CLICK_BOTH:
        // Kombination: Zuerst Ringe, dann Strahlen
        drawRingAnimation(anim->x, anim->y, anim->frame);
        drawRayAnimation(anim->x, anim->y, anim->frame);
        break;
        
      default:
        break;
    }
    
    // Frame hochzählen
    anim->frame++;
    
    // Animation beenden nach ANIMATION_DURATION Frames
    if (anim->frame > ANIMATION_DURATION) {
      anim->active = false;
      anim->frame = 0;
    }
  }
}

void DisplayManager::showConnectionInfo(const char* ssid, const char* password, const char* ip) {
  clearScreen();
  
  // Titel
  drawCenteredText("ESP32 Maus-Tracker", 20, COLOR_TEXT, 4);
  
  // Trennlinie
  tft.drawLine(20, 40, SCREEN_WIDTH - 20, 40, COLOR_TEXT_DIM);
  
  // Access Point Info
  tft.setTextColor(COLOR_STATUS_OK, COLOR_BACKGROUND);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);
  
  tft.drawString("WLAN-Accesspoint:", SCREEN_WIDTH / 2, 55, 2);
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.drawString(ssid, SCREEN_WIDTH / 2, 75, 4);
  
  tft.setTextColor(COLOR_TEXT_DIM, COLOR_BACKGROUND);
  tft.drawString("Passwort:", SCREEN_WIDTH / 2, 95, 2);
  tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
  tft.drawString(password, SCREEN_WIDTH / 2, 115, 4);
}

void DisplayManager::showMouseStatus(const char* status) {
  // Statusbereich löschen (unterste 20 Pixel)
  tft.fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20, COLOR_BACKGROUND);
  
  // Status zentriert anzeigen
  tft.setTextColor(COLOR_STATUS_OK, COLOR_BACKGROUND);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(status, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10, 2);
}

void DisplayManager::showBootScreen(const char* message) {
  clearScreen();
  
  drawCenteredText("ESP32 LilyGo", 40, COLOR_TEXT, 4);
  drawCenteredText("Maus-Tracker", 65, COLOR_TEXT, 4);
  
  // Animation: Lade-Balken
  int barWidth = 160;
  int barHeight = 8;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = 90;
  
  tft.drawRect(barX, barY, barWidth, barHeight, COLOR_TEXT_DIM);
  
  // Nachricht
  drawCenteredText(message, 110, COLOR_TEXT_DIM, 2);
}

void DisplayManager::showError(const char* errorMessage) {
  clearScreen();
  
  // Großes rotes X
  tft.drawLine(60, 30, 180, 90, COLOR_STATUS_ERROR);
  tft.drawLine(180, 30, 60, 90, COLOR_STATUS_ERROR);
  tft.drawLine(61, 30, 181, 90, COLOR_STATUS_ERROR);
  tft.drawLine(181, 30, 61, 90, COLOR_STATUS_ERROR);
  
  drawCenteredText("FEHLER", 105, COLOR_STATUS_ERROR, 4);
  drawCenteredText(errorMessage, 125, COLOR_TEXT, 2);
}

void DisplayManager::showOTAProgress(int percentage) {
  clearScreen();
  
  drawCenteredText("Firmware-Update", 30, COLOR_TEXT, 4);
  
  // Fortschrittsbalken
  int barWidth = 200;
  int barHeight = 20;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = 60;
  
  // Rahmen
  tft.drawRect(barX, barY, barWidth, barHeight, COLOR_TEXT);
  
  // Fortschritt
  int fillWidth = (barWidth - 4) * percentage / 100;
  tft.fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, COLOR_STATUS_OK);
  
  // Prozent-Anzeige
  char percentStr[8];
  sprintf(percentStr, "%d%%", percentage);
  drawCenteredText(percentStr, 95, COLOR_TEXT, 4);
  
  drawCenteredText("Bitte warten...", 115, COLOR_TEXT_DIM, 2);
}

void DisplayManager::setBrightness(uint8_t brightness) {
  analogWrite(TFT_BL, brightness);
}

// ========== Private Methoden ==========

void DisplayManager::drawRingAnimation(int centerX, int centerY, int frame) {
  // 3 konzentrische Kreise, die sich nach außen bewegen
  for (int ring = 0; ring < 3; ring++) {
    int radius = (frame + ring * 10) * RING_EXPANSION / 5;
    
    // Transparenz-Effekt durch hellere Farbe bei älteren Frames
    int alpha = 255 - (frame * 8);
    if (alpha < 0) alpha = 0;
    
    uint8_t brightness = alpha / 3;
    uint16_t color = tft.color565(0, brightness, brightness);  // Cyan
    
    // Kreis zeichnen (kein Fill, nur Outline)
    if (radius < 60) {  // Nicht über Bildschirmrand hinaus
      tft.drawCircle(centerX, centerY, radius, color);
      
      // Dickerer Ring durch mehrfaches Zeichnen
      tft.drawCircle(centerX, centerY, radius + 1, color);
    }
  }
  
  // Alte Kreise löschen (schwarzer Kreis an vorheriger Position)
  if (frame > 5) {
    int oldRadius = ((frame - 5) * RING_EXPANSION / 5);
    tft.drawCircle(centerX, centerY, oldRadius, COLOR_BACKGROUND);
    tft.drawCircle(centerX, centerY, oldRadius + 1, COLOR_BACKGROUND);
  }
}

void DisplayManager::drawRayAnimation(int centerX, int centerY, int frame) {
  // 8 Strahlen in 45°-Schritten
  for (int i = 0; i < NUM_RAYS; i++) {
    float angle = i * (2 * PI / NUM_RAYS);
    
    // Länge wächst mit Frame
    int length = RAY_LENGTH_MIN + (frame * 2);
    if (length > RAY_LENGTH_MAX) length = RAY_LENGTH_MAX;
    
    // Endpunkte berechnen
    int x2 = centerX + cos(angle) * length;
    int y2 = centerY + sin(angle) * length;
    
    // Transparenz-Effekt
    int alpha = 255 - (frame * 8);
    if (alpha < 0) alpha = 0;
    
    uint8_t brightness = alpha / 3;
    uint16_t color = tft.color565(brightness, 0, brightness);  // Magenta
    
    // Strahl zeichnen
    tft.drawLine(centerX, centerY, x2, y2, color);
    
    // Dickerer Strahl
    tft.drawLine(centerX + 1, centerY, x2 + 1, y2, color);
    tft.drawLine(centerX, centerY + 1, x2, y2 + 1, color);
  }
  
  // Alte Strahlen löschen
  if (frame > 3) {
    int oldLength = RAY_LENGTH_MIN + ((frame - 3) * 2);
    for (int i = 0; i < NUM_RAYS; i++) {
      float angle = i * (2 * PI / NUM_RAYS);
      int x2 = centerX + cos(angle) * oldLength;
      int y2 = centerY + sin(angle) * oldLength;
      tft.drawLine(centerX, centerY, x2, y2, COLOR_BACKGROUND);
      tft.drawLine(centerX + 1, centerY, x2 + 1, y2, COLOR_BACKGROUND);
      tft.drawLine(centerX, centerY + 1, x2, y2 + 1, COLOR_BACKGROUND);
    }
  }
}

uint8_t DisplayManager::calculateBrightness(float speed) {
  // Lineare Interpolation zwischen min und max Helligkeit
  int brightness = CURSOR_MIN_BRIGHTNESS + 
                   (speed / SPEED_THRESHOLD) * (CURSOR_MAX_BRIGHTNESS - CURSOR_MIN_BRIGHTNESS);
  
  // Auf gültigen Bereich beschränken
  return constrain(brightness, CURSOR_MIN_BRIGHTNESS, CURSOR_MAX_BRIGHTNESS);
}

void DisplayManager::drawCenteredText(const char* text, int y, uint16_t color, uint8_t font) {
  tft.setTextColor(color, COLOR_BACKGROUND);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, SCREEN_WIDTH / 2, y, font);
}

int DisplayManager::findFreeAnimationSlot() {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (!animations[i].active) {
      return i;
    }
  }
  return -1;  // Alle Slots belegt
}
