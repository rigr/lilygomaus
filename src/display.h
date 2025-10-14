/**
 * Display-Verwaltung für ESP32 LilyGo T-Display
 * 
 * Funktionen:
 * - TFT-Initialisierung (ST7789V, 135x240)
 * - Cursor-Rendering mit geschwindigkeitsbasierter Helligkeit
 * - Klick-Animationen (Kreise für Linksklick, Strahlen für Rechtsklick)
 * - Status-Anzeigen (Verbindung, Boot, Fehler)
 * - Doppelpufferung für flüssige Darstellung
 * 
 * @file display.h
 * @author rigr
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// ========== Display-Konstanten ==========

// Hardware-Spezifikationen LilyGo T-Display
#define SCREEN_WIDTH        240
#define SCREEN_HEIGHT       135
#define SCREEN_ROTATION     1       // Landscape-Modus

// Cursor-Einstellungen
#define CURSOR_SIZE         5       // Durchmesser in Pixel
#define CURSOR_MIN_BRIGHTNESS 100   // Minimale Helligkeit bei langsamer Bewegung
#define CURSOR_MAX_BRIGHTNESS 255   // Maximale Helligkeit bei schneller Bewegung
#define SPEED_THRESHOLD     10      // Pixel/Frame für maximale Helligkeit

// Animations-Einstellungen
#define MAX_ANIMATIONS      3       // Max. gleichzeitige Animationen
#define ANIMATION_DURATION  30      // Frames pro Animation
#define RING_EXPANSION      5       // Pixel pro Frame (Kreise)
#define RAY_LENGTH_MIN      10      // Minimale Strahlenlänge
#define RAY_LENGTH_MAX      30      // Maximale Strahlenlänge
#define NUM_RAYS            8       // Anzahl Strahlen bei Rechtsklick

// Farben (RGB565)
#define COLOR_BACKGROUND    TFT_BLACK
#define COLOR_CURSOR        TFT_WHITE
#define COLOR_TEXT          TFT_WHITE
#define COLOR_TEXT_DIM      TFT_DARKGREY
#define COLOR_CLICK_LEFT    TFT_CYAN      // Linksklick-Kreise
#define COLOR_CLICK_RIGHT   TFT_MAGENTA   // Rechtsklick-Strahlen
#define COLOR_CLICK_BOTH    TFT_YELLOW    // Beide Tasten
#define COLOR_STATUS_OK     TFT_GREEN
#define COLOR_STATUS_ERROR  TFT_RED

// ========== Enumerationen ==========

/**
 * Typ des Mausklicks für Animationen
 */
enum ClickType {
  CLICK_NONE = 0,
  CLICK_LEFT = 1,
  CLICK_RIGHT = 2,
  CLICK_BOTH = 3
};

// ========== Strukturen ==========

/**
 * Animation für Mausklicks
 * Speichert Position, Typ und Frame-Counter
 */
struct ClickAnimation {
  int x;                  // X-Position
  int y;                  // Y-Position
  ClickType type;         // Art des Klicks
  int frame;              // Aktueller Frame (0 = inaktiv)
  bool active;            // Ist die Animation aktiv?
};

// ========== Klasse: DisplayManager ==========

/**
 * Verwaltet das TFT-Display und alle grafischen Ausgaben
 */
class DisplayManager {
public:
  /**
   * Konstruktor
   */
  DisplayManager();

  /**
   * Initialisiert das Display
   * @return true bei Erfolg
   */
  bool begin();

  /**
   * Löscht den gesamten Bildschirm
   * @param color Hintergrundfarbe (optional)
   */
  void clearScreen(uint16_t color = COLOR_BACKGROUND);

  /**
   * Zeichnet den Maus-Cursor
   * @param x X-Position (0-239)
   * @param y Y-Position (0-134)
   * @param speed Geschwindigkeit für Helligkeitsberechnung
   */
  void drawCursor(int x, int y, float speed);

  /**
   * Startet eine Klick-Animation
   * @param x X-Position
   * @param y Y-Position
   * @param type Art des Klicks
   */
  void drawClickAnimation(int x, int y, ClickType type);

  /**
   * Aktualisiert alle laufenden Animationen
   * Muss in jedem Frame aufgerufen werden
   */
  void updateAnimations();

  /**
   * Zeigt Verbindungsinformationen an (wenn keine Maus verbunden)
   * @param ssid Access Point SSID
   * @param password Access Point Passwort
   * @param ip IP-Adresse
   */
  void showConnectionInfo(const char* ssid, const char* password, const char* ip);

  /**
   * Zeigt Maus-Status am unteren Bildschirmrand
   * @param status Status-Text (z.B. "Maus verbunden")
   */
  void showMouseStatus(const char* status);

  /**
   * Zeigt Boot-Screen mit Nachricht
   * @param message Boot-Nachricht
   */
  void showBootScreen(const char* message);

  /**
   * Zeigt Fehler-Screen
   * @param errorMessage Fehlermeldung
   */
  void showError(const char* errorMessage);

  /**
   * Zeigt OTA-Update-Fortschritt
   * @param percentage Fortschritt in Prozent (0-100)
   */
  void showOTAProgress(int percentage);

  /**
   * Setzt Display-Helligkeit
   * @param brightness Helligkeit (0-255)
   */
  void setBrightness(uint8_t brightness);

private:
  TFT_eSPI tft;                                 // TFT-Display-Objekt
  ClickAnimation animations[MAX_ANIMATIONS];    // Animation-Buffer
  
  // Letzter Cursor-Position für Trail-Effekt
  int lastCursorX;
  int lastCursorY;

  /**
   * Zeichnet einen animierten Kreis (Linksklick)
   * @param centerX Zentrum X
   * @param centerY Zentrum Y
   * @param frame Frame-Nummer für Größe/Transparenz
   */
  void drawRingAnimation(int centerX, int centerY, int frame);

  /**
   * Zeichnet animierte Strahlen (Rechtsklick)
   * @param centerX Zentrum X
   * @param centerY Zentrum Y
   * @param frame Frame-Nummer für Länge/Transparenz
   */
  void drawRayAnimation(int centerX, int centerY, int frame);

  /**
   * Berechnet Helligkeit basierend auf Geschwindigkeit
   * @param speed Bewegungsgeschwindigkeit
   * @return Helligkeit (0-255)
   */
  uint8_t calculateBrightness(float speed);

  /**
   * Zeichnet zentrierten Text
   * @param text Text
   * @param y Y-Position
   * @param color Textfarbe
   * @param font Font-Nummer (2, 4, 6, 7)
   */
  void drawCenteredText(const char* text, int y, uint16_t color, uint8_t font = 4);

  /**
   * Findet freien Animations-Slot
   * @return Index oder -1 wenn voll
   */
  int findFreeAnimationSlot();
};

#endif // DISPLAY_H
