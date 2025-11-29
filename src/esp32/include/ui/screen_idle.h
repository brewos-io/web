/**
 * BrewOS Idle Screen
 * 
 * Machine off/standby screen showing:
 * - Current heating strategy
 * - Power on prompt
 * - Strategy selection via rotary encoder
 */

#ifndef SCREEN_IDLE_H
#define SCREEN_IDLE_H

#include <lvgl.h>
#include "ui.h"

/**
 * Create the idle screen
 */
lv_obj_t* screen_idle_create(void);

/**
 * Update idle screen with current state
 */
void screen_idle_update(const ui_state_t* state);

/**
 * Set the selected strategy index (from encoder rotation)
 */
void screen_idle_select_strategy(int index);

/**
 * Get the currently selected strategy
 */
heating_strategy_t screen_idle_get_selected_strategy(void);

/**
 * Callback type for turn on action
 */
typedef void (*idle_turn_on_callback_t)(heating_strategy_t strategy);

/**
 * Set callback for turn on action
 */
void screen_idle_set_turn_on_callback(idle_turn_on_callback_t callback);

#endif // SCREEN_IDLE_H

