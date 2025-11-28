/**
 * BrewOS UI Manager Implementation
 */

#include <Arduino.h>
#include "ui/ui.h"
#include "ui/screen_home.h"
#include "display/theme.h"
#include "display/display_config.h"
#include "config.h"

// Global UI instance
UI ui;

// =============================================================================
// UI Manager Implementation
// =============================================================================

UI::UI()
    : _currentScreen(SCREEN_HOME)
    , _homeBrewTempLabel(nullptr)
    , _homeBrewTempArc(nullptr)
    , _homeSteamTempLabel(nullptr)
    , _homeSteamTempArc(nullptr)
    , _homePressureLabel(nullptr)
    , _homePressureBar(nullptr)
    , _homeStatusLabel(nullptr)
    , _homeStatusIndicator(nullptr)
    , _brewTimerLabel(nullptr)
    , _brewWeightLabel(nullptr)
    , _brewWeightArc(nullptr)
    , _brewFlowLabel(nullptr)
    , _brewStopBtn(nullptr) {
    
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
    
    // Create screens
    createHomeScreen();
    createBrewScreen();
    createSettingsScreen();
    createStatsScreen();
    createWiFiScreen();
    
    // Show home screen
    showScreen(SCREEN_HOME);
    
    LOG_I("UI initialized");
    return true;
}

void UI::update(const ui_state_t& state) {
    // Store state
    _state = state;
    
    // Update current screen
    switch (_currentScreen) {
        case SCREEN_HOME:
            updateHomeScreen();
            break;
        case SCREEN_BREW:
            updateBrewScreen();
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
    
    _currentScreen = screen;
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
    
    // Auto-close timer
    lv_obj_del_delayed(mbox, duration_ms);
}

void UI::showError(const char* title, const char* message) {
    static const char* btns[] = {"OK", ""};
    
    lv_obj_t* mbox = lv_msgbox_create(NULL, title, message, btns, false);
    lv_obj_center(mbox);
    
    // Style the msgbox
    lv_obj_set_style_bg_color(mbox, COLOR_BG_CARD, 0);
    lv_obj_set_style_text_color(mbox, COLOR_TEXT_PRIMARY, 0);
    lv_obj_set_style_radius(mbox, RADIUS_NORMAL, 0);
    
    // Error color for title
    lv_obj_t* title_label = lv_msgbox_get_title(mbox);
    if (title_label) {
        lv_obj_set_style_text_color(title_label, COLOR_ERROR, 0);
    }
    
    // Add to default group for encoder navigation
    lv_group_t* group = lv_group_get_default();
    if (group) {
        lv_obj_t* btns_obj = lv_msgbox_get_btns(mbox);
        if (btns_obj) {
            lv_group_add_obj(group, btns_obj);
        }
    }
}

void UI::handleButtonPress(bool long_press) {
    if (long_press) {
        // Long press - go back or show menu
        if (_currentScreen != SCREEN_HOME) {
            showScreen(SCREEN_HOME);
        } else {
            showScreen(SCREEN_SETTINGS);
        }
    } else {
        // Short press - context dependent navigation
        // Note: Brew start/stop is via physical lever, not button
        if (_currentScreen == SCREEN_HOME) {
            // Could navigate to settings or stats
        }
    }
}

// =============================================================================
// Screen Creation
// =============================================================================

void UI::createHomeScreen() {
    _screens[SCREEN_HOME] = screen_home_create();
    
    // Get element references
    screen_home_t* elements = screen_home_get_elements();
    if (elements) {
        _homeBrewTempLabel = elements->brew_temp_label;
        _homeBrewTempArc = elements->brew_temp_arc;
        _homeSteamTempLabel = elements->steam_temp_label;
        _homePressureLabel = elements->pressure_label;
        _homePressureBar = elements->pressure_bar;
        _homeStatusLabel = elements->status_label;
        _homeStatusIndicator = elements->status_led;
    }
}

void UI::createBrewScreen() {
    // Create brew screen
    _screens[SCREEN_BREW] = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screens[SCREEN_BREW], COLOR_BG_DARK, 0);
    
    lv_obj_t* container = lv_obj_create(_screens[SCREEN_BREW]);
    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_center(container);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    
    // Large timer in center
    _brewTimerLabel = lv_label_create(container);
    lv_label_set_text(_brewTimerLabel, "00:00");
    lv_obj_set_style_text_font(_brewTimerLabel, FONT_TEMP, 0);
    lv_obj_set_style_text_color(_brewTimerLabel, COLOR_TEXT_PRIMARY, 0);
    lv_obj_align(_brewTimerLabel, LV_ALIGN_CENTER, 0, -40);
    
    // Weight display
    _brewWeightLabel = lv_label_create(container);
    lv_label_set_text(_brewWeightLabel, "0.0g / 36.0g");
    lv_obj_set_style_text_font(_brewWeightLabel, FONT_LARGE, 0);
    lv_obj_set_style_text_color(_brewWeightLabel, COLOR_ACCENT_AMBER, 0);
    lv_obj_align(_brewWeightLabel, LV_ALIGN_CENTER, 0, 30);
    
    // Flow rate
    _brewFlowLabel = lv_label_create(container);
    lv_label_set_text(_brewFlowLabel, "0.0 ml/s");
    lv_obj_set_style_text_font(_brewFlowLabel, FONT_NORMAL, 0);
    lv_obj_set_style_text_color(_brewFlowLabel, COLOR_TEXT_MUTED, 0);
    lv_obj_align(_brewFlowLabel, LV_ALIGN_CENTER, 0, 70);
    
    // Progress arc
    _brewWeightArc = lv_arc_create(container);
    lv_obj_set_size(_brewWeightArc, 420, 420);
    lv_obj_center(_brewWeightArc);
    lv_arc_set_range(_brewWeightArc, 0, 100);
    lv_arc_set_value(_brewWeightArc, 0);
    lv_arc_set_bg_angles(_brewWeightArc, 135, 45);
    
    lv_obj_set_style_arc_color(_brewWeightArc, COLOR_ARC_BG, LV_PART_MAIN);
    lv_obj_set_style_arc_width(_brewWeightArc, 15, LV_PART_MAIN);
    lv_obj_set_style_arc_color(_brewWeightArc, COLOR_ACCENT_ORANGE, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(_brewWeightArc, 15, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(_brewWeightArc, true, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(_brewWeightArc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_clear_flag(_brewWeightArc, LV_OBJ_FLAG_CLICKABLE);
    
    // Status hint (brewing is controlled by physical lever)
    lv_obj_t* status_hint = lv_label_create(container);
    lv_label_set_text(status_hint, "Brewing...");
    lv_obj_set_style_text_font(status_hint, FONT_SMALL, 0);
    lv_obj_set_style_text_color(status_hint, COLOR_ACCENT_ORANGE, 0);
    lv_obj_align(status_hint, LV_ALIGN_BOTTOM_MID, 0, -50);
}

void UI::createSettingsScreen() {
    // Placeholder settings screen
    _screens[SCREEN_SETTINGS] = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screens[SCREEN_SETTINGS], COLOR_BG_DARK, 0);
    
    lv_obj_t* label = lv_label_create(_screens[SCREEN_SETTINGS]);
    lv_label_set_text(label, "Settings");
    lv_obj_set_style_text_font(label, FONT_LARGE, 0);
    lv_obj_set_style_text_color(label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_center(label);
    
    // TODO: Add settings menu items
}

void UI::createStatsScreen() {
    // Placeholder stats screen
    _screens[SCREEN_STATS] = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screens[SCREEN_STATS], COLOR_BG_DARK, 0);
    
    lv_obj_t* label = lv_label_create(_screens[SCREEN_STATS]);
    lv_label_set_text(label, "Statistics");
    lv_obj_set_style_text_font(label, FONT_LARGE, 0);
    lv_obj_set_style_text_color(label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_center(label);
    
    // TODO: Add statistics display
}

void UI::createWiFiScreen() {
    // Placeholder WiFi screen
    _screens[SCREEN_WIFI] = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screens[SCREEN_WIFI], COLOR_BG_DARK, 0);
    
    lv_obj_t* label = lv_label_create(_screens[SCREEN_WIFI]);
    lv_label_set_text(label, "WiFi Setup");
    lv_obj_set_style_text_font(label, FONT_LARGE, 0);
    lv_obj_set_style_text_color(label, COLOR_TEXT_PRIMARY, 0);
    lv_obj_center(label);
    
    // TODO: Add WiFi configuration
}

// =============================================================================
// Screen Updates
// =============================================================================

void UI::updateHomeScreen() {
    screen_home_update(_screens[SCREEN_HOME], &_state);
}

void UI::updateBrewScreen() {
    if (!_brewTimerLabel) return;
    
    // Update timer
    uint32_t secs = _state.brew_time_ms / 1000;
    uint32_t mins = secs / 60;
    secs = secs % 60;
    
    char timer_str[16];
    snprintf(timer_str, sizeof(timer_str), "%02lu:%02lu", mins, secs);
    lv_label_set_text(_brewTimerLabel, timer_str);
    
    // Update weight
    char weight_str[32];
    snprintf(weight_str, sizeof(weight_str), "%.1fg / %.1fg", 
             _state.brew_weight, _state.target_weight);
    lv_label_set_text(_brewWeightLabel, weight_str);
    
    // Update flow rate
    char flow_str[16];
    snprintf(flow_str, sizeof(flow_str), "%.1f ml/s", _state.flow_rate);
    lv_label_set_text(_brewFlowLabel, flow_str);
    
    // Update weight arc
    if (_state.target_weight > 0) {
        int pct = (int)((_state.brew_weight / _state.target_weight) * 100);
        pct = LV_CLAMP(0, pct, 100);
        lv_arc_set_value(_brewWeightArc, pct);
    }
}

// =============================================================================
// Helper Functions
// =============================================================================

const char* UI::getStateText(uint8_t state) {
    switch (state) {
        case 0: return "INIT";
        case 1: return "IDLE";
        case 2: return "HEATING";
        case 3: return "READY";
        case 4: return "BREWING";
        case 5: return "FAULT";
        case 6: return "SAFE";
        default: return "UNKNOWN";
    }
}

lv_color_t UI::getStateColor(uint8_t state) {
    switch (state) {
        case 0: return COLOR_INFO;       // INIT
        case 1: return COLOR_TEXT_MUTED; // IDLE
        case 2: return COLOR_WARNING;    // HEATING
        case 3: return COLOR_SUCCESS;    // READY
        case 4: return COLOR_ACCENT_ORANGE; // BREWING
        case 5: return COLOR_ERROR;      // FAULT
        case 6: return COLOR_ERROR;      // SAFE
        default: return COLOR_TEXT_MUTED;
    }
}

