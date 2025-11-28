/**
 * ECM Pico Firmware - PZEM-004T Power Meter Driver
 * 
 * Driver for PZEM-004T AC power meter module using Modbus RTU protocol.
 * 
 * PZEM-004T Specifications:
 * - UART: 9600 baud, 8N1
 * - Protocol: Modbus RTU
 * - Default address: 0x01
 * - Function code: 0x04 (Read Input Registers)
 * 
 * Register Map:
 * - 0x0000: Voltage (V * 10, e.g., 2300 = 230.0V)
 * - 0x0001: Current (A * 100, e.g., 500 = 5.00A)
 * - 0x0002: Power (W, e.g., 1150 = 1150W)
 * - 0x0003: Energy (Wh, e.g., 12345 = 12345Wh)
 * - 0x0004: Frequency (Hz * 10, e.g., 500 = 50.0Hz)
 * - 0x0005: Power Factor (0-100, e.g., 95 = 0.95)
 */

#ifndef PZEM_H
#define PZEM_H

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// PZEM Configuration
// =============================================================================

#define PZEM_DEFAULT_ADDRESS     0x01    // Default Modbus address
#define PZEM_BAUD_RATE           9600    // UART baud rate
#define PZEM_UART_DATA_BITS      8
#define PZEM_UART_STOP_BITS      1
#define PZEM_UART_PARITY         UART_PARITY_NONE

#define PZEM_READ_TIMEOUT_MS     100     // Timeout for reading response
// Note: PZEM is read in sensor cycle, every 10 cycles (500ms at 20Hz sensor rate)

// Modbus function codes
#define MODBUS_READ_INPUT_REGS   0x04

// Register addresses
#define PZEM_REG_VOLTAGE         0x0000
#define PZEM_REG_CURRENT         0x0001
#define PZEM_REG_POWER           0x0002
#define PZEM_REG_ENERGY          0x0003
#define PZEM_REG_FREQUENCY       0x0004
#define PZEM_REG_POWER_FACTOR    0x0005

// =============================================================================
// PZEM Data Structure
// =============================================================================

typedef struct {
    float voltage;          // Voltage in Volts
    float current;          // Current in Amperes
    uint16_t power;         // Power in Watts
    uint32_t energy;        // Energy in Watt-hours (accumulated)
    float frequency;        // Frequency in Hz
    float power_factor;     // Power factor (0.0 to 1.0)
    
    bool valid;             // True if data is valid (successful read)
    uint32_t last_read_ms; // Timestamp of last successful read
    uint32_t error_count;  // Number of read errors
} pzem_data_t;

// =============================================================================
// PZEM Functions
// =============================================================================

/**
 * Initialize PZEM-004T driver
 * @param address Modbus device address (default: 0x01)
 * @return true if initialization successful, false otherwise
 */
bool pzem_init(uint8_t address);

/**
 * Check if PZEM is available (UART pins configured)
 * @return true if PZEM is available, false otherwise
 */
bool pzem_is_available(void);

/**
 * Read power data from PZEM-004T
 * @param data Pointer to pzem_data_t structure to fill
 * @return true if read successful, false on error
 */
bool pzem_read(pzem_data_t* data);

/**
 * Get last read power data (cached)
 * @param data Pointer to pzem_data_t structure to fill
 * @return true if data is valid, false if not available
 */
bool pzem_get_data(pzem_data_t* data);

/**
 * Reset energy counter on PZEM
 * @return true if reset successful, false on error
 */
bool pzem_reset_energy(void);

/**
 * Get error count (for diagnostics)
 * @return Number of read errors since initialization
 */
uint32_t pzem_get_error_count(void);

#endif // PZEM_H

