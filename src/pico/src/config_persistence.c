/**
 * ECM Pico Firmware - Configuration Persistence Implementation
 * 
 * Saves and loads configuration to/from flash storage using Pico SDK.
 * Uses a reserved flash sector for configuration storage.
 */

#include "config_persistence.h"
#include "config.h"
#include "environmental_config.h"
#include "control.h"
#include "state.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include <string.h>
#include <stddef.h>  // for offsetof

// XIP_BASE is the base address for execute-in-place flash
#ifndef XIP_BASE
#define XIP_BASE 0x10000000
#endif

// =============================================================================
// Flash Storage Configuration
// =============================================================================

// Use last flash sector for configuration (2MB flash = 512 sectors, use sector 511)
// Each sector is 4KB, we only need ~200 bytes, but must erase entire sector
// PICO_FLASH_SIZE_BYTES and FLASH_SECTOR_SIZE are defined in hardware/flash.h
#define CONFIG_FLASH_OFFSET   (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define CONFIG_FLASH_SECTOR   (CONFIG_FLASH_OFFSET / FLASH_SECTOR_SIZE)

// Current configuration in RAM
static persisted_config_t g_persisted_config = {0};
static bool g_config_loaded = false;
static bool g_env_valid = false;

// =============================================================================
// CRC32 Calculation
// =============================================================================

static uint32_t crc32_calculate(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    const uint32_t polynomial = 0xEDB88320;
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return ~crc;
}

// =============================================================================
// Default Configuration
// =============================================================================

static void set_defaults(persisted_config_t* config) {
    if (!config) return;
    
    memset(config, 0, sizeof(persisted_config_t));
    
    config->magic = CONFIG_MAGIC;
    config->version = CONFIG_VERSION;
    
    // Environmental config: NOT set by default (must be configured)
    config->environmental.nominal_voltage = 0;
    config->environmental.max_current_draw = 0.0f;
    
    // PID defaults (from config.h)
    config->pid_brew.kp = PID_DEFAULT_KP;
    config->pid_brew.ki = PID_DEFAULT_KI;
    config->pid_brew.kd = PID_DEFAULT_KD;
    
    config->pid_steam.kp = PID_DEFAULT_KP;
    config->pid_steam.ki = PID_DEFAULT_KI;
    config->pid_steam.kd = PID_DEFAULT_KD;
    
    // Temperature setpoints (from config.h)
    config->brew_setpoint = DEFAULT_BREW_TEMP;
    config->steam_setpoint = DEFAULT_STEAM_TEMP;
    
    // Heating strategy default
    config->heating_strategy = 1;  // HEAT_SEQUENTIAL (default safe strategy)
    
    // Pre-infusion defaults
    config->preinfusion_enabled = false;
    config->preinfusion_on_ms = 3000;
    config->preinfusion_pause_ms = 5000;
    
    // Cleaning mode defaults
    config->cleaning_brew_count = 0;
    config->cleaning_threshold = 100;  // Default: 100 cycles before reminder
}

// =============================================================================
// Flash Operations
// =============================================================================

static bool flash_read_config(persisted_config_t* config) {
    if (!config) return false;
    
    // Read from flash into RAM buffer
    // We can't read directly from XIP while executing, so we use a local buffer
    persisted_config_t flash_config;
    
    // Read from flash using flash_read (reads into RAM)
    // Note: flash_read requires the data to be in a RAM buffer
    // We'll read directly from XIP address (safe for read-only access)
    const uint8_t* flash_addr = (const uint8_t*)(XIP_BASE + CONFIG_FLASH_OFFSET);
    memcpy(&flash_config, flash_addr, sizeof(persisted_config_t));
    
    // Validate magic number
    if (flash_config.magic != CONFIG_MAGIC) {
        return false;  // No valid config in flash
    }
    
    // Validate version
    if (flash_config.version != CONFIG_VERSION) {
        return false;  // Wrong version
    }
    
    // Calculate CRC32 (excluding CRC field itself)
    size_t crc_size = offsetof(persisted_config_t, crc32);
    uint32_t calculated_crc = crc32_calculate((const uint8_t*)&flash_config, crc_size);
    
    if (calculated_crc != flash_config.crc32) {
        return false;  // CRC mismatch - data corrupted
    }
    
    // Copy to output
    *config = flash_config;
    return true;
}

static bool flash_write_config(const persisted_config_t* config) {
    if (!config) return false;
    
    // Calculate CRC32 before saving
    persisted_config_t config_with_crc = *config;
    size_t crc_size = offsetof(persisted_config_t, crc32);
    config_with_crc.crc32 = crc32_calculate((const uint8_t*)&config_with_crc, crc_size);
    
    // Disable interrupts during flash write
    uint32_t ints = save_and_disable_interrupts();
    
    // Erase sector (must erase before writing)
    flash_range_erase(CONFIG_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    
    // Write configuration (must be 256-byte aligned, pad if needed)
    uint8_t write_buffer[FLASH_PAGE_SIZE];
    memset(write_buffer, 0xFF, FLASH_PAGE_SIZE);
    memcpy(write_buffer, &config_with_crc, sizeof(persisted_config_t));
    
    flash_range_program(CONFIG_FLASH_OFFSET, write_buffer, FLASH_PAGE_SIZE);
    
    restore_interrupts(ints);
    
    return true;
}

// =============================================================================
// Validation
// =============================================================================

static bool validate_environmental_config(const environmental_electrical_t* env) {
    if (!env) return false;
    
    // Environmental config is REQUIRED
    // Must have valid voltage (100-250V) and current limit (>0)
    if (env->nominal_voltage < 100 || env->nominal_voltage > 250) {
        return false;
    }
    
    if (env->max_current_draw <= 0.0f || env->max_current_draw > 50.0f) {
        return false;
    }
    
    return true;
}

// =============================================================================
// Public Functions
// =============================================================================

bool config_persistence_init(void) {
    // Try to load from flash
    if (config_persistence_load()) {
        // Validate environmental config
        g_env_valid = validate_environmental_config(&g_persisted_config.environmental);
        
        if (g_env_valid) {
            // Apply loaded configuration
            environmental_config_set(&g_persisted_config.environmental);
            
            // Apply PID settings
            control_set_pid(0, g_persisted_config.pid_brew.kp, 
                           g_persisted_config.pid_brew.ki, 
                           g_persisted_config.pid_brew.kd);
            control_set_pid(1, g_persisted_config.pid_steam.kp, 
                           g_persisted_config.pid_steam.ki, 
                           g_persisted_config.pid_steam.kd);
            
            // Apply setpoints
            control_set_setpoint(0, g_persisted_config.brew_setpoint);
            control_set_setpoint(1, g_persisted_config.steam_setpoint);
            
            // Apply heating strategy (validate against electrical limits)
            if (!control_set_heating_strategy(g_persisted_config.heating_strategy)) {
                // Strategy not allowed - fall back to safe default
                DEBUG_PRINT("Config: Saved heating strategy %d not allowed, using HEAT_SEQUENTIAL\n", 
                           g_persisted_config.heating_strategy);
                if (!control_set_heating_strategy(1)) {  // HEAT_SEQUENTIAL = 1
                    // Even sequential not allowed - use brew only
                    control_set_heating_strategy(0);  // HEAT_BREW_ONLY = 0
                }
            }
            
            // Apply pre-infusion
            state_set_preinfusion(g_persisted_config.preinfusion_enabled,
                                 g_persisted_config.preinfusion_on_ms,
                                 g_persisted_config.preinfusion_pause_ms);
            
            // Cleaning mode values are loaded but not applied here
            // They are applied in cleaning_init() via config_persistence_get_cleaning()
            
            DEBUG_PRINT("Config: Loaded from flash (env valid)\n");
            return true;  // Machine can operate
        } else {
            DEBUG_PRINT("Config: Loaded from flash but environmental config invalid\n");
            return false;  // Machine disabled - environmental config invalid
        }
    } else {
        // No valid config in flash - use defaults
        set_defaults(&g_persisted_config);
        g_config_loaded = false;
        g_env_valid = false;
        
        DEBUG_PRINT("Config: No valid config in flash, using defaults\n");
        DEBUG_PRINT("Config: Machine disabled - environmental config required\n");
        return false;  // Machine disabled - environmental config not set
    }
}

bool config_persistence_is_env_valid(void) {
    return g_env_valid;
}

bool config_persistence_save(void) {
    // Update environmental config from current state
    environmental_config_get(&g_persisted_config.environmental);
    
    // Update PID settings from control module
    control_get_pid(0, &g_persisted_config.pid_brew.kp, 
                    &g_persisted_config.pid_brew.ki, 
                    &g_persisted_config.pid_brew.kd);
    control_get_pid(1, &g_persisted_config.pid_steam.kp, 
                    &g_persisted_config.pid_steam.ki, 
                    &g_persisted_config.pid_steam.kd);
    
    // Update setpoints
    g_persisted_config.brew_setpoint = control_get_setpoint(0);
    g_persisted_config.steam_setpoint = control_get_setpoint(1);
    
    // Update heating strategy
    g_persisted_config.heating_strategy = control_get_heating_strategy();
    
    // Get pre-infusion settings from state module
    state_get_preinfusion(&g_persisted_config.preinfusion_enabled,
                          &g_persisted_config.preinfusion_on_ms,
                          &g_persisted_config.preinfusion_pause_ms);
    
    // Cleaning mode settings are updated via config_persistence_save_cleaning()
    // They are already in g_persisted_config when this function is called
    
    // Ensure magic and version are set
    g_persisted_config.magic = CONFIG_MAGIC;
    g_persisted_config.version = CONFIG_VERSION;
    
    // Save to flash
    if (flash_write_config(&g_persisted_config)) {
        g_config_loaded = true;
        DEBUG_PRINT("Config: Saved to flash\n");
        return true;
    }
    
    DEBUG_PRINT("Config: Failed to save to flash\n");
    return false;
}

bool config_persistence_load(void) {
    persisted_config_t loaded_config;
    
    if (flash_read_config(&loaded_config)) {
        g_persisted_config = loaded_config;
        g_config_loaded = true;
        return true;
    }
    
    return false;
}

void config_persistence_get(persisted_config_t* config) {
    if (config) {
        *config = g_persisted_config;
    }
}

void config_persistence_set(const persisted_config_t* config) {
    if (config) {
        g_persisted_config = *config;
        g_config_loaded = true;
        
        // Validate environmental config
        g_env_valid = validate_environmental_config(&g_persisted_config.environmental);
    }
}

void config_persistence_reset_to_defaults(void) {
    // Save environmental config (don't reset it)
    environmental_electrical_t saved_env = g_persisted_config.environmental;
    
    // Set defaults
    set_defaults(&g_persisted_config);
    
    // Restore environmental config
    g_persisted_config.environmental = saved_env;
    
    // Revalidate
    g_env_valid = validate_environmental_config(&g_persisted_config.environmental);
}

bool config_persistence_is_setup_mode(void) {
    return !g_env_valid;
}

bool config_persistence_save_cleaning(uint16_t brew_count, uint16_t threshold) {
    // Update cleaning values in persisted config
    g_persisted_config.cleaning_brew_count = brew_count;
    g_persisted_config.cleaning_threshold = threshold;
    
    // Ensure magic and version are set
    g_persisted_config.magic = CONFIG_MAGIC;
    g_persisted_config.version = CONFIG_VERSION;
    
    // Save to flash
    if (flash_write_config(&g_persisted_config)) {
        g_config_loaded = true;
        DEBUG_PRINT("Config: Saved cleaning settings (brew_count=%d, threshold=%d)\n", 
                   brew_count, threshold);
        return true;
    }
    
    DEBUG_PRINT("Config: Failed to save cleaning settings to flash\n");
    return false;
}

void config_persistence_get_cleaning(uint16_t* brew_count, uint16_t* threshold) {
    if (brew_count) {
        *brew_count = g_persisted_config.cleaning_brew_count;
    }
    if (threshold) {
        *threshold = g_persisted_config.cleaning_threshold;
    }
}

