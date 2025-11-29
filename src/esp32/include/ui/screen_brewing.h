/**
 * BrewOS Brewing Screen
 * 
 * Active brewing display showing:
 * - Shot timer (large, central)
 * - Weight progress arc
 * - Flow rate
 * - Real-time updates
 */

#ifndef SCREEN_BREWING_H
#define SCREEN_BREWING_H

#include <lvgl.h>
#include "ui.h"

/**
 * Create the brewing screen
 */
lv_obj_t* screen_brewing_create(void);

/**
 * Update brewing screen with current state
 */
void screen_brewing_update(const ui_state_t* state);

/**
 * Reset brewing screen (for new shot)
 */
void screen_brewing_reset(void);

#endif // SCREEN_BREWING_H

