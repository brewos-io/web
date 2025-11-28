/**
 * ECM Pico Firmware - PZEM-004T Power Meter Driver Implementation
 * 
 * Implements Modbus RTU protocol for reading power data from PZEM-004T.
 */

#include "pzem.h"
#include "pcb_config.h"
#include "hardware.h"
#include "config.h"  // For DEBUG_PRINT
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <string.h>
#include <math.h>

// =============================================================================
// Private State
// =============================================================================

static bool g_pzem_initialized = false;
static uint8_t g_pzem_address = PZEM_DEFAULT_ADDRESS;
static uart_inst_t* g_pzem_uart = NULL;
static uint8_t g_pzem_uart_num = 0;
static pzem_data_t g_pzem_data = {0};
static uint32_t g_pzem_last_update = 0;
static uint32_t g_pzem_error_count = 0;

// =============================================================================
// Modbus CRC16 Calculation
// =============================================================================

/**
 * Calculate Modbus CRC16
 * @param data Data buffer
 * @param length Data length
 * @return CRC16 value
 */
static uint16_t modbus_crc16(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

// =============================================================================
// UART Communication
// =============================================================================

/**
 * Send Modbus request to PZEM
 * @param function_code Modbus function code
 * @param start_register Starting register address
 * @param num_registers Number of registers to read
 * @return true if sent successfully
 */
static bool pzem_send_request(uint8_t function_code, uint16_t start_register, uint16_t num_registers) {
    if (!g_pzem_uart) return false;
    
    // Build Modbus RTU request frame
    uint8_t frame[8];
    frame[0] = g_pzem_address;              // Device address
    frame[1] = function_code;               // Function code
    frame[2] = (start_register >> 8) & 0xFF;  // Register address high
    frame[3] = start_register & 0xFF;       // Register address low
    frame[4] = (num_registers >> 8) & 0xFF; // Quantity high
    frame[5] = num_registers & 0xFF;         // Quantity low
    
    // Calculate CRC16
    uint16_t crc = modbus_crc16(frame, 6);
    frame[6] = crc & 0xFF;                  // CRC low
    frame[7] = (crc >> 8) & 0xFF;           // CRC high
    
    // Send frame
    uart_write_blocking(g_pzem_uart, frame, 8);
    
    return true;
}

/**
 * Read Modbus response from PZEM
 * @param buffer Buffer to store response
 * @param max_length Maximum buffer length
 * @param timeout_ms Timeout in milliseconds
 * @return Number of bytes read, or -1 on timeout/error
 */
static int pzem_read_response(uint8_t* buffer, uint16_t max_length, uint32_t timeout_ms) {
    if (!g_pzem_uart) return -1;
    
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint16_t index = 0;
    
    while (index < max_length) {
        if (uart_is_readable(g_pzem_uart)) {
            buffer[index++] = uart_getc(g_pzem_uart);
            start_time = to_ms_since_boot(get_absolute_time()); // Reset timeout on data
        }
        
        uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - start_time;
        if (elapsed > timeout_ms) {
            break; // Timeout
        }
        
        sleep_us(100); // Small delay
    }
    
    return (index > 0) ? index : -1;
}

/**
 * Parse PZEM response and extract register values
 * @param response Response buffer
 * @param length Response length
 * @param data Pointer to pzem_data_t to fill
 * @return true if parsing successful
 */
static bool pzem_parse_response(const uint8_t* response, uint16_t length, pzem_data_t* data) {
    if (!response || !data || length < 9) {
        return false; // Invalid response
    }
    
    // Check response header
    if (response[0] != g_pzem_address || response[1] != MODBUS_READ_INPUT_REGS) {
        return false; // Invalid header
    }
    
    uint8_t byte_count = response[2];
    if (byte_count != 12 || length < (5 + byte_count)) {
        return false; // Invalid byte count
    }
    
    // Verify CRC16
    uint16_t received_crc = response[length - 2] | (response[length - 1] << 8);
    uint16_t calculated_crc = modbus_crc16(response, length - 2);
    if (received_crc != calculated_crc) {
        return false; // CRC mismatch
    }
    
    // Parse register values (big-endian)
    // Register 0x0000: Voltage (V * 10)
    uint16_t voltage_raw = (response[3] << 8) | response[4];
    data->voltage = voltage_raw / 10.0f;
    
    // Register 0x0001: Current (A * 100)
    uint16_t current_raw = (response[5] << 8) | response[6];
    data->current = current_raw / 100.0f;
    
    // Register 0x0002: Power (W)
    data->power = (response[7] << 8) | response[8];
    
    // Register 0x0003: Energy (Wh)
    uint32_t energy_low = (response[9] << 8) | response[10];
    uint32_t energy_high = (response[11] << 8) | response[12];
    data->energy = (energy_high << 16) | energy_low;
    
    // Register 0x0004: Frequency (Hz * 10)
    uint16_t frequency_raw = (response[13] << 8) | response[14];
    data->frequency = frequency_raw / 10.0f;
    
    // Register 0x0005: Power Factor (0-100)
    uint16_t pf_raw = (response[15] << 8) | response[16];
    data->power_factor = pf_raw / 100.0f;
    
    return true;
}

// =============================================================================
// Public Functions
// =============================================================================

bool pzem_is_available(void) {
    const pcb_config_t* pcb = pcb_config_get();
    if (!pcb) return false;
    
    // Check if PZEM UART pins are configured
    return (pcb->pins.uart_pzem_tx >= 0 && pcb->pins.uart_pzem_rx >= 0);
}

bool pzem_init(uint8_t address) {
    if (g_pzem_initialized) {
        return true; // Already initialized
    }
    
    if (!pzem_is_available()) {
        DEBUG_PRINT("PZEM: Not available (UART pins not configured)\n");
        return false;
    }
    
    const pcb_config_t* pcb = pcb_config_get();
    if (!pcb) return false;
    
    g_pzem_address = address;
    
    // Determine UART instance (UART1 for PZEM, UART0 is for ESP32)
    g_pzem_uart_num = 1;
    g_pzem_uart = uart1;
    
    // Initialize UART
    uint baud = uart_init(g_pzem_uart, PZEM_BAUD_RATE);
    if (baud == 0) {
        DEBUG_PRINT("PZEM: Failed to initialize UART\n");
        return false;
    }
    
    // Configure GPIO pins
    gpio_set_function(pcb->pins.uart_pzem_tx, GPIO_FUNC_UART);
    gpio_set_function(pcb->pins.uart_pzem_rx, GPIO_FUNC_UART);
    
    // Configure UART parameters
    uart_set_hw_flow(g_pzem_uart, false, false); // No hardware flow control
    uart_set_format(g_pzem_uart, PZEM_UART_DATA_BITS, PZEM_UART_STOP_BITS, false); // No parity
    
    // Clear UART buffers
    uart_tx_wait_blocking(g_pzem_uart);
    while (uart_is_readable(g_pzem_uart)) {
        uart_getc(g_pzem_uart);
    }
    
    g_pzem_initialized = true;
    g_pzem_data.valid = false;
    g_pzem_error_count = 0;
    
    DEBUG_PRINT("PZEM: Initialized (UART%d, address=0x%02X, baud=%d)\n", 
                g_pzem_uart_num, g_pzem_address, baud);
    
    return true;
}

bool pzem_read(pzem_data_t* data) {
    if (!data) return false;
    
    if (!g_pzem_initialized) {
        if (!pzem_init(PZEM_DEFAULT_ADDRESS)) {
            return false;
        }
    }
    
    // Send read request for all 6 registers (0x0000-0x0005)
    if (!pzem_send_request(MODBUS_READ_INPUT_REGS, PZEM_REG_VOLTAGE, 6)) {
        g_pzem_error_count++;
        return false;
    }
    
    // Read response (max 23 bytes: 1 addr + 1 func + 1 byte_count + 12 data + 2 CRC)
    // Note: pzem_read_response() has built-in timeout, no need for sleep_ms
    // The response typically arrives within 10-20ms at 9600 baud
    uint8_t response[23];
    int length = pzem_read_response(response, sizeof(response), PZEM_READ_TIMEOUT_MS);
    
    if (length < 0) {
        g_pzem_error_count++;
        DEBUG_PRINT("PZEM: Read timeout\n");
        return false;
    }
    
    // Parse response
    pzem_data_t temp_data = {0};
    if (!pzem_parse_response(response, length, &temp_data)) {
        g_pzem_error_count++;
        DEBUG_PRINT("PZEM: Parse error\n");
        return false;
    }
    
    // Update cached data
    temp_data.valid = true;
    temp_data.last_read_ms = to_ms_since_boot(get_absolute_time());
    g_pzem_data = temp_data;
    
    *data = temp_data;
    
    return true;
}

bool pzem_get_data(pzem_data_t* data) {
    if (!data) return false;
    
    if (!g_pzem_data.valid) {
        return false;
    }
    
    *data = g_pzem_data;
    return true;
}

// Note: pzem_update() removed - PZEM is now read in sensor cycle (sensors_read())
// This keeps all sensor reading synchronized in Core 0

bool pzem_reset_energy(void) {
    // PZEM-004T doesn't have a standard Modbus reset command
    // Energy counter typically resets via hardware button or power cycle
    // This function is a placeholder for future implementation
    DEBUG_PRINT("PZEM: Reset energy not implemented (requires hardware reset)\n");
    return false;
}

uint32_t pzem_get_error_count(void) {
    return g_pzem_error_count;
}

