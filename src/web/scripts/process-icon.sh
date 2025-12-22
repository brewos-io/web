#!/bin/bash
# Process and crop the new PWA icon
# This script crops the icon to focus on the center content and generates all required sizes

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PUBLIC_DIR="$SCRIPT_DIR/../public"
SOURCE_ICON="$PUBLIC_DIR/icon-source-new.png"
OUTPUT_ICON="$PUBLIC_DIR/icon-source.png"

# Colors
BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}🎨 Processing new PWA icon...${NC}"

# Check if source icon exists
if [ ! -f "$SOURCE_ICON" ]; then
    echo -e "${YELLOW}❌ Source icon not found at: $SOURCE_ICON${NC}"
    echo "Please save your new icon image as: icon-source-new.png"
    exit 1
fi

# Get image dimensions
WIDTH=$(sips -g pixelWidth "$SOURCE_ICON" | grep pixelWidth | awk '{print $2}')
HEIGHT=$(sips -g pixelHeight "$SOURCE_ICON" | grep pixelHeight | awk '{print $2}')

echo -e "${BLUE}📐 Original size: ${WIDTH}x${HEIGHT}${NC}"

# Calculate crop to focus on center (crop 20% from edges to zoom in on center)
CROP_PERCENT=20
CROP_X=$((WIDTH * CROP_PERCENT / 100))
CROP_Y=$((HEIGHT * CROP_PERCENT / 100))
CROP_WIDTH=$((WIDTH - CROP_X * 2))
CROP_HEIGHT=$((HEIGHT - CROP_Y * 2))

echo -e "${BLUE}✂️  Cropping to focus on center content...${NC}"
echo -e "   Crop area: ${CROP_X},${CROP_Y} ${CROP_WIDTH}x${CROP_HEIGHT}"

# Crop and resize to 2048x2048 (square, high resolution for app icons)
sips -c "$CROP_WIDTH" "$CROP_HEIGHT" "$SOURCE_ICON" \
     --cropOffset "$CROP_X" "$CROP_Y" \
     --out "$OUTPUT_ICON.tmp" > /dev/null 2>&1

# Resize to 2048x2048 square
sips -z 2048 2048 "$OUTPUT_ICON.tmp" --out "$OUTPUT_ICON" > /dev/null 2>&1
rm -f "$OUTPUT_ICON.tmp"

echo -e "${GREEN}✅ Processed icon saved to: $OUTPUT_ICON${NC}"

# Generate all required icon sizes
echo -e "${BLUE}📦 Generating icon sizes...${NC}"

mkdir -p "$PUBLIC_DIR/icons"

# Standard icons
sips -z 512 512 "$OUTPUT_ICON" --out "$PUBLIC_DIR/icons/icon-512.png" > /dev/null 2>&1
sips -z 192 192 "$OUTPUT_ICON" --out "$PUBLIC_DIR/icons/icon-192.png" > /dev/null 2>&1

# Maskable icons (same images, Android will apply masking)
cp "$PUBLIC_DIR/icons/icon-512.png" "$PUBLIC_DIR/icons/icon-maskable-512.png"
cp "$PUBLIC_DIR/icons/icon-192.png" "$PUBLIC_DIR/icons/icon-maskable-192.png"

# Favicons and Apple touch icon
sips -z 180 180 "$OUTPUT_ICON" --out "$PUBLIC_DIR/apple-touch-icon.png" > /dev/null 2>&1
sips -z 32 32 "$OUTPUT_ICON" --out "$PUBLIC_DIR/favicon-32x32.png" > /dev/null 2>&1
sips -z 16 16 "$OUTPUT_ICON" --out "$PUBLIC_DIR/favicon-16x16.png" > /dev/null 2>&1

echo -e "${GREEN}✅ All icon sizes generated!${NC}"
echo ""
echo -e "${BLUE}📊 Generated files:${NC}"
ls -lh "$PUBLIC_DIR/icons/"*.png "$PUBLIC_DIR"/favicon*.png "$PUBLIC_DIR"/apple-touch-icon.png 2>/dev/null | awk '{print "   " $9 " (" $5 ")"}'

echo ""
echo -e "${GREEN}✅ Icon processing complete!${NC}"

