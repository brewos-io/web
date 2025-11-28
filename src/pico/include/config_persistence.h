/**
 * ECM Pico Firmware - Configuration Persistence
 * 
 * Saves and loads configuration settings to/from flash storage.
 * 
 * Configuration includes:
 * - Environmental config (voltage, current limits) - REQUIRED
 * - PID settings (Kp, Ki, Kd for brew and steam)
 * - Temperature setpoints (brew, steam)
 * - Heating strategy
 * - Pre-infusion settings
 * - Cleaning mode settings (brew count, threshold)
 * 
 * Runtime state (STATE_HEATING, etc.) is NOT persisted - machine always starts fresh.
 */

#ifndef CONFIG_PERSISTENCE_H
#define CONFIG_PERSISTENCE_H

#include <stdint.h>
#include <stdbool.h>
#include "environmental_config.h"
#include "protocol_defs.h"

// =============================================================================
// Configuration Structure (saved to flash)
// =============================================================================

#define CONFIG_MAGIC          0x45434D43  // "ECMC" - ECM Coffee Machine Config
#define CONFIG_VERSION        1           // Configuration format version

typedef struct __attribute__((packed)) {
    // Magic number and version for validation
    uint32_t magic;                      // Must be CONFIG_MAGIC
    uint32_t version;                    // Configuration format version
    
    // Environmental config (REQUIRED - machine disabled if invalid)
    environmental_electrical_t environmental;
    
    // PID settings
    struct {
        float kp, ki, kd;                // Brew PID
    } pid_brew;
    struct {
        float kp, ki, kd;                // Steam PID
    } pid_steam;
    
    // Temperature setpoints (Celsius * 10)
    int16_t brew_setpoint;
    int16_t steam_setpoint;
    
    // Heating strategy
    uint8_t heating_strategy;            // HEAT_STRATEGY_* enum
    
    // Pre-infusion settings
    bool preinfusion_enabled;
    uint16_t preinfusion_on_ms;
    uint16_t preinfusion_pause_ms;
    
    // Cleaning mode settings
    uint16_t cleaning_brew_count;          // Brew counter (persists across reboots)
    uint16_t cleaning_threshold;          // Cleaning reminder threshold (10-200)
    
    // Reserved for future use
    uint8_t reserved[28];
    
    // CRC32 for integrity check
    uint32_t crc32;
} persisted_config_t;

// =============================================================================
// Functions
// =============================================================================

/**
 * Initialize configuration persistence
 * Loads configuration from flash, validates, and applies defaults if needed
 * 
 * @return true if environmental config is valid (machine can operate)
 * @return false if environmental config is invalid (machine disabled)
 */
bool config_persistence_init(void);

/**
 * Check if environmental config is valid
 * Machine is disabled if environmental config is not set
 */
bool config_persistence_is_env_valid(void);

/**
 * Save all configuration to flash
 * @return true on success, false on failure
 */
bool config_persistence_save(void);

/**
 * Load configuration from flash
 * @return true if valid config loaded, false if invalid or not found
 */
bool config_persistence_load(void);

/**
 * Get current persisted configuration
 */
void config_persistence_get(persisted_config_t* config);

/**
 * Set configuration (does not save to flash - call save() after)
 */
void config_persistence_set(const persisted_config_t* config);

/**
 * Reset to defaults (does not save - call save() after)
 * Environmental config is NOT reset (must be set manually)
 */
void config_persistence_reset_to_defaults(void);

/**
 * Check if we're in setup mode (environmental config not set)
 */
bool config_persistence_is_setup_mode(void);

/**
 * Save cleaning mode settings (brew count and threshold) to flash
 * This is a convenience function that updates only cleaning settings
 * @return true on success, false on failure
 */
bool config_persistence_save_cleaning(uint16_t brew_count, uint16_t threshold);

/**
 * Get cleaning mode settings from persisted config
 * @param brew_count Output parameter for brew count
 * @param threshold Output parameter for threshold
 */
void config_persistence_get_cleaning(uint16_t* brew_count, uint16_t* threshold);

#endif // CONFIG_PERSISTENCE_H

