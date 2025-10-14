# 🖱️ ESP32 LilyGo T-Display Mouse Tracker

Ein vollständiges IoT-Projekt für den ESP32 LilyGo T-Display, das USB-, Bluetooth-Classic- und BLE-Mäuse unterstützt und deren Bewegung sowie Klicks grafisch auf dem Display darstellt.

## 📋 Projektübersicht

| Datei | Beschreibung |
|-------|--------------|
| `platformio.ini` | PlatformIO-Konfiguration für Build und Dependencies |
| `src/main.cpp` | Hauptprogramm mit Setup und Loop |
| `src/display.h` | Display-Verwaltung und Grafikfunktionen |
| `src/display.cpp` | Implementierung der Display-Logik |
| `src/mouse_handler.h` | Maus-Input-Handler (USB/BT/BLE) |
| `src/mouse_handler.cpp` | Implementierung der Maus-Erkennung |
| `src/webserver.h` | Webserver und OTA-Funktionen |
| `src/webserver.cpp` | Webserver-Implementierung |
| `src/network.h` | Netzwerk-Management (AP + Station) |
| `src/network.cpp` | Netzwerk-Implementierung |
| `data/index.html` | Webinterface (wird in SPIFFS gespeichert) |
| `.github/workflows/build.yml` | GitHub Actions für automatischen Build |

## 🎯 Funktionen

### Display-Visualisierung
- **Punktbewegung**: Synchrone Bewegung mit Mauskoordinaten (5px Durchmesser)
- **Geschwindigkeitsbasierte Helligkeit**: Schnelle Bewegung = heller, langsame = dunkler
- **Klick-Animationen**:
  - Linksklick: Konzentrische Kreise
  - Rechtsklick: Ausbreitende Strahlen
  - Beide Tasten: Kombination aus Strahlen + Ringen

### Verbindungsstatus
- **Keine Maus**: Anzeige von Access Point-Zugangsdaten
  ```
  Accesspoint: ESPMAUS
  Passwort: MausMaus
  ```
- **Maus verbunden**: Status-Anzeige am unteren Bildschirmrand

### Webinterface
- **Statusanzeige**: Maus-Verbindung, Tastendruck (links/rechts/beide/keine)
- **Scan-Funktionen**:
  - 🔍 BLE-Mäuse scannen
  - 🔍 Bluetooth-Classic-Mäuse scannen
  - 🔍 USB-Mäuse scannen
  - 📶 WLAN-Netze scannen
- **OTA-Update**: Firmware-Upload (.bin) über Webinterface
- **Dual-Netzwerk**: Access Point + WLAN-Client gleichzeitig

## 🛠️ Hardware-Anforderungen

- **ESP32 LilyGo T-Display**
  - ESP32 WROVER Modul
  - 1.14" ST7789V TFT Display (135x240)
  - 2x Buttons (GPIO 0 & 35)
  - USB-C Anschluss

## 📦 Software-Anforderungen

### Für GitHub Actions Build (empfohlen)
- GitHub Account
- Keine lokale Installation erforderlich

### Für lokalen Build (optional)
- PlatformIO Core oder PlatformIO IDE
- Python 3.7+
- esptool.py

## 🚀 Schnellstart

### GitHub Codespaces Build (100% Online, keine lokale Installation)

1. **Repository öffnen**
   - Gehe zu: https://github.com/rigr/lilygomaus
   - Klicke auf: Grüner `Code` Button
   - Wähle: `Codespaces` Tab
   - Klicke: `Create codespace on main`

2. **Automatischer Build startet**
   - Codespace öffnet sich im Browser
   - Setup-Script läuft automatisch
   - PlatformIO wird installiert
   - Firmware wird kompiliert (~3-5 Minuten)

3. **Firmware herunterladen**
   - Im Terminal erscheint: `✓ Build erfolgreich!`
   - Datei wird angezeigt: `.pio/build/lilygo-t-display/firmware.bin`
   - Rechtsklick auf Datei → `Download`

4. **Alternative: GitHub Actions (wenn Codespaces nicht verfügbar)**
   - Gehe zu: `Actions` Tab
   - Klicke: `Build ESP32 Firmware` → `Run workflow`
   - Nach ~3 Minuten: Download `firmware.zip` aus Artifacts

5. **Auf ESP32 flashen**
   - Öffne: https://web.esphome.io (Web-Tool, keine Installation nötig!)
   - Oder nutze: https://espressif.github.io/esptool-js/
   - ESP32 per USB verbinden
   - `Connect` → Firmware auswählen → `Flash`

**Wichtig**: Keine lokale Installation erforderlich! Alles läuft im Browser.

## 🌐 Verwendung

### Erste Inbetriebnahme

1. **ESP32 mit Strom versorgen** (USB oder Batterie)
2. **Mit Access Point verbinden**
   - SSID: `ESPMAUS`
   - Passwort: `MausMaus`
3. **Webinterface öffnen**
   - Browser: `http://192.168.4.1`

### Maus anschließen

#### USB-Maus
1. USB-Maus über USB-OTG-Adapter an ESP32 anschließen
2. Automatische Erkennung
3. Display zeigt: "Maus verbunden"

#### Bluetooth-Maus
1. Im Webinterface: "Scan BT-Mäuse" oder "Scan BLE-Mäuse"
2. Maus auswählen aus der Liste
3. Verbindung wird automatisch hergestellt

### WLAN-Konfiguration

1. **Im Webinterface**: "Scan WLAN-Netze"
2. **Netzwerk auswählen**: SSID eingeben
3. **Passwort eingeben**
4. **Verbinden**: ESP32 bleibt als AP aktiv und verbindet sich zusätzlich mit dem WLAN

### OTA-Update

1. **Webinterface öffnen**: `http://192.168.4.1` oder `http://<ESP32-IP>`
2. **"Firmware-Upload" anklicken**
3. **.bin-Datei auswählen**: Neue Firmware-Version
4. **Upload starten**: ESP32 führt automatisch Update durch und startet neu

## 📊 Display-Darstellung

```
┌─────────────────────────┐
│   ESPMAUS verbunden     │  ← Status-Zeile
│                         │
│         ●               │  ← Mauszeiger (5px)
│                         │
│                         │
│                         │
│    🔘  ))) )))          │  ← Klick-Animationen
│                         │
│                         │
│ Maus verbunden          │  ← Verbindungsstatus
└─────────────────────────┘
```

### Farben und Effekte

- **Punkt-Helligkeit**: 
  - Langsam: RGB(100, 100, 100)
  - Mittel: RGB(150, 150, 200)
  - Schnell: RGB(255, 255, 255)

- **Linksklick-Kreise**: 
  - Farbe: Cyan
  - 3 konzentrische Ringe
  - Expansion: 5px/Frame

- **Rechtsklick-Strahlen**:
  - Farbe: Magenta
  - 8 Strahlen (45° Schritte)
  - Länge: 10-30px

## 🔧 Konfiguration

### Netzwerk-Einstellungen (src/network.h)

```cpp
#define AP_SSID "ESPMAUS"
#define AP_PASSWORD "MausMaus"
#define AP_CHANNEL 6
#define AP_MAX_CONNECTIONS 4
```

### Display-Einstellungen (src/display.h)

```cpp
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 135
#define CURSOR_SIZE 5
#define CURSOR_MIN_BRIGHTNESS 100
#define CURSOR_MAX_BRIGHTNESS 255
```

### USB/Bluetooth-Einstellungen (src/mouse_handler.h)

```cpp
#define BLE_SCAN_DURATION 5      // Sekunden
#define BT_SCAN_DURATION 5       // Sekunden
#define USB_POLL_INTERVAL 10     // Millisekunden
```

## 📐 Projektstruktur

```
lilygomaus/
├── .github/
│   └── workflows/
│       └── build.yml          # GitHub Actions Build
├── data/
│   └── index.html             # Webinterface
├── include/
├── lib/
├── src/
│   ├── main.cpp               # Hauptprogramm
│   ├── display.h/.cpp         # Display-Verwaltung
│   ├── mouse_handler.h/.cpp   # Maus-Input
│   ├── webserver.h/.cpp       # Webserver & OTA
│   └── network.h/.cpp         # Netzwerk-Management
├── platformio.ini             # Build-Konfiguration
└── README.md                  # Diese Datei
```

## 🐛 Troubleshooting

### ESP32 wird nicht erkannt
```bash
# Treiber prüfen
# Windows: CH340/CP2102 Treiber installieren
# Linux: sudo usermod -a -G dialout $USER (dann neu anmelden)
# macOS: Sollte automatisch funktionieren
```

### Upload schlägt fehl
```bash
# Boot-Modus erzwingen
# GPIO 0 gedrückt halten + Reset drücken + GPIO 0 loslassen
esptool.py --chip esp32 erase_flash
```

### Display zeigt nichts
- TFT-Kabel prüfen (bei externem Display)
- Helligkeit erhöhen (kann im Code angepasst werden)
- Power Cycle (Strom aus/an)

### Maus wird nicht erkannt
- **USB**: OTG-Adapter prüfen, USB-Host aktiviert?
- **Bluetooth**: Maus in Pairing-Modus versetzen
- **BLE**: Maus-Batterie prüfen, Reichweite < 5m

### Webinterface nicht erreichbar
- Verbindung mit AP prüfen: `ESPMAUS`
- IP-Adresse prüfen: `192.168.4.1`
- Firewall/Antivirus temporär deaktivieren

## 🔐 Sicherheitshinweise

⚠️ **Produktionsumgebung**:
- AP-Passwort ändern (mind. 8 Zeichen)
- OTA-Authentifizierung aktivieren
- HTTPS für Webinterface implementieren
- Verschlüsselung für gespeicherte WLAN-Credentials

## 📚 Abhängigkeiten

Automatisch installiert via PlatformIO:

- **TFT_eSPI**: Display-Treiber für ST7789V
- **ESP32-BLE-Mouse**: BLE-Maus-Unterstützung
- **ESP32-USB-Host**: USB-HID-Support
- **ArduinoJson**: JSON-Parsing für Webinterface
- **ESPAsyncWebServer**: Asynchroner Webserver
- **AsyncTCP**: TCP-Library für Webserver

## 🚧 Erweiterungsideen

- [ ] **Multi-Maus-Support**: Mehrere Mäuse gleichzeitig
- [ ] **Bewegungsprofil**: Aufzeichnung und Wiedergabe
- [ ] **Makros**: Programmierbare Mausaktionen
- [ ] **MQTT-Integration**: Fernsteuerung über MQTT
- [ ] **Bildschirmfoto**: Display-Screenshot über Webinterface
- [ ] **Kalibrierung**: Geschwindigkeits- und Beschleunigungsanpassung
- [ ] **Themes**: Verschiedene Farbschemata für Animationen
- [ ] **Statistiken**: Klick-Counter, zurückgelegte Distanz

## 📄 Lizenz

MIT License - Siehe LICENSE-Datei

## 👥 Beiträge

Contributions sind willkommen! Bitte:
1. Fork das Repository
2. Erstelle einen Feature-Branch (`git checkout -b feature/AmazingFeature`)
3. Commit deine Änderungen (`git commit -m 'Add some AmazingFeature'`)
4. Push zum Branch (`git push origin feature/AmazingFeature`)
5. Öffne einen Pull Request

## 📞 Support

- **Issues**: https://github.com/rigr/lilygomaus/issues
- **Diskussionen**: https://github.com/rigr/lilygomaus/discussions

## 🙏 Danksagungen

- LilyGo für das großartige T-Display Board
- Bodmer für die TFT_eSPI Library
- ESP32 Community für Support und Libraries

---

**Version**: 1.0.0  
**Autor**: rigr  
**Datum**: 2025
