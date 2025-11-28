/**
 * BrewOS UI Manager
 * 
 * Manages all UI screens and navigation
 */

#ifndef UI_H
#define UI_H

#include <lvgl.h>

// =============================================================================
// Screen IDs
// =============================================================================

typedef enum {
    SCREEN_HOME,
    SCREEN_BREW,
    SCREEN_SETTINGS,
    SCREEN_STATS,
    SCREEN_WIFI,
    SCREEN_COUNT
} screen_id_t;

// =============================================================================
// Machine State for UI Display
// =============================================================================

typedef struct {
    // Temperatures
    float brew_temp;
    float brew_setpoint;
    float steam_temp;
    float steam_setpoint;
    
    // Pressure
    float pressure;
    
    // State
    uint8_t machine_state;      // STATE_INIT, STATE_IDLE, etc.
    bool is_brewing;
    bool is_heating;
    bool water_low;
    bool alarm_active;
    
    // Brewing info
    uint32_t brew_time_ms;      // Current brew time
    float brew_weight;          // Current weight (from scale)
    float target_weight;        // Target weight
    float flow_rate;            // ml/s
    
    // Connection status
    bool pico_connected;
    bool wifi_connected;
    bool mqtt_connected;
    bool scale_connected;
    
    // WiFi info
    char wifi_ssid[32];
    int wifi_rssi;
} ui_state_t;

// =============================================================================
// UI Manager Class
// =============================================================================

class UI {
public:
    UI();
    
    /**
     * Initialize all UI screens
     * Call after display.begin()
     */
    bool begin();
    
    /**
     * Update UI with new state data
     */
    void update(const ui_state_t& state);
    
    /**
     * Switch to a specific screen
     */
    void showScreen(screen_id_t screen);
    
    /**
     * Get current screen
     */
    screen_id_t getCurrentScreen() const { return _currentScreen; }
    
    /**
     * Show a notification/toast message
     */
    void showNotification(const char* message, uint16_t duration_ms = 3000);
    
    /**
     * Show an error dialog
     */
    void showError(const char* title, const char* message);
    
    /**
     * Handle encoder button events
     */
    void handleButtonPress(bool long_press);
    
private:
    screen_id_t _currentScreen;
    ui_state_t _state;
    
    // Screen objects
    lv_obj_t* _screens[SCREEN_COUNT];
    
    // Home screen elements
    lv_obj_t* _homeBrewTempLabel;
    lv_obj_t* _homeBrewTempArc;
    lv_obj_t* _homeSteamTempLabel;
    lv_obj_t* _homeSteamTempArc;
    lv_obj_t* _homePressureLabel;
    lv_obj_t* _homePressureBar;
    lv_obj_t* _homeStatusLabel;
    lv_obj_t* _homeStatusIndicator;
    
    // Brew screen elements
    lv_obj_t* _brewTimerLabel;
    lv_obj_t* _brewWeightLabel;
    lv_obj_t* _brewWeightArc;
    lv_obj_t* _brewFlowLabel;
    lv_obj_t* _brewStopBtn;
    
    // Screen creation methods
    void createHomeScreen();
    void createBrewScreen();
    void createSettingsScreen();
    void createStatsScreen();
    void createWiFiScreen();
    
    // Update methods
    void updateHomeScreen();
    void updateBrewScreen();
    
    // Helper methods
    const char* getStateText(uint8_t state);
    lv_color_t getStateColor(uint8_t state);
};

// Global UI instance
extern UI ui;

#endif // UI_H

