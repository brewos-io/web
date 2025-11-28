/**
 * BrewOS Home Screen
 * 
 * Main dashboard showing:
 * - Brew temperature with arc gauge
 * - Steam temperature with arc gauge
 * - Pressure bar
 * - Machine status
 */

#ifndef SCREEN_HOME_H
#define SCREEN_HOME_H

#include <lvgl.h>
#include "ui.h"

/**
 * Create the home screen
 * Returns the screen object
 */
lv_obj_t* screen_home_create(void);

/**
 * Update home screen with current state
 */
void screen_home_update(lv_obj_t* screen, const ui_state_t* state);

/**
 * Get reference to home screen elements for external updates
 */
typedef struct {
    lv_obj_t* screen;
    lv_obj_t* brew_temp_label;
    lv_obj_t* brew_temp_arc;
    lv_obj_t* steam_temp_label;
    lv_obj_t* steam_temp_arc;
    lv_obj_t* pressure_label;
    lv_obj_t* pressure_bar;
    lv_obj_t* status_label;
    lv_obj_t* status_led;
    lv_obj_t* connection_icons;
} screen_home_t;

screen_home_t* screen_home_get_elements(void);

#endif // SCREEN_HOME_H

