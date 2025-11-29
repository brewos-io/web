/**
 * BrewOS Home Screen
 * 
 * Main dashboard showing:
 * - Brew temperature with arc gauge
 * - Steam temperature
 * - Pressure bar
 * - Machine status
 * - Connection indicators
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

#endif // SCREEN_HOME_H
