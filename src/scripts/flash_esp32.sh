#!/bin/bash
# Flash ESP32 firmware via USB
# Usage: ./scripts/flash_esp32.sh [port]

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ESP32_DIR="$SCRIPT_DIR/../esp32"
FIRMWARE="$ESP32_DIR/.pio/build/esp32s3/firmware.bin"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}"
echo "========================================"
echo "  BrewOS ESP32 Flasher"
echo "========================================"
echo -e "${NC}"

# Check if firmware exists
if [ ! -f "$FIRMWARE" ]; then
    echo -e "${YELLOW}⚠ Firmware not found. Building first...${NC}"
    cd "$ESP32_DIR"
    pio run -e esp32s3
    echo ""
fi

# Detect port
if [ -n "$1" ]; then
    PORT="$1"
else
    # Auto-detect on macOS
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # Look for common ESP32-S3 USB patterns
        PORT=$(ls /dev/cu.usbmodem* /dev/cu.usbserial* /dev/cu.wchusbserial* 2>/dev/null | head -1)
    else
        # Linux
        PORT=$(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -1)
    fi
fi

if [ -z "$PORT" ]; then
    echo -e "${RED}✗ No ESP32 device found!${NC}"
    echo ""
    echo "Available serial ports:"
    if [[ "$OSTYPE" == "darwin"* ]]; then
        ls /dev/cu.* 2>/dev/null || echo "  (none)"
    else
        ls /dev/tty{USB,ACM}* 2>/dev/null || echo "  (none)"
    fi
    echo ""
    echo "Usage: $0 [port]"
    echo "Example: $0 /dev/cu.usbmodem14101"
    exit 1
fi

echo -e "📍 Port: ${GREEN}$PORT${NC}"
echo -e "📦 Firmware: ${GREEN}$(basename $FIRMWARE)${NC} ($(du -h "$FIRMWARE" | cut -f1))"
echo ""

# Flash using PlatformIO
echo -e "${YELLOW}⚡ Flashing ESP32...${NC}"
cd "$ESP32_DIR"
pio run -e esp32s3 -t upload --upload-port "$PORT"

echo ""
echo -e "${GREEN}✓ Flash complete!${NC}"
echo ""
echo -e "Monitor serial output with:"
echo -e "  ${CYAN}pio device monitor -p $PORT${NC}"

