/**
 * ECM Coffee Machine Controller - Pico Firmware
 * 
 * Main entry point for the RP2040-based control board.
 * 
 * Core 0: Real-time control loop (safety, sensors, PID, outputs)
 * Core 1: Communication with ESP32
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#include "config.h"
#include "protocol.h"
#include "safety.h"
#include "state.h"
#include "sensors.h"
#include "control.h"
#include "environmental_config.h"
#include "machine_electrical.h"
#include "machine_config.h"
#include "pcb_config.h"
#include "gpio_init.h"
#include "hardware.h"
#include "water_management.h"
#include "config_persistence.h"
#include "cleaning.h"
#include "statistics.h"
#include "bootloader.h"
#include "pzem.h"

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------
static volatile bool g_core1_ready = false;
static uint32_t g_boot_time = 0;

// Status payload (updated by control loop, read by comms)
static volatile status_payload_t g_status;
static volatile bool g_status_updated = false;

// -----------------------------------------------------------------------------
// Core 1 Entry Point (Communication)
// -----------------------------------------------------------------------------
void core1_main(void) {
    DEBUG_PRINT("Core 1: Starting communication loop\n");
    
    // Initialize protocol
    protocol_init();
    
    // Send boot message
    protocol_send_boot();
    
    // Send environmental config after boot
    env_config_payload_t env_resp;
    environmental_electrical_t env;
    environmental_config_get(&env);
    electrical_state_t state;
    electrical_state_get(&state);
    
    env_resp.nominal_voltage = env.nominal_voltage;
    env_resp.max_current_draw = env.max_current_draw;
    env_resp.brew_heater_current = state.brew_heater_current;
    env_resp.steam_heater_current = state.steam_heater_current;
    env_resp.max_combined_current = state.max_combined_current;
    protocol_send_env_config(&env_resp);
    
    // Signal ready
    g_core1_ready = true;
    
    uint32_t last_status_send = 0;
    
    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        // Process incoming packets
        protocol_process();
        
        // Send status periodically
        if (now - last_status_send >= STATUS_SEND_PERIOD_MS) {
            last_status_send = now;
            
            if (g_status_updated) {
                status_payload_t status_copy;
                
                // Quick copy of status (minimize time with interrupts consideration)
                memcpy((void*)&status_copy, (const void*)&g_status, sizeof(status_payload_t));
                
                protocol_send_status(&status_copy);
            }
        }
        
        // Small sleep to not hog CPU
        sleep_us(100);
    }
}

// -----------------------------------------------------------------------------
// Packet Handler (called from Core 1)
// -----------------------------------------------------------------------------
void handle_packet(const packet_t* packet) {
    DEBUG_PRINT("RX packet: type=0x%02X len=%d\n", packet->type, packet->length);
    
    // Register heartbeat from ESP32
    safety_esp32_heartbeat();
    
    switch (packet->type) {
        case MSG_PING: {
            // Respond with ACK
            protocol_send_ack(MSG_PING, packet->seq, ACK_SUCCESS);
            break;
        }
        
        case MSG_CMD_SET_TEMP: {
            if (packet->length >= sizeof(cmd_set_temp_t)) {
                cmd_set_temp_t* cmd = (cmd_set_temp_t*)packet->payload;
                control_set_setpoint(cmd->target, cmd->temperature);
                protocol_send_ack(MSG_CMD_SET_TEMP, packet->seq, ACK_SUCCESS);
            } else {
                protocol_send_ack(MSG_CMD_SET_TEMP, packet->seq, ACK_ERROR_INVALID);
            }
            break;
        }
        
        case MSG_CMD_SET_PID: {
            if (packet->length >= sizeof(cmd_set_pid_t)) {
                cmd_set_pid_t* cmd = (cmd_set_pid_t*)packet->payload;
                control_set_pid(cmd->target, cmd->kp / 100.0f, cmd->ki / 100.0f, cmd->kd / 100.0f);
                protocol_send_ack(MSG_CMD_SET_PID, packet->seq, ACK_SUCCESS);
            } else {
                protocol_send_ack(MSG_CMD_SET_PID, packet->seq, ACK_ERROR_INVALID);
            }
            break;
        }
        
        case MSG_CMD_BREW: {
            if (packet->length >= sizeof(cmd_brew_t)) {
                cmd_brew_t* cmd = (cmd_brew_t*)packet->payload;
                if (cmd->action) {
                    state_start_brew();
                } else {
                    state_stop_brew();
                }
                protocol_send_ack(MSG_CMD_BREW, packet->seq, ACK_SUCCESS);
            } else {
                protocol_send_ack(MSG_CMD_BREW, packet->seq, ACK_ERROR_INVALID);
            }
            break;
        }
        
        case MSG_CMD_MODE: {
            if (packet->length >= sizeof(cmd_mode_t)) {
                cmd_mode_t* cmd = (cmd_mode_t*)packet->payload;
                machine_mode_t mode = (machine_mode_t)cmd->mode;
                if (mode <= MODE_STEAM) {  // Valid modes: MODE_IDLE, MODE_BREW, MODE_STEAM
                    if (state_set_mode(mode)) {
                        protocol_send_ack(MSG_CMD_MODE, packet->seq, ACK_SUCCESS);
                    } else {
                        // Mode change rejected (e.g., brewing in progress)
                        protocol_send_ack(MSG_CMD_MODE, packet->seq, ACK_ERROR_REJECTED);
                    }
                } else {
                    protocol_send_ack(MSG_CMD_MODE, packet->seq, ACK_ERROR_INVALID);
                }
            } else {
                protocol_send_ack(MSG_CMD_MODE, packet->seq, ACK_ERROR_INVALID);
            }
            break;
        }
        
        case MSG_CMD_GET_CONFIG: {
            config_payload_t config;
            control_get_config(&config);
            protocol_send_config(&config);
            break;
        }
        
        case MSG_CMD_CONFIG: {
            // Variable payload based on config_type
            if (packet->length >= 1) {
                uint8_t config_type = packet->payload[0];
                
                if (config_type == CONFIG_ENVIRONMENTAL) {
                    // Set environmental configuration
                    if (packet->length >= sizeof(config_environmental_t) + 1) {
                        config_environmental_t* env_cmd = (config_environmental_t*)(&packet->payload[1]);
                        environmental_electrical_t env_config = {
                            .nominal_voltage = env_cmd->nominal_voltage,
                            .max_current_draw = env_cmd->max_current_draw
                        };
                        environmental_config_set(&env_config);
                        
                        // Save configuration to flash
                        persisted_config_t config;
                        config_persistence_get(&config);
                        config.environmental = env_config;
                        config_persistence_set(&config);
                        config_persistence_save();
                        
                        DEBUG_PRINT("Environmental config saved: %dV, %.1fA\n",
                                   env_config.nominal_voltage, env_config.max_current_draw);
                        
                        // Send ACK
                        protocol_send_ack(MSG_CMD_CONFIG, packet->seq, ACK_SUCCESS);
                        
                        // Send updated environmental config
                        env_config_payload_t env_resp;
                        environmental_electrical_t env;
                        environmental_config_get(&env);
                        electrical_state_t state;
                        electrical_state_get(&state);
                        
                        env_resp.nominal_voltage = env.nominal_voltage;
                        env_resp.max_current_draw = env.max_current_draw;
                        env_resp.brew_heater_current = state.brew_heater_current;
                        env_resp.steam_heater_current = state.steam_heater_current;
                        env_resp.max_combined_current = state.max_combined_current;
                        protocol_send_env_config(&env_resp);
                    } else {
                        protocol_send_ack(MSG_CMD_CONFIG, packet->seq, ACK_ERROR_INVALID);
                    }
                } else {
                    // Other config types handled elsewhere
                    protocol_send_ack(MSG_CMD_CONFIG, packet->seq, ACK_SUCCESS);
                }
            }
            break;
        }
        
        case MSG_CMD_GET_ENV_CONFIG: {
            // Request environmental configuration
            env_config_payload_t env_resp;
            environmental_electrical_t env;
            environmental_config_get(&env);
            electrical_state_t state;
            electrical_state_get(&state);
            
            env_resp.nominal_voltage = env.nominal_voltage;
            env_resp.max_current_draw = env.max_current_draw;
            env_resp.brew_heater_current = state.brew_heater_current;
            env_resp.steam_heater_current = state.steam_heater_current;
            env_resp.max_combined_current = state.max_combined_current;
            protocol_send_env_config(&env_resp);
            break;
        }
        
        case MSG_CMD_CLEANING_START: {
            // Start cleaning cycle
            if (cleaning_start_cycle()) {
                protocol_send_ack(MSG_CMD_CLEANING_START, packet->seq, ACK_SUCCESS);
            } else {
                protocol_send_ack(MSG_CMD_CLEANING_START, packet->seq, ACK_ERROR_REJECTED);
            }
            break;
        }
        
        case MSG_CMD_CLEANING_STOP: {
            // Stop cleaning cycle
            cleaning_stop_cycle();
            protocol_send_ack(MSG_CMD_CLEANING_STOP, packet->seq, ACK_SUCCESS);
            break;
        }
        
        case MSG_CMD_CLEANING_RESET: {
            // Reset brew counter
            cleaning_reset_brew_count();
            protocol_send_ack(MSG_CMD_CLEANING_RESET, packet->seq, ACK_SUCCESS);
            break;
        }
        
        case MSG_CMD_CLEANING_SET_THRESHOLD: {
            // Set cleaning reminder threshold
            if (packet->length >= 2) {
                uint16_t threshold = (packet->payload[0] << 8) | packet->payload[1];
                if (cleaning_set_threshold(threshold)) {
                    protocol_send_ack(MSG_CMD_CLEANING_SET_THRESHOLD, packet->seq, ACK_SUCCESS);
                } else {
                    protocol_send_ack(MSG_CMD_CLEANING_SET_THRESHOLD, packet->seq, ACK_ERROR_INVALID);
                }
            } else {
                protocol_send_ack(MSG_CMD_CLEANING_SET_THRESHOLD, packet->seq, ACK_ERROR_INVALID);
            }
            break;
        }
        
        case MSG_CMD_BOOTLOADER: {
            // Enter bootloader mode and receive firmware
            DEBUG_PRINT("Entering bootloader mode!\n");
            protocol_send_ack(MSG_CMD_BOOTLOADER, packet->seq, ACK_SUCCESS);
            
            // Small delay to ensure ACK is sent
            sleep_ms(50);
            
            // Enter bootloader mode (does not return on success)
            bootloader_result_t result = bootloader_receive_firmware();
            
            // If we get here, bootloader failed
            DEBUG_PRINT("Bootloader error: %s\n", bootloader_get_status_message(result));
            
            // Send error ACK with appropriate error code
            uint8_t ack_result = ACK_ERROR_FAILED;
            if (result == BOOTLOADER_ERROR_TIMEOUT) {
                ack_result = ACK_ERROR_TIMEOUT;
            }
            // Note: Can't send ACK here as we're back in normal protocol mode
            // Send error message via debug instead
            char error_msg[64];
            snprintf(error_msg, sizeof(error_msg), "Bootloader failed: %s", 
                     bootloader_get_status_message(result));
            protocol_send_debug(error_msg);
            break;
        }
        
        default:
            DEBUG_PRINT("Unknown packet type: 0x%02X\n", packet->type);
            break;
    }
}

// -----------------------------------------------------------------------------
// Core 0 Entry Point (Control)
// -----------------------------------------------------------------------------
int main(void) {
    // Record boot time
    g_boot_time = to_ms_since_boot(get_absolute_time());
    
    // Initialize stdio (USB for debugging)
    stdio_init_all();
    sleep_ms(100);  // Brief delay for USB
    
    DEBUG_PRINT("\n========================================\n");
    DEBUG_PRINT("ECM Pico Controller v%d.%d.%d\n", 
                FIRMWARE_VERSION_MAJOR, 
                FIRMWARE_VERSION_MINOR, 
                FIRMWARE_VERSION_PATCH);
    DEBUG_PRINT("========================================\n");
    
    // Initialize hardware abstraction layer
    if (!hw_init()) {
        DEBUG_PRINT("ERROR: Failed to initialize hardware abstraction layer\n");
        // Continue anyway, but hardware may not work correctly
    } else {
        DEBUG_PRINT("Hardware: %s mode\n", hw_is_simulation_mode() ? "SIMULATION" : "REAL");
    }
    
    // Initialize PCB configuration and GPIO
    if (!gpio_init_all()) {
        DEBUG_PRINT("ERROR: Failed to initialize GPIO (invalid PCB config)\n");
        // Continue anyway, but GPIO may not work correctly
    } else {
        const pcb_config_t* pcb = pcb_config_get();
        if (pcb) {
            DEBUG_PRINT("PCB: %s v%d.%d.%d\n",
                       pcb->name,
                       pcb->version.major,
                       pcb->version.minor,
                       pcb->version.patch);
        }
    }
    
    // Log machine configuration (lazy initialized on first access)
    const machine_features_t* features = machine_get_features();
    if (features) {
        DEBUG_PRINT("Machine: %s (%s)\n", features->name, features->description);
        DEBUG_PRINT("  Type: %s\n", 
            features->type == MACHINE_TYPE_DUAL_BOILER ? "Dual Boiler" :
            features->type == MACHINE_TYPE_SINGLE_BOILER ? "Single Boiler" :
            features->type == MACHINE_TYPE_HEAT_EXCHANGER ? "Heat Exchanger" : "Unknown");
        DEBUG_PRINT("  Boilers: %d, SSRs: %d\n", features->num_boilers, features->num_ssrs);
        DEBUG_PRINT("  Sensors: brew_ntc=%d steam_ntc=%d group_tc=%d\n",
                   features->has_brew_ntc, features->has_steam_ntc, features->has_group_thermocouple);
    } else {
        DEBUG_PRINT("ERROR: Machine configuration not available!\n");
    }
    
    // SAF-001: Enable watchdog immediately after GPIO initialization
    watchdog_enable(SAFETY_WATCHDOG_TIMEOUT_MS, true);
    DEBUG_PRINT("Watchdog enabled (%dms timeout)\n", SAFETY_WATCHDOG_TIMEOUT_MS);
    
    // Check reset reason
    if (watchdog_caused_reboot()) {
        DEBUG_PRINT("WARNING: Watchdog reset!\n");
        // SAF-004: On watchdog timeout, outputs are already OFF from gpio_init_outputs()
        // which sets all outputs to 0 (safe state) on boot
    }
    
    // Initialize safety system FIRST
    safety_init();
    DEBUG_PRINT("Safety system initialized\n");
    
    // Initialize sensors
    sensors_init();
    DEBUG_PRINT("Sensors initialized\n");
    
    // Initialize configuration persistence (loads from flash)
    bool env_valid = config_persistence_init();
    if (!env_valid) {
        DEBUG_PRINT("ERROR: Environmental configuration not set - machine disabled\n");
        DEBUG_PRINT("ERROR: Please configure voltage and current limits via ESP32\n");
        // Machine will remain in safe state until environmental config is set
    } else {
        electrical_state_t elec_state;
        electrical_state_get(&elec_state);
        DEBUG_PRINT("Electrical: %dV, %dW brew, %dW steam, %.1fA max\n",
                    elec_state.nominal_voltage,
                    elec_state.brew_heater_power,
                    elec_state.steam_heater_power,
                    elec_state.max_current_draw);
    }
    
    // Initialize control
    control_init();
    DEBUG_PRINT("Control initialized\n");
    
    // Initialize state machine
    state_init();
    DEBUG_PRINT("State machine initialized\n");
    
    // Initialize water management
    water_management_init();
    DEBUG_PRINT("Water management initialized\n");
    
    // Initialize cleaning mode
    cleaning_init();
    DEBUG_PRINT("Cleaning mode initialized\n");
    
    // Initialize statistics
    statistics_init();
    DEBUG_PRINT("Statistics initialized\n");
    
    // Launch Core 1 for communication
    multicore_launch_core1(core1_main);
    DEBUG_PRINT("Core 1 launched\n");
    
    // Wait for Core 1 to be ready
    while (!g_core1_ready) {
        sleep_ms(1);
    }
    
    // Set up packet handler
    protocol_set_callback(handle_packet);
    
    DEBUG_PRINT("Entering main control loop\n");
    
    // Timing
    uint32_t last_control = 0;
    uint32_t last_sensor = 0;
    uint32_t last_water = 0;
    
    // Main control loop (Core 0)
    while (true) {
        uint32_t now = to_ms_since_boot(get_absolute_time());
        
        // Read sensors (20Hz)
        if (now - last_sensor >= SENSOR_READ_PERIOD_MS) {
            last_sensor = now;
            sensors_read();
        }
        
        // Update water management (10Hz)
        if (now - last_water >= 100) {
            last_water = now;
            water_management_update();
        }
        
        // Update cleaning mode (10Hz)
        cleaning_update();
        
        // Update statistics time-based calculations (every hour)
        static uint32_t last_stats_update = 0;
        if (now - last_stats_update >= 3600000) {  // 1 hour
            last_stats_update = now;
            statistics_update_time_based();
        }
        
        // Control loop (10Hz)
        if (now - last_control >= CONTROL_LOOP_PERIOD_MS) {
            last_control = now;
            
            // Check safety first
            safety_state_t safety = safety_check();
            
            if (safety == SAFETY_CRITICAL) {
                // Enter safe state - all outputs off
                safety_enter_safe_state();
                protocol_send_alarm(safety_get_last_alarm(), 2, 0);
            } else if (safety == SAFETY_FAULT) {
                // Warning condition - may continue with limits
                protocol_send_alarm(safety_get_last_alarm(), 1, 0);
            }
            
            // SAF-003: Feed watchdog only from main control loop after safety checks pass
            // (safety checks have been executed - feed watchdog to indicate loop is running)
            safety_kick_watchdog();
            
            // Update state machine
            state_update();
            
            // Run control (PID, outputs)
            if (!safety_is_safe_state()) {
                control_update();
            }
            
            // Update status for Core 1
            sensor_data_t sensor_data;
            sensors_get_data(&sensor_data);
            
            control_outputs_t outputs;
            control_get_outputs(&outputs);
            
            // Machine-type aware status population
            // HX machines: brew_temp is invalid (no brew NTC), use group_temp
            // Single boiler: steam_temp is invalid (no steam NTC)
            const machine_features_t* machine_features = machine_get_features();
            
            if (machine_features && machine_features->type == MACHINE_TYPE_HEAT_EXCHANGER) {
                // HX: brew_temp not available, report group_temp as brew indicator
                g_status.brew_temp = sensor_data.group_temp;  // Use group as brew proxy
                g_status.steam_temp = sensor_data.steam_temp;
                g_status.group_temp = sensor_data.group_temp;
            } else if (machine_features && machine_features->type == MACHINE_TYPE_SINGLE_BOILER) {
                // Single boiler: use brew NTC for both (same physical sensor)
                g_status.brew_temp = sensor_data.brew_temp;
                g_status.steam_temp = sensor_data.brew_temp;  // Same as brew for display
                g_status.group_temp = sensor_data.group_temp;
            } else {
                // Dual boiler: all sensors independent
                g_status.brew_temp = sensor_data.brew_temp;
                g_status.steam_temp = sensor_data.steam_temp;
                g_status.group_temp = sensor_data.group_temp;
            }
            
            g_status.pressure = sensor_data.pressure;
            g_status.brew_setpoint = control_get_setpoint(0);
            g_status.steam_setpoint = control_get_setpoint(1);
            g_status.brew_output = outputs.brew_heater;
            g_status.steam_output = outputs.steam_heater;
            g_status.pump_output = outputs.pump;
            g_status.state = state_get();
            g_status.water_level = sensor_data.water_level;
            g_status.power_watts = outputs.power_watts;
            g_status.uptime_ms = now;
            g_status.shot_start_timestamp_ms = state_get_brew_start_timestamp_ms();
            
            // Set flags
            g_status.flags = 0;
            if (state_is_brewing()) g_status.flags |= STATUS_FLAG_BREWING;
            if (outputs.pump > 0) g_status.flags |= STATUS_FLAG_PUMP_ON;
            if (outputs.brew_heater > 0 || outputs.steam_heater > 0) g_status.flags |= STATUS_FLAG_HEATING;
            if (sensor_data.water_level < SAFETY_MIN_WATER_LEVEL) g_status.flags |= STATUS_FLAG_WATER_LOW;
            if (safety_is_safe_state()) g_status.flags |= STATUS_FLAG_ALARM;
            
            g_status_updated = true;
        }
        
        // Small sleep
        sleep_us(100);
    }
    
    return 0;
}

