/**
 * BrewOS Alarm Screen
 * 
 * Full-screen alarm display:
 * - Alarm icon (pulsing)
 * - Alarm message
 * - Instructions
 */

#ifndef SCREEN_ALARM_H
#define SCREEN_ALARM_H

#include <lvgl.h>
#include "ui.h"

/**
 * Create the alarm screen
 */
lv_obj_t* screen_alarm_create(void);

/**
 * Set alarm details
 */
void screen_alarm_set(uint8_t code, const char* message);

/**
 * Clear alarm display
 */
void screen_alarm_clear(void);

#endif // SCREEN_ALARM_H

