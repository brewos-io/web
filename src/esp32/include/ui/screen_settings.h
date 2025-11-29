/**
 * BrewOS Settings Screen
 * 
 * Settings menu with radial options:
 * - WiFi
 * - MQTT
 * - Scale
 * - Temperature
 * - System
 */

#ifndef SCREEN_SETTINGS_H
#define SCREEN_SETTINGS_H

#include <lvgl.h>
#include "ui.h"

/**
 * Settings menu items
 */
typedef enum {
    SETTINGS_WIFI,
    SETTINGS_MQTT,
    SETTINGS_SCALE,
    SETTINGS_TEMP,
    SETTINGS_SYSTEM,
    SETTINGS_ABOUT,
    SETTINGS_COUNT
} settings_item_t;

/**
 * Create the settings screen
 */
lv_obj_t* screen_settings_create(void);

/**
 * Update settings screen with current state
 */
void screen_settings_update(const ui_state_t* state);

/**
 * Navigate menu (encoder rotation)
 */
void screen_settings_navigate(int direction);

/**
 * Get current selection
 */
settings_item_t screen_settings_get_selection(void);

/**
 * Select current item (encoder press)
 */
void screen_settings_select(void);

/**
 * Callback for menu item selection
 */
typedef void (*settings_select_callback_t)(settings_item_t item);
void screen_settings_set_select_callback(settings_select_callback_t callback);

#endif // SCREEN_SETTINGS_H

