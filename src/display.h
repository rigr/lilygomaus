/**
 * Display-Verwaltung für LilyGo T-Display
 * ST7789V TFT Display (135x240)
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <Arduino.h>

// Display-Dimensionen
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135

// Cursor-Eigenschaften
#define CURSOR_SIZE 5
#define CURSOR_MIN_BRIGHTNESS 100
#define CURSOR_MAX_BRIGHTNESS 255

// Animationstypen
enum ClickType {
  CLICK_NONE,
  CLICK_LEFT,
  CLICK_RIGHT,
  CLICK_BOTH
};

// Animation-Struktur
struct ClickAnimation {
  int x;
  int y;
  ClickType type;
  int frame;
  bool active;
};

class DisplayManager {
private:
  TFT_eSPI tft;
  
  // Animation-Tracking
  static const int MAX_ANIMATIONS = 5;
  ClickAnimation animations[MAX_ANIMATIONS];
  
  // Hilfsfunktionen
  void drawConcentricCircles(int x, int y, int frame);
  void drawRays(int x, int y, int frame);
  uint16_t speedToColor(float speed);

public:
  DisplayManager();
  
  bool begin();
  void clearScreen();
  
  // Boot und Info-Screens
  void showBootScreen(const char* message);
  void showConnectionInfo(const char* ssid, const char* password, const char* ip);
  void showMouseStatus(const char* status);
  void showError(const char* error);
  void showOTAProgress(int percentage);
  
  // Maus-Visualisierung
  void drawCursor(int x, int y, float speed);
  void drawClickAnimation(int x, int y, ClickType type);
  void updateAnimations();
};

#endif
