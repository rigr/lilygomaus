#!/bin/bash

# ESP32 Development Environment Setup für GitHub Codespaces
# Automatische Einrichtung von PlatformIO und Build

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║  ESP32 LilyGo T-Display - Development Setup          ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Python und pip aktualisieren
echo "📦 Aktualisiere Python und pip..."
python3 -m pip install --upgrade pip setuptools wheel

# PlatformIO installieren
echo "🔧 Installiere PlatformIO..."
pip install --upgrade platformio

# PlatformIO Version anzeigen
echo "✅ PlatformIO installiert:"
pio --version

# ESP32 Platform und Tools installieren
echo "📥 Installiere ESP32 Platform..."
pio pkg install --global --platform espressif32

# Dependencies installieren
echo "📚 Installiere Bibliotheken..."
pio pkg install

# Build durchführen
echo "🏗️  Baue Firmware..."
pio run

# Build-Informationen anzeigen
echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║  ✅ Setup abgeschlossen!                               ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "📦 Firmware wurde gebaut:"
ls -lh .pio/build/lilygo-t-display/firmware.bin
echo ""
echo "📥 Download der Firmware:"
echo "   → Rechtsklick auf .pio/build/lilygo-t-display/firmware.bin"
echo "   → 'Download' wählen"
echo ""
echo "🔧 Flash-Anleitungen:"
echo "   → Web-Tool: https://espressif.github.io/esptool-js/"
echo "   → Mit esptool: esptool.py --chip esp32 --port PORT write_flash 0x10000 firmware.bin"
echo ""
echo "📚 Weitere Infos: siehe README.md"
echo ""

# Hilfreiche Aliases erstellen
cat >> ~/.bashrc << 'EOF'

# ESP32 Projekt Aliases
alias build='pio run'
alias clean='pio run --target clean'
alias size='pio run --target size'
alias monitor='pio device monitor'
alias upload='pio run --target upload'

echo ""
echo "💡 Verfügbare Befehle:"
echo "   build   - Firmware bauen"
echo "   clean   - Build-Dateien löschen"
echo "   size    - Firmware-Größe anzeigen"
echo "   monitor - Serieller Monitor"
echo "   upload  - Firmware hochladen (wenn USB verbunden)"
echo ""

EOF

source ~/.bashrc || true

echo "🎉 Bereit zum Entwickeln!"
