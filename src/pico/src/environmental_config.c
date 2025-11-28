/**
 * ECM Pico Firmware - Environmental Configuration Implementation
 * 
 * Manages environmental electrical configuration and runtime electrical state.
 */

#include "environmental_config.h"
#include "machine_electrical.h"

// Current electrical state (computed from machine + environment)
static electrical_state_t g_electrical_state = {0};

// Current environmental config (can be set at compile-time or runtime)
static environmental_electrical_t g_environmental_config = ENVIRONMENTAL_ELECTRICAL_CONFIG;

// =============================================================================
// Initialization
// =============================================================================

void electrical_state_init(electrical_state_t* state, 
                          const machine_electrical_t* machine,
                          const environmental_electrical_t* env) {
    if (!state || !machine || !env) return;
    
    // Copy machine specs
    state->brew_heater_power = machine->brew_heater_power;
    state->steam_heater_power = machine->steam_heater_power;
    
    // Copy environmental config
    state->nominal_voltage = env->nominal_voltage;
    state->max_current_draw = env->max_current_draw;
    
    // Calculate currents
    if (env->nominal_voltage > 0) {
        state->brew_heater_current = (float)machine->brew_heater_power / env->nominal_voltage;
        state->steam_heater_current = (float)machine->steam_heater_power / env->nominal_voltage;
    } else {
        state->brew_heater_current = 0;
        state->steam_heater_current = 0;
    }
    
    // Calculate max combined current with 5% safety margin
    state->max_combined_current = env->max_current_draw * 0.95f;
}

void electrical_state_get(electrical_state_t* state) {
    if (state) {
        // Initialize from current configs if not already initialized
        if (g_electrical_state.nominal_voltage == 0) {
            electrical_state_init(&g_electrical_state, 
                                 &MACHINE_ELECTRICAL_CONFIG,
                                 &g_environmental_config);
        }
        *state = g_electrical_state;
    }
}

// =============================================================================
// Environmental Config Access
// =============================================================================

void environmental_config_set(const environmental_electrical_t* config) {
    if (config) {
        g_environmental_config = *config;
        // Recalculate electrical state
        electrical_state_init(&g_electrical_state,
                             &MACHINE_ELECTRICAL_CONFIG,
                             &g_environmental_config);
    }
}

void environmental_config_get(environmental_electrical_t* config) {
    if (config) {
        *config = g_environmental_config;
    }
}

