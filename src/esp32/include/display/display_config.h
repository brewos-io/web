/**
 * BrewOS Display Configuration
 * 
 * Target: UEDX48480021-MD80E (ESP32-S3 Knob Display)
 * - 2.1" Round IPS 480x480
 * - GC9A01 or ST7701 controller (common for round displays)
 * - Rotary Encoder + Push Button
 */

#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

// =============================================================================
// Display Specifications
// =============================================================================

#define DISPLAY_WIDTH           480
#define DISPLAY_HEIGHT          480
#define DISPLAY_ROTATION        0       // 0, 90, 180, 270

// Round display - center point
#define DISPLAY_CENTER_X        (DISPLAY_WIDTH / 2)
#define DISPLAY_CENTER_Y        (DISPLAY_HEIGHT / 2)
#define DISPLAY_RADIUS          (DISPLAY_WIDTH / 2)

// Safe area for round display (content should stay within this)
// Elements at top/bottom need ~70-80px margin from edge
// Elements at corners are cut off - avoid placing content there
#define DISPLAY_SAFE_MARGIN     70
#define DISPLAY_SAFE_RADIUS     (DISPLAY_RADIUS - DISPLAY_SAFE_MARGIN)

// =============================================================================
// Display SPI Pins (adjust based on actual board pinout)
// These need to be verified for UEDX48480021-MD80E
// =============================================================================

// SPI pins for display
#define DISPLAY_MOSI_PIN        11      // SPI MOSI
#define DISPLAY_SCLK_PIN        12      // SPI Clock
#define DISPLAY_CS_PIN          10      // Chip Select
#define DISPLAY_DC_PIN          13      // Data/Command
#define DISPLAY_RST_PIN         14      // Reset
#define DISPLAY_BL_PIN          21      // Backlight PWM

// SPI settings
#define DISPLAY_SPI_FREQ        40000000    // 40MHz SPI clock
#define DISPLAY_SPI_HOST        SPI2_HOST

// =============================================================================
// Rotary Encoder Pins (adjust based on actual board pinout)
// =============================================================================

#define ENCODER_A_PIN           1       // Encoder A (CLK)
#define ENCODER_B_PIN           2       // Encoder B (DT)
#define ENCODER_BTN_PIN         0       // Encoder Button (SW) - also boot button

// Encoder settings
#define ENCODER_STEPS_PER_NOTCH 4       // Typical for most encoders
#define ENCODER_DEBOUNCE_MS     5       // Debounce time

// Button settings
#define BTN_DEBOUNCE_MS         50      // Button debounce
#define BTN_LONG_PRESS_MS       2000    // Long press threshold
#define BTN_DOUBLE_PRESS_MS     300     // Double press window

// =============================================================================
// Backlight Settings
// =============================================================================

#define BACKLIGHT_PWM_CHANNEL   0
#define BACKLIGHT_PWM_FREQ      5000    // 5kHz PWM
#define BACKLIGHT_PWM_RES       8       // 8-bit resolution (0-255)

#define BACKLIGHT_MAX           255
#define BACKLIGHT_MIN           10
#define BACKLIGHT_DEFAULT       200

// Auto-dim settings
#define BACKLIGHT_DIM_TIMEOUT   30000   // Dim after 30s idle
#define BACKLIGHT_DIM_LEVEL     50      // Dimmed brightness
#define BACKLIGHT_OFF_TIMEOUT   60000   // Turn off after 60s idle (0 = never)

// =============================================================================
// LVGL Display Buffer
// =============================================================================

// Use full screen buffer in PSRAM for smooth animations
#define LVGL_BUFFER_SIZE        (DISPLAY_WIDTH * DISPLAY_HEIGHT)

// Alternative: partial buffer for less memory usage
// #define LVGL_BUFFER_SIZE     (DISPLAY_WIDTH * 40)

// Double buffering for DMA (recommended for smooth display)
#define LVGL_DOUBLE_BUFFER      1

#endif // DISPLAY_CONFIG_H

