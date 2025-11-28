/**
 * BrewOS Rotary Encoder Driver Implementation
 */

#include "display/encoder.h"
#include "display/display.h"
#include "config.h"

// Global encoder instance
Encoder encoder;

// Static pointer for ISR
static Encoder* encoderInstance = nullptr;

// =============================================================================
// Encoder Implementation
// =============================================================================

Encoder::Encoder()
    : _indev(nullptr)
    , _position(0)
    , _lastReportedPosition(0)
    , _buttonPressed(false)
    , _buttonState(BTN_RELEASED)
    , _buttonPressTime(0)
    , _lastButtonReleaseTime(0)
    , _waitingForDoublePress(false)
    , _longPressHandled(false)
    , _lastEncoded(0)
    , _lastEncoderTime(0)
    , _callback(nullptr) {
}

bool Encoder::begin() {
    LOG_I("Initializing encoder...");
    
    // Store instance for ISR
    encoderInstance = this;
    
    // Configure encoder pins with internal pullups
    pinMode(ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(ENCODER_B_PIN, INPUT_PULLUP);
    pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);
    
    // Read initial state
    uint8_t a = digitalRead(ENCODER_A_PIN);
    uint8_t b = digitalRead(ENCODER_B_PIN);
    _lastEncoded = (a << 1) | b;
    
    // Attach interrupts for encoder
    attachInterrupt(digitalPinToInterrupt(ENCODER_A_PIN), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_B_PIN), encoderISR, CHANGE);
    
    // Initialize LVGL input device
    lv_indev_drv_init(&_indevDrv);
    _indevDrv.type = LV_INDEV_TYPE_ENCODER;
    _indevDrv.read_cb = readCallback;
    _indevDrv.user_data = this;
    
    _indev = lv_indev_drv_register(&_indevDrv);
    
    // Create a group and set it as default for encoder navigation
    lv_group_t* group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_set_group(_indev, group);
    
    LOG_I("Encoder initialized on pins A=%d, B=%d, BTN=%d",
          ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_BTN_PIN);
    
    return true;
}

void IRAM_ATTR Encoder::encoderISR() {
    if (!encoderInstance) return;
    encoderInstance->readEncoder();
}

void IRAM_ATTR Encoder::readEncoder() {
    // Debounce
    unsigned long now = millis();
    if (now - _lastEncoderTime < ENCODER_DEBOUNCE_MS) return;
    _lastEncoderTime = now;
    
    // Read current state
    uint8_t a = digitalRead(ENCODER_A_PIN);
    uint8_t b = digitalRead(ENCODER_B_PIN);
    uint8_t encoded = (a << 1) | b;
    
    // State machine for quadrature decoding
    uint8_t sum = (_lastEncoded << 2) | encoded;
    
    // Valid transitions for CW: 0001, 0111, 1110, 1000
    // Valid transitions for CCW: 0010, 0100, 1101, 1011
    switch (sum) {
        case 0b0001:
        case 0b0111:
        case 0b1110:
        case 0b1000:
            _position++;
            break;
        case 0b0010:
        case 0b0100:
        case 0b1101:
        case 0b1011:
            _position--;
            break;
    }
    
    _lastEncoded = encoded;
}

void Encoder::update() {
    // Read button state
    readButton();
    
    // Check for position change
    int32_t diff = _position - _lastReportedPosition;
    if (diff != 0 || _buttonState != BTN_RELEASED) {
        // Reset display idle timer on any input
        display.resetIdleTimer();
        
        // Call callback if set
        if (_callback) {
            _callback(diff, _buttonState);
        }
        
        _lastReportedPosition = _position;
    }
}

void Encoder::readButton() {
    static bool lastRaw = true;  // Pulled up, so HIGH when not pressed
    bool raw = digitalRead(ENCODER_BTN_PIN);
    unsigned long now = millis();
    
    // Button pressed (active low)
    if (!raw && lastRaw) {
        // Debounce
        if (now - _lastButtonReleaseTime > BTN_DEBOUNCE_MS) {
            _buttonPressed = true;
            _buttonPressTime = now;
            _longPressHandled = false;
        }
    }
    // Button released
    else if (raw && !lastRaw) {
        if (_buttonPressed) {
            _buttonPressed = false;
            
            // Check if it was a long press (already handled)
            if (_longPressHandled) {
                _buttonState = BTN_RELEASED;
                _longPressHandled = false;
            }
            // Check for double press
            else if (_waitingForDoublePress && (now - _lastButtonReleaseTime < BTN_DOUBLE_PRESS_MS)) {
                _buttonState = BTN_DOUBLE_PRESSED;
                _waitingForDoublePress = false;
            }
            // Short press - wait for potential double press
            else {
                _buttonState = BTN_PRESSED;
                _waitingForDoublePress = true;
                _lastButtonReleaseTime = now;
            }
        }
    }
    // Check for long press while held
    else if (!raw && _buttonPressed && !_longPressHandled) {
        if (now - _buttonPressTime >= BTN_LONG_PRESS_MS) {
            _buttonState = BTN_LONG_PRESSED;
            _longPressHandled = true;
            _waitingForDoublePress = false;
        }
    }
    
    // Clear double-press waiting after timeout
    if (_waitingForDoublePress && (now - _lastButtonReleaseTime >= BTN_DOUBLE_PRESS_MS)) {
        _waitingForDoublePress = false;
        // Short press was the final state
    }
    
    lastRaw = raw;
}

void Encoder::readCallback(lv_indev_drv_t* drv, lv_indev_data_t* data) {
    Encoder* enc = (Encoder*)drv->user_data;
    
    // Get encoder difference
    int32_t diff = enc->_position - enc->_lastReportedPosition;
    enc->_lastReportedPosition = enc->_position;
    
    // Report to LVGL
    data->enc_diff = diff / ENCODER_STEPS_PER_NOTCH;
    
    // Report button state
    if (enc->_buttonPressed) {
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    
    // Reset display idle on any input
    if (diff != 0 || enc->_buttonPressed) {
        display.resetIdleTimer();
    }
}

