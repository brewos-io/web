/**
 * ECM Pico Firmware - Water Management
 * 
 * Handles water tank auto-fill and water LED control.
 */

#ifndef WATER_MANAGEMENT_H
#define WATER_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Functions
// =============================================================================

// Initialize water management system
void water_management_init(void);

// Update water management (call periodically, e.g., every 100ms)
void water_management_update(void);

// Auto-fill control
void water_management_set_auto_fill(bool enabled);
bool water_management_is_auto_fill_enabled(void);
bool water_management_is_filling(void);

// Manual fill solenoid control (for testing or override)
void water_management_set_fill_solenoid(bool on);

#endif // WATER_MANAGEMENT_H

