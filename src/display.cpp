Corrected display.cpp created
Length: 4905

=== COPY THIS ===

/**
 * Display-Implementierung
 */

#include "display.h"

DisplayManager::DisplayManager() : tft(TFT_eSPI()) {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    animations[i].active = false;
  }
}

bool DisplayManager::begin() {
  tft.init();
  tft.setRotation(1); // Landscape
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  return true;
}

void DisplayManager::clearScreen() {
  tft.fillScreen(TFT_BLACK);
}

void DisplayManager::showBootScreen(const char* message) {
  clearScreen();
  tft.setTextSize(2);
  tft.drawString("LILYGOMAUS", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20);
  tft.setTextSize(1);
  tft.drawString(message, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10);
}

void DisplayManager::showConnectionInfo(const char* ssid, const char* password, const char* ip) {
  clearScreen();
  tft.setTextSize(1);
  tft.setTextDatum(TL_DATUM);
  
  int y = 10;
  tft.drawString("WiFi Access Point:", 10, y); y += 20;
  tft.setTextSize(2);
  tft.drawString(ssid, 10, y); y += 25;
  
  tft.setTextSize(1);
  tft.drawString("Password:", 10, y); y += 20;
  tft.setTextSize(2);
  tft.drawString(password, 10, y); y += 25;
  
  tft.setTextSize(1);
  tft.drawString("Web Interface:", 10, y); y += 20;
  tft.drawString(ip, 10, y);
}

void DisplayManager::showMouseStatus(const char* status) {
  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  tft.fillRect(0, SCREEN_HEIGHT - 15, SCREEN_WIDTH, 15, TFT_BLACK);
  tft.drawString(status, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 5);
}

void DisplayManager::showError(const char* error) {
  clearScreen();
  tft.setTextSize(1);
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("ERROR", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 10);
  tft.drawString(error, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 10);
}

void DisplayManager::showOTAProgress(int percentage) {
  clearScreen();
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("OTA Update", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20);
  
  // Progress bar
  int barWidth = 200;
  int barHeight = 20;
  int barX = (SCREEN_WIDTH - barWidth) / 2;
  int barY = SCREEN_HEIGHT / 2 + 10;
  
  tft.drawRect(barX, barY, barWidth, barHeight, TFT_WHITE);
  int fillWidth = (barWidth - 4) * percentage / 100;
  tft.fillRect(barX + 2, barY + 2, fillWidth, barHeight - 4, TFT_GREEN);
  
  tft.setTextSize(1);
  char percentStr[10];
  sprintf(percentStr, "%d%%", percentage);
  tft.drawString(percentStr, SCREEN_WIDTH / 2, barY + barHeight + 15);
}

void DisplayManager::drawCursor(int x, int y, float speed) {
  // Map speed to brightness
  int brightness = map(constrain(speed, 0, 50), 0, 50, CURSOR_MIN_BRIGHTNESS, CURSOR_MAX_BRIGHTNESS);
  uint16_t color = tft.color565(brightness, brightness, brightness);
  
  // Draw cursor
  tft.fillCircle(x, y, CURSOR_SIZE, color);
}

uint16_t DisplayManager::speedToColor(float speed) {
  int brightness = map(constrain(speed, 0, 50), 0, 50, CURSOR_MIN_BRIGHTNESS, CURSOR_MAX_BRIGHTNESS);
  return tft.color565(brightness, brightness, brightness);
}

void DisplayManager::drawClickAnimation(int x, int y, ClickType type) {
  // Find free animation slot
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (!animations[i].active) {
      animations[i].x = x;
      animations[i].y = y;
      animations[i].type = type;
      animations[i].frame = 0;
      animations[i].active = true;
      break;
    }
  }
}

void DisplayManager::drawConcentricCircles(int x, int y, int frame) {
  // Draw 3 expanding circles
  for (int i = 0; i < 3; i++) {
    int radius = (frame + i * 5) * 2;
    if (radius < 50) {
      tft.drawCircle(x, y, radius, TFT_CYAN);
    }
  }
}

void DisplayManager::drawRays(int x, int y, int frame) {
  // Draw 8 rays in 45° steps
  int length = 10 + frame * 2;
  if (length > 30) length = 30;
  
  for (int angle = 0; angle < 360; angle += 45) {
    float rad = angle * PI / 180.0;
    int x2 = x + cos(rad) * length;
    int y2 = y + sin(rad) * length;
    tft.drawLine(x, y, x2, y2, TFT_MAGENTA);
  }
}

void DisplayManager::updateAnimations() {
  for (int i = 0; i < MAX_ANIMATIONS; i++) {
    if (animations[i].active) {
      animations[i].frame++;
      
      // Draw animation based on type
      switch (animations[i].type) {
        case CLICK_LEFT:
          drawConcentricCircles(animations[i].x, animations[i].y, animations[i].frame);
          break;
        case CLICK_RIGHT:
          drawRays(animations[i].x, animations[i].y, animations[i].frame);
          break;
        case CLICK_BOTH:
          drawConcentricCircles(animations[i].x, animations[i].y, animations[i].frame);
          drawRays(animations[i].x, animations[i].y, animations[i].frame);
          break;
        default:
          break;
      }
      
      // Deactivate after 15 frames
      if (animations[i].frame > 15) {
        animations[i].active = false;
      }
    }
  }
}
