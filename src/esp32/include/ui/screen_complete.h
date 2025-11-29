/**
 * BrewOS Shot Complete Screen
 * 
 * Shows shot summary after brewing:
 * - Total time
 * - Final weight
 * - Ratio achieved
 * - Option to save/rate
 */

#ifndef SCREEN_COMPLETE_H
#define SCREEN_COMPLETE_H

#include <lvgl.h>
#include "ui.h"

/**
 * Create the complete screen
 */
lv_obj_t* screen_complete_create(void);

/**
 * Update complete screen with shot data
 */
void screen_complete_update(uint32_t time_ms, float weight, float dose, float avg_flow);

/**
 * Set callback for when user dismisses
 */
typedef void (*complete_dismiss_callback_t)(void);
void screen_complete_set_dismiss_callback(complete_dismiss_callback_t callback);

#endif // SCREEN_COMPLETE_H

