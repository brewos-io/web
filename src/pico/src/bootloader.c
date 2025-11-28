/**
 * ECM Pico Firmware - Serial Bootloader
 * 
 * Receives firmware over UART and writes to flash for OTA updates.
 */

#include "bootloader.h"
#include "config.h"
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "hardware/watchdog.h"

// Bootloader protocol constants
#define BOOTLOADER_MAGIC_1          0x55
#define BOOTLOADER_MAGIC_2          0xAA
#define BOOTLOADER_END_MAGIC_1      0xAA
#define BOOTLOADER_END_MAGIC_2      0x55
#define BOOTLOADER_CHUNK_MAX_SIZE   256
#define BOOTLOADER_TIMEOUT_MS       30000  // 30 seconds total timeout
#define BOOTLOADER_CHUNK_TIMEOUT_MS 5000   // 5 seconds per chunk

// Flash layout
// RP2040 has 2MB flash typically, firmware starts at 0x10000000
// We'll use a reserved region for new firmware (last 512KB)
// For now, we'll write to a backup region and swap later
#define FLASH_TARGET_OFFSET         (1536 * 1024)  // Start of last 512KB
#define FLASH_SECTOR_SIZE           4096           // RP2040 flash sector size
#define FLASH_PAGE_SIZE             256            // RP2040 flash page size

// Bootloader state
static uint32_t g_total_size = 0;
static uint32_t g_received_size = 0;
static uint32_t g_chunk_count = 0;
static bool g_receiving = false;

// -----------------------------------------------------------------------------
// UART Helpers
// -----------------------------------------------------------------------------

static bool uart_read_byte_timeout(uint8_t* byte, uint32_t timeout_ms) {
    absolute_time_t timeout = make_timeout_time_ms(timeout_ms);
    
    while (!uart_is_readable(ESP32_UART_ID)) {
        if (time_reached(timeout)) {
            return false;
        }
        sleep_us(100);
    }
    
    *byte = uart_getc(ESP32_UART_ID);
    return true;
}

static bool uart_read_bytes_timeout(uint8_t* buffer, size_t count, uint32_t timeout_ms) {
    absolute_time_t start = get_absolute_time();
    
    for (size_t i = 0; i < count; i++) {
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(start);
        uint32_t remaining = timeout_ms > elapsed_ms ? timeout_ms - elapsed_ms : 100;
        
        if (!uart_read_byte_timeout(&buffer[i], remaining)) {
            return false;
        }
    }
    
    return true;
}

static void uart_write_byte(uint8_t byte) {
    uart_putc(ESP32_UART_ID, byte);
}

static void uart_write_bytes(const uint8_t* data, size_t len) {
    uart_write_blocking(ESP32_UART_ID, data, len);
}

// -----------------------------------------------------------------------------
// Flash Helpers
// -----------------------------------------------------------------------------

static bool flash_erase_sector(uint32_t offset) {
    // Validate offset is sector-aligned
    if (offset % FLASH_SECTOR_SIZE != 0) {
        return false;
    }
    
    // Validate offset is within flash bounds
    // RP2040 flash typically starts at 0x10000000, size varies
    // For safety, check offset is reasonable (less than 2MB)
    if (offset > (2 * 1024 * 1024)) {
        return false;
    }
    
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    return true;
}

static bool flash_write_page(uint32_t offset, const uint8_t* data) {
    // Validate offset is page-aligned
    if (offset % FLASH_PAGE_SIZE != 0) {
        return false;
    }
    
    // Validate offset is within flash bounds
    if (offset > (2 * 1024 * 1024)) {
        return false;
    }
    
    // Validate data pointer
    if (data == NULL) {
        return false;
    }
    
    uint32_t ints = save_and_disable_interrupts();
    flash_range_program(offset, data, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    return true;
}

// -----------------------------------------------------------------------------
// Bootloader Protocol
// -----------------------------------------------------------------------------

static bool receive_chunk_header(uint32_t* chunk_num, uint16_t* chunk_size) {
    uint8_t header[8];
    
    // Read magic bytes
    if (!uart_read_bytes_timeout(header, 2, BOOTLOADER_CHUNK_TIMEOUT_MS)) {
        return false;
    }
    
    if (header[0] != BOOTLOADER_MAGIC_1 || header[1] != BOOTLOADER_MAGIC_2) {
        // Check for end marker
        if (header[0] == BOOTLOADER_END_MAGIC_1 && header[1] == BOOTLOADER_END_MAGIC_2) {
            *chunk_num = 0xFFFFFFFF;
            *chunk_size = 0;
            return true;
        }
        return false;
    }
    
    // Read chunk number (4 bytes, little-endian)
    if (!uart_read_bytes_timeout(&header[2], 4, BOOTLOADER_CHUNK_TIMEOUT_MS)) {
        return false;
    }
    *chunk_num = header[2] | (header[3] << 8) | (header[4] << 16) | (header[5] << 24);
    
    // Read chunk size (2 bytes, little-endian)
    if (!uart_read_bytes_timeout(&header[6], 2, BOOTLOADER_CHUNK_TIMEOUT_MS)) {
        return false;
    }
    *chunk_size = header[6] | (header[7] << 8);
    
    return true;
}

static bool receive_chunk_data(uint8_t* buffer, uint16_t size, uint8_t* checksum) {
    // Read data
    if (!uart_read_bytes_timeout(buffer, size, BOOTLOADER_CHUNK_TIMEOUT_MS)) {
        return false;
    }
    
    // Read checksum
    if (!uart_read_byte_timeout(checksum, BOOTLOADER_CHUNK_TIMEOUT_MS)) {
        return false;
    }
    
    // Verify checksum (XOR of all data bytes)
    uint8_t calculated_checksum = 0;
    for (uint16_t i = 0; i < size; i++) {
        calculated_checksum ^= buffer[i];
    }
    
    if (calculated_checksum != *checksum) {
        return false;
    }
    
    return true;
}

// -----------------------------------------------------------------------------
// Main Bootloader Function
// -----------------------------------------------------------------------------

bootloader_result_t bootloader_receive_firmware(void) {
    g_receiving = true;
    g_total_size = 0;
    g_received_size = 0;
    g_chunk_count = 0;
    
    // Send acknowledgment to ESP32
    uart_write_byte(0xAA);  // Simple ACK byte
    uart_write_byte(0x55);
    
    // Wait a bit for ESP32 to start streaming
    sleep_ms(100);
    
    // Flash write state (page buffering)
    static uint8_t page_buffer[FLASH_PAGE_SIZE];
    static uint32_t page_buffer_offset = 0;
    static uint32_t current_page_start = FLASH_TARGET_OFFSET;
    static uint32_t current_sector = FLASH_TARGET_OFFSET / FLASH_SECTOR_SIZE;
    static bool sector_erased = false;
    
    // Reset page buffer state
    page_buffer_offset = 0;
    current_page_start = FLASH_TARGET_OFFSET;
    current_sector = FLASH_TARGET_OFFSET / FLASH_SECTOR_SIZE;
    sector_erased = false;
    
    absolute_time_t start_time = get_absolute_time();
    
    while (true) {
        // Check timeout
        uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(start_time);
        if (elapsed_ms > BOOTLOADER_TIMEOUT_MS) {
            uart_write_byte(0xFF);  // Error marker
            uart_write_byte(BOOTLOADER_ERROR_TIMEOUT);
            return BOOTLOADER_ERROR_TIMEOUT;
        }
        
        // Receive chunk header
        uint32_t chunk_num;
        uint16_t chunk_size;
        
        if (!receive_chunk_header(&chunk_num, &chunk_size)) {
            uart_write_byte(0xFF);  // Error marker
            uart_write_byte(BOOTLOADER_ERROR_TIMEOUT);
            return BOOTLOADER_ERROR_TIMEOUT;
        }
        
        // Check for end marker
        if (chunk_num == 0xFFFFFFFF) {
            // End of firmware
            break;
        }
        
        // Validate chunk size
        if (chunk_size == 0 || chunk_size > BOOTLOADER_CHUNK_MAX_SIZE) {
            // Send error indication to ESP32
            uart_write_byte(0xFF);  // Error marker
            uart_write_byte(BOOTLOADER_ERROR_INVALID_SIZE);
            return BOOTLOADER_ERROR_INVALID_SIZE;
        }
        
        // Validate chunk number (must be sequential)
        if (chunk_num != g_chunk_count) {
            // Out of order chunk - send error indication
            uart_write_byte(0xFF);  // Error marker
            uart_write_byte(BOOTLOADER_ERROR_INVALID_CHUNK);
            return BOOTLOADER_ERROR_INVALID_CHUNK;
        }
        
        // Check for flash overflow
        uint32_t total_written = g_received_size + chunk_size;
        uint32_t max_firmware_size = (2 * 1024 * 1024) - FLASH_TARGET_OFFSET;  // Remaining flash space
        if (total_written > max_firmware_size) {
            uart_write_byte(0xFF);  // Error marker
            uart_write_byte(BOOTLOADER_ERROR_INVALID_SIZE);
            return BOOTLOADER_ERROR_INVALID_SIZE;
        }
        
        // Receive chunk data
        uint8_t chunk_data[BOOTLOADER_CHUNK_MAX_SIZE];
        uint8_t checksum;
        
        if (!receive_chunk_data(chunk_data, chunk_size, &checksum)) {
            // Checksum error or timeout - send error indication
            uart_write_byte(0xFF);  // Error marker
            uart_write_byte(BOOTLOADER_ERROR_CHECKSUM);
            return BOOTLOADER_ERROR_CHECKSUM;
        }
        
        // Erase sector if needed (before writing to it)
        uint32_t page_sector = current_page_start / FLASH_SECTOR_SIZE;
        if (page_sector != current_sector || !sector_erased) {
            uint32_t sector_start = page_sector * FLASH_SECTOR_SIZE;
            
            // Validate sector start is within bounds
            if (sector_start < FLASH_TARGET_OFFSET || sector_start > (2 * 1024 * 1024)) {
                uart_write_byte(0xFF);  // Error marker
                uart_write_byte(BOOTLOADER_ERROR_FLASH_WRITE);
                return BOOTLOADER_ERROR_FLASH_WRITE;
            }
            
            if (!flash_erase_sector(sector_start)) {
                uart_write_byte(0xFF);  // Error marker
                uart_write_byte(BOOTLOADER_ERROR_FLASH_ERASE);
                return BOOTLOADER_ERROR_FLASH_ERASE;
            }
            current_sector = page_sector;
            sector_erased = true;
        }
        
        // Write chunk to flash
        // Flash writes must be page-aligned (256 bytes) and sector-erased
        // We'll buffer chunks until we have a full page
        
        // Copy chunk into page buffer
        if (page_buffer_offset + chunk_size <= FLASH_PAGE_SIZE) {
            // Fits in current page
            memcpy(&page_buffer[page_buffer_offset], chunk_data, chunk_size);
            page_buffer_offset += chunk_size;
            
            // If page is full, write it
            if (page_buffer_offset == FLASH_PAGE_SIZE) {
                if (!flash_write_page(current_page_start, page_buffer)) {
                    return BOOTLOADER_ERROR_FLASH_WRITE;
                }
                current_page_start += FLASH_PAGE_SIZE;
                page_buffer_offset = 0;
            }
        } else {
            // Chunk spans page boundary - write current page first
            if (page_buffer_offset > 0) {
                // Fill rest of page with 0xFF
                memset(&page_buffer[page_buffer_offset], 0xFF, FLASH_PAGE_SIZE - page_buffer_offset);
                if (!flash_write_page(current_page_start, page_buffer)) {
                    return BOOTLOADER_ERROR_FLASH_WRITE;
                }
                current_page_start += FLASH_PAGE_SIZE;
            }
            
            // Handle remaining data
            uint16_t remaining = chunk_size;
            uint16_t offset = 0;
            
            while (remaining > 0) {
                uint16_t to_copy = (remaining > FLASH_PAGE_SIZE) ? FLASH_PAGE_SIZE : remaining;
                memcpy(page_buffer, &chunk_data[offset], to_copy);
                
                if (to_copy < FLASH_PAGE_SIZE) {
                    // Partial page - fill rest with 0xFF
                    memset(&page_buffer[to_copy], 0xFF, FLASH_PAGE_SIZE - to_copy);
                    page_buffer_offset = to_copy;
                } else {
                    // Full page - write immediately
                    if (!flash_write_page(current_page_start, page_buffer)) {
                        return BOOTLOADER_ERROR_FLASH_WRITE;
                    }
                    current_page_start += FLASH_PAGE_SIZE;
                    page_buffer_offset = 0;
                }
                
                remaining -= to_copy;
                offset += to_copy;
            }
        }
        
        g_received_size += chunk_size;
        g_chunk_count++;
        
        // Check if we need to erase next sector
        uint32_t next_page_sector = (current_page_start + FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE;
        if (next_page_sector != current_sector) {
            sector_erased = false;
        }
        
        // Send progress acknowledgment (optional, helps ESP32 know we're alive)
        if (g_chunk_count % 10 == 0) {
            uart_write_byte(0xAA);  // Progress ACK
        }
    }
    
    // Write any remaining partial page
    if (page_buffer_offset > 0) {
        memset(&page_buffer[page_buffer_offset], 0xFF, FLASH_PAGE_SIZE - page_buffer_offset);
        if (!flash_write_page(current_page_start, page_buffer)) {
            return BOOTLOADER_ERROR_FLASH_WRITE;
        }
    }
    
    g_receiving = false;
    
    // TODO: Verify firmware integrity (CRC32 of entire image)
    // TODO: Swap firmware regions (copy new firmware to main location)
    // For now, we'll just reset and let the bootloader handle it
    
    // Send success acknowledgment
    uart_write_byte(0xAA);
    uart_write_byte(0x55);
    uart_write_byte(0x00);  // Success code
    
    // Small delay to ensure message is sent
    sleep_ms(100);
    
    // Reset the device
    watchdog_reboot(0, 0, 0);
    
    // Should never reach here
    return BOOTLOADER_SUCCESS;
}

const char* bootloader_get_status_message(bootloader_result_t result) {
    switch (result) {
        case BOOTLOADER_SUCCESS:
            return "Firmware update successful";
        case BOOTLOADER_ERROR_TIMEOUT:
            return "Bootloader timeout";
        case BOOTLOADER_ERROR_INVALID_MAGIC:
            return "Invalid magic bytes";
        case BOOTLOADER_ERROR_INVALID_SIZE:
            return "Invalid chunk size or flash overflow";
        case BOOTLOADER_ERROR_INVALID_CHUNK:
            return "Invalid or out-of-order chunk";
        case BOOTLOADER_ERROR_CHECKSUM:
            return "Checksum verification failed";
        case BOOTLOADER_ERROR_FLASH_WRITE:
            return "Flash write error";
        case BOOTLOADER_ERROR_FLASH_ERASE:
            return "Flash erase error";
        case BOOTLOADER_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

