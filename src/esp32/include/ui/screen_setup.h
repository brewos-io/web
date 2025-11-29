/**
 * BrewOS Setup Screen
 * 
 * WiFi AP mode setup screen showing:
 * - AP SSID and password
 * - Instructions to connect
 * - Connection status
 */

#ifndef SCREEN_SETUP_H
#define SCREEN_SETUP_H

#include <lvgl.h>
#include "ui.h"

/**
 * Create the setup screen
 */
lv_obj_t* screen_setup_create(void);

/**
 * Update setup screen with current state
 */
void screen_setup_update(const ui_state_t* state);

/**
 * Set the AP credentials to display
 */
void screen_setup_set_ap_info(const char* ssid, const char* password, const char* ip);

#endif // SCREEN_SETUP_H

