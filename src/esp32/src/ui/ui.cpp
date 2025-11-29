/**
 * BrewOS UI Manager Implementation
 * 
 * Manages all screens and navigation
 */

#include <Arduino.h>
#include "ui/ui.h"
#include "ui/screen_setup.h"
#include "ui/screen_idle.h"
#include "ui/screen_home.h"
#include "ui/screen_brewing.h"
#include "ui/screen_complete.h"
#include "ui/screen_settings.h"
#include "ui/screen_alarm.h"
#include "display/theme.h"
#include "display/display_config.h"
#include "config.h"

// Global UI instance
UI ui;

// Local state for tracking
static bool was_brewing = false;
static uint32_t last_brew_time = 0;
static float last_brew_weight = 0;

// =============================================================================
// UI Manager Implementation
// =============================================================================

UI::UI()
    : _currentScreen(SCREEN_HOME)
    , _previousScreen(SCREEN_HOME)
    , _onTurnOn(nullptr)
    , _onTurnOff(nullptr)
    , _onSetTemp(nullptr)
    , _onSetStrategy(nullptr)
    , _onTareScale(nullptr)
    , _onSetTargetWeight(nullptr) {
    
    // Initialize state
    memset(&_state, 0, sizeof(_state));
    
    // Initialize screen pointers
    for (int i = 0; i < SCREEN_COUNT; i++) {
        _screens[i] = nullptr;
    }
}

bool UI::begin() {
    LOG_I("Initializing UI...");
    
    // Initialize theme
    theme_init();
    
    // Create all screens
    createSetupScreen();
    createIdleScreen();
    createHomeScreen();
    createBrewingScreen();
    createCompleteScreen();
    createSettingsScreen();
    createTempSettingsScreen();
    createScaleScreen();
    createAlarmScreen();
    
    // Show initial screen based on WiFi state
    // In practice, main.cpp will call showScreen based on actual state
    showScreen(SCREEN_HOME);
    
    LOG_I("UI initialized with %d screens", SCREEN_COUNT);
    return true;
}

void UI::update(const ui_state_t& state) {
    // Store previous state for change detection
    bool state_changed = (state.machine_state != _state.machine_state);
    bool brewing_changed = (state.is_brewing != _state.is_brewing);
    
    // Store new state
    _state = state;
    
    // Auto screen switching based on machine state
    checkAutoScreenSwitch();
    
    // Track brewing state for shot complete
    if (brewing_changed) {
        if (state.is_brewing) {
            // Brewing started
            was_brewing = true;
            screen_brewing_reset();
        } else if (was_brewing) {
            // Brewing just ended - store final values for complete screen
            last_brew_time = _state.brew_time_ms;
            last_brew_weight = _state.brew_weight;
            was_brewing = false;
            
            // Show complete screen (if we have valid data)
            if (last_brew_time > 5000 || last_brew_weight > 5.0f) {
                screen_complete_update(last_brew_time, last_brew_weight, 
                                       _state.dose_weight, _state.flow_rate);
                showScreen(SCREEN_COMPLETE);
            }
        }
    }
    
    // Update current screen
    switch (_currentScreen) {
        case SCREEN_SETUP:
            updateSetupScreen();
            break;
        case SCREEN_IDLE:
            updateIdleScreen();
            break;
        case SCREEN_HOME:
            updateHomeScreen();
            break;
        case SCREEN_BREWING:
            updateBrewingScreen();
            break;
        case SCREEN_COMPLETE:
            updateCompleteScreen();
            break;
        case SCREEN_SETTINGS:
            updateSettingsScreen();
            break;
        case SCREEN_ALARM:
            updateAlarmScreen();
            break;
        default:
            break;
    }
}

void UI::showScreen(screen_id_t screen) {
    if (screen >= SCREEN_COUNT || !_screens[screen]) {
        LOG_W("Invalid screen: %d", screen);
        return;
    }
    
    _previousScreen = _currentScreen;
    _currentScreen = screen;
    
    // Use fade animation for transitions
    lv_scr_load_anim(_screens[screen], LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
    
    LOG_I("Switched to screen: %d", screen);
}

void UI::showNotification(const char* message, uint16_t duration_ms) {
    // Create notification using msgbox
    lv_obj_t* mbox = lv_msgbox_create(NULL, NULL, message, NULL, false);
    lv_obj_center(mbox);
    
    // Style the msgbox
    lv_obj_set_style_bg_color(mbox, COLOR_BG_CARD, 0);
    lv_obj_set_style_text_color(mbox, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_radius(mbox, RADIUS_NORMAL, 0);
    lv_obj_set_style_border_color(mbox, COLOR_ACCENT_AMBER, 0);
    lv_obj_set_style_border_width(mbox, 2, 0);
    
    // Auto-close timer
    lv_obj_del_delayed(mbox, duration_ms);
}

void UI::showAlarm(uint8_t code, const char* message) {
    screen_alarm_set(code, message);
    showScreen(SCREEN_ALARM);
}

void UI::clearAlarm() {
    screen_alarm_clear();
    showScreen(_previousScreen);
}

void UI::handleEncoder(int direction) {
    switch (_currentScreen) {
        case SCREEN_IDLE:
            // Navigate heating strategies
            screen_idle_select_strategy(
                screen_idle_get_selected_strategy() + direction);
            break;
            
        case SCREEN_HOME:
            // Could navigate between info panels or to settings
            if (direction > 0) {
                showScreen(SCREEN_SETTINGS);
            }
            break;
            
        case SCREEN_SETTINGS:
            screen_settings_navigate(direction);
            break;
            
        case SCREEN_BREWING:
            // During brewing, encoder could adjust target weight
            if (_onSetTargetWeight) {
                float new_target = _state.target_weight + (direction * 0.5f);
                if (new_target >= 10.0f && new_target <= 100.0f) {
                    _onSetTargetWeight(new_target);
                }
            }
            break;
            
        default:
            break;
    }
}

void UI::handleButtonPress() {
    switch (_currentScreen) {
        case SCREEN_SETUP:
            // Short press during setup - no action (wait for WiFi config)
            break;
            
        case SCREEN_IDLE:
            // Turn on machine with selected strategy
            if (_onTurnOn) {
                _onTurnOn();
            }
            if (_onSetStrategy) {
                _onSetStrategy(screen_idle_get_selected_strategy());
            }
            showScreen(SCREEN_HOME);
            break;
            
        case SCREEN_HOME:
            // Short press on home - go to settings
            showScreen(SCREEN_SETTINGS);
            break;
            
        case SCREEN_BREWING:
            // During brewing, button tares scale
            if (_onTareScale) {
                _onTareScale();
            }
            break;
            
        case SCREEN_COMPLETE:
            // Dismiss complete screen
            showScreen(SCREEN_HOME);
            break;
            
        case SCREEN_SETTINGS:
            // Select current menu item
            screen_settings_select();
            break;
            
        case SCREEN_ALARM:
            // Acknowledge alarm
            clearAlarm();
            break;
            
        default:
            break;
    }
}

void UI::handleLongPress() {
    switch (_currentScreen) {
        case SCREEN_HOME:
            // Long press on home - turn off machine
            if (_onTurnOff) {
                _onTurnOff();
            }
            showScreen(SCREEN_IDLE);
            break;
            
        case SCREEN_SETTINGS:
        case SCREEN_TEMP_SETTINGS:
        case SCREEN_SCALE:
            // Long press goes back
            showScreen(SCREEN_HOME);
            break;
            
        case SCREEN_BREWING:
            // Long press during brewing - show home (can't stop via UI)
            showScreen(SCREEN_HOME);
            break;
            
        default:
            // Default: go to home
            showScreen(SCREEN_HOME);
            break;
    }
}

void UI::handleDoublePress() {
    switch (_currentScreen) {
        case SCREEN_BREWING:
            // Double press - tare scale
            if (_onTareScale) {
                _onTareScale();
                showNotification("Scale tared", 1000);
            }
            break;
            
        case SCREEN_HOME:
            // Double press on home - quick settings access
            showScreen(SCREEN_SETTINGS);
            break;
            
        default:
            break;
    }
}

// =============================================================================
// Screen Creation Methods
// =============================================================================

void UI::createSetupScreen() {
    _screens[SCREEN_SETUP] = screen_setup_create();
}

void UI::createIdleScreen() {
    _screens[SCREEN_IDLE] = screen_idle_create();
}

void UI::createHomeScreen() {
    _screens[SCREEN_HOME] = screen_home_create();
}

void UI::createBrewingScreen() {
    _screens[SCREEN_BREWING] = screen_brewing_create();
}

void UI::createCompleteScreen() {
    _screens[SCREEN_COMPLETE] = screen_complete_create();
}

void UI::createSettingsScreen() {
    _screens[SCREEN_SETTINGS] = screen_settings_create();
    
    // Set callback for menu item selection
    screen_settings_set_select_callback([](settings_item_t item) {
        switch (item) {
            case SETTINGS_WIFI:
                ui.showScreen(SCREEN_SETUP);
                break;
            case SETTINGS_SCALE:
                ui.showScreen(SCREEN_SCALE);
                break;
            case SETTINGS_TEMP:
                ui.showScreen(SCREEN_TEMP_SETTINGS);
                break;
            default:
                ui.showNotification("Coming soon", 2000);
                break;
        }
    });
}

void UI::createTempSettingsScreen() {
    // Temperature settings - create a simple placeholder for now
    _screens[SCREEN_TEMP_SETTINGS] = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screens[SCREEN_TEMP_SETTINGS], COLOR_BG_DARK, 0);
    
    lv_obj_t* container = lv_obj_create(_screens[SCREEN_TEMP_SETTINGS]);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_center(container);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* title = lv_label_create(container);
    lv_label_set_text(title, "Temperature");
    lv_obj_set_style_text_font(title, FONT_LARGE, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 50);
    
    // Brew temp display
    lv_obj_t* brew_label = lv_label_create(container);
    lv_label_set_text(brew_label, "Brew: 93.0°C");
    lv_obj_set_style_text_font(brew_label, FONT_XLARGE, 0);
    lv_obj_set_style_text_color(brew_label, COLOR_ACCENT_AMBER, 0);
    lv_obj_align(brew_label, LV_ALIGN_CENTER, 0, -30);
    
    // Steam temp display
    lv_obj_t* steam_label = lv_label_create(container);
    lv_label_set_text(steam_label, "Steam: 145.0°C");
    lv_obj_set_style_text_font(steam_label, FONT_XLARGE, 0);
    lv_obj_set_style_text_color(steam_label, COLOR_TEMP_HOT, 0);
    lv_obj_align(steam_label, LV_ALIGN_CENTER, 0, 30);
    
    lv_obj_t* hint = lv_label_create(container);
    lv_label_set_text(hint, "Rotate to adjust • Long press to exit");
    lv_obj_set_style_text_font(hint, FONT_SMALL, 0);
    lv_obj_set_style_text_color(hint, COLOR_TEXT_MUTED, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -50);
}

void UI::createScaleScreen() {
    // Scale pairing screen - placeholder
    _screens[SCREEN_SCALE] = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screens[SCREEN_SCALE], COLOR_BG_DARK, 0);
    
    lv_obj_t* container = lv_obj_create(_screens[SCREEN_SCALE]);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_center(container);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* icon = lv_label_create(container);
    lv_label_set_text(icon, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_font(icon, FONT_TEMP, 0);
    lv_obj_set_style_text_color(icon, COLOR_INFO, 0);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, -60);
    
    lv_obj_t* title = lv_label_create(container);
    lv_label_set_text(title, "Bluetooth Scale");
    lv_obj_set_style_text_font(title, FONT_LARGE, 0);
    lv_obj_set_style_text_color(title, COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 10);
    
    lv_obj_t* status = lv_label_create(container);
    lv_label_set_text(status, "Searching for scales...");
    lv_obj_set_style_text_font(status, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(status, COLOR_TEXT_MUTED, 0);
    lv_obj_align(status, LV_ALIGN_CENTER, 0, 50);
    
    // Spinner
    lv_obj_t* spinner = lv_spinner_create(container, 1000, 60);
    lv_obj_set_size(spinner, 40, 40);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 100);
    lv_obj_set_style_arc_color(spinner, COLOR_INFO, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, COLOR_BG_ELEVATED, LV_PART_MAIN);
    
    lv_obj_t* hint = lv_label_create(container);
    lv_label_set_text(hint, "Long press to cancel");
    lv_obj_set_style_text_font(hint, FONT_SMALL, 0);
    lv_obj_set_style_text_color(hint, COLOR_TEXT_MUTED, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -50);
}

void UI::createAlarmScreen() {
    _screens[SCREEN_ALARM] = screen_alarm_create();
}

// =============================================================================
// Screen Update Methods
// =============================================================================

void UI::updateSetupScreen() {
    screen_setup_update(&_state);
}

void UI::updateIdleScreen() {
    screen_idle_update(&_state);
}

void UI::updateHomeScreen() {
    screen_home_update(_screens[SCREEN_HOME], &_state);
}

void UI::updateBrewingScreen() {
    screen_brewing_update(&_state);
}

void UI::updateCompleteScreen() {
    // Complete screen is updated when shown (with final values)
}

void UI::updateSettingsScreen() {
    screen_settings_update(&_state);
}

void UI::updateAlarmScreen() {
    // Alarm screen is updated when set
}

// =============================================================================
// Helper Methods
// =============================================================================

void UI::checkAutoScreenSwitch() {
    // Don't auto-switch if on alarm or setup screens
    if (_currentScreen == SCREEN_ALARM || _currentScreen == SCREEN_SETUP) {
        return;
    }
    
    // Show alarm screen if alarm is active
    if (_state.alarm_active && _currentScreen != SCREEN_ALARM) {
        showAlarm(_state.alarm_code, nullptr);
        return;
    }
    
    // Switch to brewing screen when brewing starts
    if (_state.is_brewing && _currentScreen != SCREEN_BREWING) {
        showScreen(SCREEN_BREWING);
        return;
    }
    
    // Show setup screen if in AP mode
    if (_state.wifi_ap_mode && _currentScreen != SCREEN_SETUP) {
        showScreen(SCREEN_SETUP);
        return;
    }
    
    // Show idle screen if machine is off
    if (_state.machine_state == STATE_IDLE && 
        _currentScreen != SCREEN_IDLE && 
        _currentScreen != SCREEN_SETTINGS) {
        // Only auto-switch to idle if not in settings
        // showScreen(SCREEN_IDLE);
        return;
    }
}

const char* UI::getStateText(uint8_t state) {
    switch (state) {
        case STATE_INIT: return "INIT";
        case STATE_IDLE: return "OFF";
        case STATE_HEATING: return "HEATING";
        case STATE_READY: return "READY";
        case STATE_BREWING: return "BREWING";
        case STATE_STEAMING: return "STEAMING";
        case STATE_COOLDOWN: return "COOLING";
        case STATE_FAULT: return "FAULT";
        case STATE_SAFE: return "SAFE MODE";
        default: return "UNKNOWN";
    }
}

const char* UI::getStrategyText(uint8_t strategy) {
    switch (strategy) {
        case HEAT_BREW_ONLY: return "Brew Only";
        case HEAT_SEQUENTIAL: return "Sequential";
        case HEAT_STEAM_PRIORITY: return "Steam Priority";
        case HEAT_PARALLEL: return "Parallel";
        case HEAT_SMART_STAGGER: return "Smart Stagger";
        default: return "Unknown";
    }
}

lv_color_t UI::getStateColor(uint8_t state) {
    switch (state) {
        case STATE_INIT: return COLOR_INFO;
        case STATE_IDLE: return COLOR_TEXT_MUTED;
        case STATE_HEATING: return COLOR_WARNING;
        case STATE_READY: return COLOR_SUCCESS;
        case STATE_BREWING: return COLOR_ACCENT_ORANGE;
        case STATE_STEAMING: return COLOR_TEMP_HOT;
        case STATE_COOLDOWN: return COLOR_INFO;
        case STATE_FAULT: return COLOR_ERROR;
        case STATE_SAFE: return COLOR_ERROR;
        default: return COLOR_TEXT_MUTED;
    }
}
