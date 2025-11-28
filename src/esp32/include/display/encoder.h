/**
 * BrewOS Rotary Encoder Driver
 * 
 * Handles rotary encoder and button input for LVGL integration
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>
#include <lvgl.h>
#include "display_config.h"

// =============================================================================
// Button State Enum
// =============================================================================

typedef enum {
    BTN_RELEASED,
    BTN_PRESSED,
    BTN_LONG_PRESSED,
    BTN_DOUBLE_PRESSED
} button_state_t;

// =============================================================================
// Encoder Event Callback
// =============================================================================

typedef void (*encoder_callback_t)(int32_t diff, button_state_t btn);

// =============================================================================
// Encoder Driver Class
// =============================================================================

class Encoder {
public:
    Encoder();
    
    /**
     * Initialize encoder hardware and LVGL input device
     */
    bool begin();
    
    /**
     * Update encoder state - call this in main loop
     */
    void update();
    
    /**
     * Get current encoder position (relative since last read)
     */
    int32_t getPosition() const { return _position; }
    
    /**
     * Get current button state
     */
    button_state_t getButtonState() const { return _buttonState; }
    
    /**
     * Check if button is currently pressed
     */
    bool isPressed() const { return _buttonPressed; }
    
    /**
     * Check if button was long-pressed
     */
    bool wasLongPressed() const { return _buttonState == BTN_LONG_PRESSED; }
    
    /**
     * Check if button was double-pressed
     */
    bool wasDoublePressed() const { return _buttonState == BTN_DOUBLE_PRESSED; }
    
    /**
     * Reset encoder position to zero
     */
    void resetPosition() { _position = 0; }
    
    /**
     * Clear button state (after handling)
     */
    void clearButtonState() { _buttonState = BTN_RELEASED; }
    
    /**
     * Set callback for encoder events
     */
    void setCallback(encoder_callback_t callback) { _callback = callback; }
    
    /**
     * Get LVGL input device (for advanced usage)
     */
    lv_indev_t* getInputDevice() const { return _indev; }
    
private:
    // LVGL input device
    lv_indev_t* _indev;
    lv_indev_drv_t _indevDrv;
    
    // Encoder state
    volatile int32_t _position;
    int32_t _lastReportedPosition;
    
    // Button state
    bool _buttonPressed;
    button_state_t _buttonState;
    unsigned long _buttonPressTime;
    unsigned long _lastButtonReleaseTime;
    bool _waitingForDoublePress;
    bool _longPressHandled;
    
    // Debouncing
    volatile uint8_t _lastEncoded;
    unsigned long _lastEncoderTime;
    
    // Callback
    encoder_callback_t _callback;
    
    // Internal methods
    void readEncoder();
    void readButton();
    
    // Static callbacks for LVGL
    static void readCallback(lv_indev_drv_t* drv, lv_indev_data_t* data);
    
    // ISR for encoder (static wrapper)
    static void IRAM_ATTR encoderISR();
};

// Global encoder instance
extern Encoder encoder;

#endif // ENCODER_H

