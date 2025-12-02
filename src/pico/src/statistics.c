/**
 * ECM Pico Firmware - Brew Statistics Implementation
 * 
 * Tracks comprehensive brew statistics including times, averages, and historical data.
 */

#include "statistics.h"
#include "config.h"
#include "flash_safe.h"       // Flash safety utilities
#include "pico/stdlib.h"
#include "hardware/flash.h"
#include <string.h>
#include <stddef.h>

// =============================================================================
// Private State
// =============================================================================

static statistics_t g_stats = {0};
static bool g_initialized = false;
static uint16_t g_brews_since_last_save = 0;

// =============================================================================
// Flash Storage Configuration
// =============================================================================

// Use second-to-last flash sector for statistics (2MB flash = 512 sectors, use sector 510)
// Config uses sector 511, statistics uses sector 510
// Each sector is 4KB
#ifndef XIP_BASE
#define XIP_BASE 0x10000000
#endif

#define STATS_MAGIC           0x45434D53  // "ECMS" - ECM Statistics
#define STATS_VERSION         1           // Statistics format version
#define STATS_SAVE_INTERVAL   10          // Save every 10 brews (not every brew to reduce flash wear)

// Persisted statistics structure (smaller than full statistics_t - only essential data)
typedef struct __attribute__((packed)) {
    uint32_t magic;                      // Must be STATS_MAGIC
    uint16_t version;                     // Statistics format version
    
    // Overall statistics
    uint32_t total_brews;
    uint32_t total_brew_time_ms;
    uint16_t avg_brew_time_ms;
    uint16_t min_brew_time_ms;
    uint16_t max_brew_time_ms;
    
    // Last 50 history entries (reduced from 100 to fit in flash)
    brew_entry_t history[50];
    uint8_t history_count;                // Number of entries saved
    
    // Metadata
    uint32_t last_brew_timestamp;
    
    // Reserved for future use
    uint8_t reserved[32];
    
    // CRC32 for integrity check
    uint32_t crc32;
} persisted_statistics_t;

#define STATS_FLASH_OFFSET   (PICO_FLASH_SIZE_BYTES - (2 * FLASH_SECTOR_SIZE))  // Second-to-last sector
#define STATS_FLASH_SECTOR   (STATS_FLASH_OFFSET / FLASH_SECTOR_SIZE)

// Forward declarations
static bool load_statistics_from_flash(void);
static bool save_statistics_to_flash(void);

// =============================================================================
// Helper Functions
// =============================================================================

/**
 * Get current timestamp (milliseconds since boot)
 * TODO: If RTC is available, use actual time/date
 */
static uint32_t get_current_timestamp(void) {
    return to_ms_since_boot(get_absolute_time());
}

/**
 * Check if timestamp is within last N milliseconds
 */
static bool is_within_time(uint32_t timestamp, uint32_t window_ms) {
    uint32_t now = get_current_timestamp();
    if (now < timestamp) {
        // Timestamp is in the future (shouldn't happen, but handle gracefully)
        return false;
    }
    return (now - timestamp) <= window_ms;
}

/**
 * Recalculate time-based statistics
 */
static void recalculate_time_based_stats(void) {
    uint32_t now = get_current_timestamp();
    
    // Reset time-based stats
    g_stats.daily.count = 0;
    g_stats.daily.total_time_ms = 0;
    g_stats.weekly.count = 0;
    g_stats.weekly.total_time_ms = 0;
    g_stats.monthly.count = 0;
    g_stats.monthly.total_time_ms = 0;
    
    // Iterate through history and count entries within time windows
    for (uint8_t i = 0; i < g_stats.history_count; i++) {
        brew_entry_t* entry = &g_stats.history[i];
        
        if (is_within_time(entry->timestamp, 24 * 60 * 60 * 1000UL)) {  // 24 hours
            g_stats.daily.count++;
            g_stats.daily.total_time_ms += entry->duration_ms;
        }
        
        if (is_within_time(entry->timestamp, 7 * 24 * 60 * 60 * 1000UL)) {  // 7 days
            g_stats.weekly.count++;
            g_stats.weekly.total_time_ms += entry->duration_ms;
        }
        
        if (is_within_time(entry->timestamp, 30 * 24 * 60 * 60 * 1000UL)) {  // 30 days
            g_stats.monthly.count++;
            g_stats.monthly.total_time_ms += entry->duration_ms;
        }
    }
    
    // Calculate averages
    if (g_stats.daily.count > 0) {
        g_stats.daily.avg_time_ms = g_stats.daily.total_time_ms / g_stats.daily.count;
    } else {
        g_stats.daily.avg_time_ms = 0;
    }
    
    if (g_stats.weekly.count > 0) {
        g_stats.weekly.avg_time_ms = g_stats.weekly.total_time_ms / g_stats.weekly.count;
    } else {
        g_stats.weekly.avg_time_ms = 0;
    }
    
    if (g_stats.monthly.count > 0) {
        g_stats.monthly.avg_time_ms = g_stats.monthly.total_time_ms / g_stats.monthly.count;
    } else {
        g_stats.monthly.avg_time_ms = 0;
    }
}

// =============================================================================
// Flash Operations
// =============================================================================

/**
 * Calculate CRC32 for statistics data
 */
static uint32_t stats_crc32_calculate(const uint8_t* data, size_t length) {
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

/**
 * Read statistics from flash
 */
static bool flash_read_statistics(persisted_statistics_t* stats) {
    if (!stats) return false;
    
    // Read from flash
    const uint8_t* flash_addr = (const uint8_t*)(XIP_BASE + STATS_FLASH_OFFSET);
    persisted_statistics_t flash_stats;
    memcpy(&flash_stats, flash_addr, sizeof(persisted_statistics_t));
    
    // Validate magic number
    if (flash_stats.magic != STATS_MAGIC) {
        return false;  // No valid statistics in flash
    }
    
    // Validate version
    if (flash_stats.version != STATS_VERSION) {
        return false;  // Wrong version
    }
    
    // Calculate CRC32 (excluding CRC field itself)
    size_t crc_size = offsetof(persisted_statistics_t, crc32);
    uint32_t calculated_crc = stats_crc32_calculate((const uint8_t*)&flash_stats, crc_size);
    
    if (calculated_crc != flash_stats.crc32) {
        return false;  // CRC mismatch - data corrupted
    }
    
    // Copy to output
    *stats = flash_stats;
    return true;
}

/**
 * Write statistics to flash
 * 
 * NOTE: persisted_statistics_t is larger than FLASH_PAGE_SIZE (256 bytes),
 * so we must write multiple pages.
 * 
 * Uses flash_safe API for XIP safety (compatible with RP2040 and RP2350).
 */
static bool flash_write_statistics(const persisted_statistics_t* stats) {
    if (!stats) return false;
    
    // Calculate CRC32 before saving
    persisted_statistics_t stats_with_crc = *stats;
    size_t crc_size = offsetof(persisted_statistics_t, crc32);
    stats_with_crc.crc32 = stats_crc32_calculate((const uint8_t*)&stats_with_crc, crc_size);
    
    // Calculate how many pages we need
    size_t total_size = sizeof(persisted_statistics_t);
    size_t num_pages = (total_size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;
    size_t total_write_size = num_pages * FLASH_PAGE_SIZE;
    
    // Erase sector first (using flash_safe API)
    if (!flash_safe_erase(STATS_FLASH_OFFSET, FLASH_SECTOR_SIZE)) {
        DEBUG_PRINT("Statistics: Flash erase failed\n");
        return false;
    }
    
    // Write statistics page by page
    const uint8_t* src = (const uint8_t*)&stats_with_crc;
    size_t remaining = total_size;
    
    for (size_t page = 0; page < num_pages; page++) {
        uint8_t write_buffer[FLASH_PAGE_SIZE];
        size_t bytes_this_page = (remaining > FLASH_PAGE_SIZE) ? FLASH_PAGE_SIZE : remaining;
        
        // Copy data to page buffer, pad with 0xFF
        memset(write_buffer, 0xFF, FLASH_PAGE_SIZE);
        memcpy(write_buffer, src + (page * FLASH_PAGE_SIZE), bytes_this_page);
        
        // Write page (using flash_safe API)
        if (!flash_safe_program(STATS_FLASH_OFFSET + (page * FLASH_PAGE_SIZE), 
                                write_buffer, FLASH_PAGE_SIZE)) {
            DEBUG_PRINT("Statistics: Flash program failed at page %zu\n", page);
            return false;
        }
        
        remaining -= bytes_this_page;
    }
    
    return true;
}

/**
 * Save statistics to flash (only essential data)
 */
static bool save_statistics_to_flash(void) {
    persisted_statistics_t persisted = {0};
    
    persisted.magic = STATS_MAGIC;
    persisted.version = STATS_VERSION;
    
    // Copy overall statistics
    persisted.total_brews = g_stats.total_brews;
    persisted.total_brew_time_ms = g_stats.total_brew_time_ms;
    persisted.avg_brew_time_ms = g_stats.avg_brew_time_ms;
    persisted.min_brew_time_ms = g_stats.min_brew_time_ms;
    persisted.max_brew_time_ms = g_stats.max_brew_time_ms;
    persisted.last_brew_timestamp = g_stats.last_brew_timestamp;
    
    // Copy last 50 history entries (newest first)
    uint8_t entries_to_save = (g_stats.history_count < 50) ? g_stats.history_count : 50;
    persisted.history_count = entries_to_save;
    
    if (g_stats.history_count <= STATS_MAX_HISTORY_ENTRIES) {
        // Not wrapped yet - copy last N entries
        for (uint8_t i = 0; i < entries_to_save; i++) {
            persisted.history[i] = g_stats.history[g_stats.history_count - 1 - i];
        }
    } else {
        // Wrapped - need to handle circular buffer
        uint8_t start_idx = g_stats.history_index;
        for (uint8_t i = 0; i < entries_to_save; i++) {
            uint8_t src_idx = (start_idx - 1 - i + STATS_MAX_HISTORY_ENTRIES) % STATS_MAX_HISTORY_ENTRIES;
            persisted.history[i] = g_stats.history[src_idx];
        }
    }
    
    if (flash_write_statistics(&persisted)) {
        g_brews_since_last_save = 0;
        DEBUG_PRINT("Statistics: Saved to flash (total=%lu, history=%u entries)\n",
                    persisted.total_brews, persisted.history_count);
        return true;
    }
    
    DEBUG_PRINT("Statistics: Failed to save to flash\n");
    return false;
}

/**
 * Load statistics from flash
 */
static bool load_statistics_from_flash(void) {
    persisted_statistics_t persisted = {0};
    
    if (!flash_read_statistics(&persisted)) {
        return false;  // No valid statistics in flash
    }
    
    // Restore overall statistics
    g_stats.total_brews = persisted.total_brews;
    g_stats.total_brew_time_ms = persisted.total_brew_time_ms;
    g_stats.avg_brew_time_ms = persisted.avg_brew_time_ms;
    g_stats.min_brew_time_ms = persisted.min_brew_time_ms;
    g_stats.max_brew_time_ms = persisted.max_brew_time_ms;
    g_stats.last_brew_timestamp = persisted.last_brew_timestamp;
    
    // Restore history (copy to beginning of circular buffer)
    g_stats.history_count = persisted.history_count;
    g_stats.history_index = 0;
    
    // Copy entries in chronological order (oldest first)
    for (uint8_t i = 0; i < persisted.history_count; i++) {
        // Reverse order (newest first in persisted, oldest first in g_stats)
        g_stats.history[persisted.history_count - 1 - i] = persisted.history[i];
    }
    
    // Update history_index to point after last entry
    g_stats.history_index = persisted.history_count;
    
    // Recalculate time-based stats from history
    recalculate_time_based_stats();
    
    DEBUG_PRINT("Statistics: Loaded from flash (total=%lu, history=%u entries)\n",
                g_stats.total_brews, g_stats.history_count);
    
    return true;
}

// =============================================================================
// Initialization
// =============================================================================

void statistics_init(void) {
    if (g_initialized) {
        return;
    }
    
    memset(&g_stats, 0, sizeof(statistics_t));
    g_stats.initialized = true;
    g_brews_since_last_save = 0;
    
    // Try to load statistics from flash
    if (load_statistics_from_flash()) {
        DEBUG_PRINT("Statistics: Loaded from flash\n");
    } else {
        DEBUG_PRINT("Statistics: No valid data in flash, starting fresh\n");
    }
    
    g_initialized = true;
    
    DEBUG_PRINT("Statistics initialized (total brews=%lu)\n", g_stats.total_brews);
}

// =============================================================================
// Record Brew
// =============================================================================

bool statistics_record_brew(uint32_t duration_ms) {
    if (!g_initialized) {
        statistics_init();
    }
    
    // Validate brew duration
    if (duration_ms < STATS_MIN_BREW_TIME_MS || duration_ms > STATS_MAX_BREW_TIME_MS) {
        DEBUG_PRINT("Statistics: Invalid brew duration %lu ms (ignored)\n", duration_ms);
        return false;
    }
    
    uint32_t now = get_current_timestamp();
    
    // Update overall statistics
    g_stats.total_brews++;
    g_stats.total_brew_time_ms += duration_ms;
    
    if (g_stats.total_brews > 0) {
        g_stats.avg_brew_time_ms = g_stats.total_brew_time_ms / g_stats.total_brews;
    }
    
    // Update min/max
    if (g_stats.total_brews == 1) {
        g_stats.min_brew_time_ms = duration_ms;
        g_stats.max_brew_time_ms = duration_ms;
    } else {
        if (duration_ms < g_stats.min_brew_time_ms) {
            g_stats.min_brew_time_ms = duration_ms;
        }
        if (duration_ms > g_stats.max_brew_time_ms) {
            g_stats.max_brew_time_ms = duration_ms;
        }
    }
    
    // Add to history (circular buffer)
    brew_entry_t* entry = &g_stats.history[g_stats.history_index];
    entry->timestamp = now;
    entry->duration_ms = (uint16_t)duration_ms;  // Clamp to uint16_t range
    
    g_stats.history_index++;
    if (g_stats.history_index >= STATS_MAX_HISTORY_ENTRIES) {
        g_stats.history_index = 0;  // Wrap around
    }
    
    if (g_stats.history_count < STATS_MAX_HISTORY_ENTRIES) {
        g_stats.history_count++;
    }
    
    // Update last brew timestamp
    g_stats.last_brew_timestamp = now;
    
    // Recalculate time-based statistics
    recalculate_time_based_stats();
    
    DEBUG_PRINT("Statistics: Brew recorded (total=%lu, duration=%lu ms, avg=%u ms)\n",
                g_stats.total_brews, duration_ms, g_stats.avg_brew_time_ms);
    
    // Save to flash periodically (every STATS_SAVE_INTERVAL brews) to reduce flash wear
    g_brews_since_last_save++;
    if (g_brews_since_last_save >= STATS_SAVE_INTERVAL) {
        save_statistics_to_flash();
    }
    
    return true;
}

// =============================================================================
// Get Statistics
// =============================================================================

bool statistics_get_all(statistics_t* stats) {
    if (!stats || !g_initialized) {
        return false;
    }
    
    // Update time-based stats before returning
    recalculate_time_based_stats();
    
    *stats = g_stats;
    return true;
}

uint32_t statistics_get_total_brews(void) {
    return g_stats.total_brews;
}

uint16_t statistics_get_avg_brew_time_ms(void) {
    return g_stats.avg_brew_time_ms;
}

bool statistics_get_daily(daily_stats_t* stats) {
    if (!stats || !g_initialized) {
        return false;
    }
    
    recalculate_time_based_stats();
    *stats = g_stats.daily;
    return true;
}

bool statistics_get_weekly(weekly_stats_t* stats) {
    if (!stats || !g_initialized) {
        return false;
    }
    
    recalculate_time_based_stats();
    *stats = g_stats.weekly;
    return true;
}

bool statistics_get_monthly(monthly_stats_t* stats) {
    if (!stats || !g_initialized) {
        return false;
    }
    
    recalculate_time_based_stats();
    *stats = g_stats.monthly;
    return true;
}

uint8_t statistics_get_history(brew_entry_t* entries, uint8_t max_entries) {
    if (!entries || max_entries == 0 || !g_initialized) {
        return 0;
    }
    
    uint8_t count = (g_stats.history_count < max_entries) ? g_stats.history_count : max_entries;
    
    // Copy entries in reverse chronological order (newest first)
    // History is stored in chronological order, so we need to reverse
    if (g_stats.history_count <= STATS_MAX_HISTORY_ENTRIES) {
        // Not wrapped yet - simple copy
        for (uint8_t i = 0; i < count; i++) {
            entries[i] = g_stats.history[g_stats.history_count - 1 - i];
        }
    } else {
        // Wrapped - need to handle circular buffer
        uint8_t start_idx = g_stats.history_index;
        for (uint8_t i = 0; i < count; i++) {
            uint8_t src_idx = (start_idx - 1 - i + STATS_MAX_HISTORY_ENTRIES) % STATS_MAX_HISTORY_ENTRIES;
            entries[i] = g_stats.history[src_idx];
        }
    }
    
    return count;
}

void statistics_reset_all(void) {
    memset(&g_stats, 0, sizeof(statistics_t));
    g_stats.initialized = true;
    g_brews_since_last_save = 0;
    
    DEBUG_PRINT("Statistics: All statistics reset\n");
    
    // Save to flash (clear persisted data)
    save_statistics_to_flash();
}

void statistics_update_time_based(void) {
    if (!g_initialized) {
        return;
    }
    
    recalculate_time_based_stats();
}

/**
 * Force save statistics to flash (call before shutdown or periodically)
 */
void statistics_save_to_flash(void) {
    if (!g_initialized) {
        return;
    }
    
    save_statistics_to_flash();
}

