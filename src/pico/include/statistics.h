/**
 * ECM Pico Firmware - Brew Statistics
 * 
 * Comprehensive statistics tracking for brew cycles:
 * - Individual brew times
 * - Average brew times
 * - Cups per day/week/month
 * - Total brew counter
 * - Historical data
 * 
 * Note: This is separate from the cleaning counter, which resets after cleaning.
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Configuration
// =============================================================================

#define STATS_MAX_HISTORY_ENTRIES    100     // Maximum number of brew entries to store
#define STATS_MIN_BREW_TIME_MS       5000    // Minimum brew time to count (5 seconds)
#define STATS_MAX_BREW_TIME_MS       120000  // Maximum brew time (2 minutes)

// =============================================================================
// Statistics Data Structures
// =============================================================================

/**
 * Individual brew entry
 */
typedef struct __attribute__((packed)) {
    uint32_t timestamp;        // Milliseconds since boot (or epoch if RTC available)
    uint16_t duration_ms;      // Brew duration in milliseconds
    uint8_t reserved[2];       // Reserved for future use
} brew_entry_t;  // 8 bytes

/**
 * Daily statistics (rolling 24 hours)
 */
typedef struct __attribute__((packed)) {
    uint16_t count;            // Number of brews in last 24 hours
    uint32_t total_time_ms;    // Total brew time in last 24 hours
    uint16_t avg_time_ms;      // Average brew time in last 24 hours
} daily_stats_t;  // 8 bytes

/**
 * Weekly statistics (rolling 7 days)
 */
typedef struct __attribute__((packed)) {
    uint16_t count;            // Number of brews in last 7 days
    uint32_t total_time_ms;    // Total brew time in last 7 days
    uint16_t avg_time_ms;      // Average brew time in last 7 days
} weekly_stats_t;  // 8 bytes

/**
 * Monthly statistics (rolling 30 days)
 */
typedef struct __attribute__((packed)) {
    uint16_t count;            // Number of brews in last 30 days
    uint32_t total_time_ms;    // Total brew time in last 30 days
    uint16_t avg_time_ms;      // Average brew time in last 30 days
} monthly_stats_t;  // 8 bytes

/**
 * Complete statistics structure
 */
typedef struct {
    // Overall statistics
    uint32_t total_brews;          // Total number of brews (all time)
    uint32_t total_brew_time_ms;   // Total brew time (all time)
    uint16_t avg_brew_time_ms;     // Average brew time (all time)
    uint16_t min_brew_time_ms;     // Shortest brew time
    uint16_t max_brew_time_ms;     // Longest brew time
    
    // Time-based statistics
    daily_stats_t daily;
    weekly_stats_t weekly;
    monthly_stats_t monthly;
    
    // History (circular buffer)
    brew_entry_t history[STATS_MAX_HISTORY_ENTRIES];
    uint8_t history_count;         // Number of entries in history
    uint8_t history_index;         // Next index to write (circular buffer)
    
    // Metadata
    uint32_t last_brew_timestamp;  // Timestamp of last brew
    bool initialized;              // Statistics initialized flag
} statistics_t;

// =============================================================================
// Functions
// =============================================================================

/**
 * Initialize statistics system
 */
void statistics_init(void);

/**
 * Record a brew cycle
 * @param duration_ms Brew duration in milliseconds
 * @return true if recorded successfully
 */
bool statistics_record_brew(uint32_t duration_ms);

/**
 * Get complete statistics
 * @param stats Pointer to statistics structure to fill
 * @return true if statistics retrieved successfully
 */
bool statistics_get_all(statistics_t* stats);

/**
 * Get total brew count (all time)
 */
uint32_t statistics_get_total_brews(void);

/**
 * Get average brew time (all time) in milliseconds
 */
uint16_t statistics_get_avg_brew_time_ms(void);

/**
 * Get daily statistics
 * @param stats Pointer to daily_stats_t to fill
 * @return true if retrieved successfully
 */
bool statistics_get_daily(daily_stats_t* stats);

/**
 * Get weekly statistics
 * @param stats Pointer to weekly_stats_t to fill
 * @return true if retrieved successfully
 */
bool statistics_get_weekly(weekly_stats_t* stats);

/**
 * Get monthly statistics
 * @param stats Pointer to monthly_stats_t to fill
 * @return true if retrieved successfully
 */
bool statistics_get_monthly(monthly_stats_t* stats);

/**
 * Get brew history
 * @param entries Pointer to array of brew_entry_t
 * @param max_entries Maximum number of entries to retrieve
 * @return Number of entries retrieved
 */
uint8_t statistics_get_history(brew_entry_t* entries, uint8_t max_entries);

/**
 * Reset all statistics (clears all data)
 */
void statistics_reset_all(void);

/**
 * Update time-based statistics (call periodically, e.g., every hour)
 * This recalculates daily/weekly/monthly stats based on current time
 */
void statistics_update_time_based(void);

/**
 * Force save statistics to flash (call before shutdown or periodically)
 * Statistics are automatically saved every STATS_SAVE_INTERVAL brews,
 * but this can be called to force an immediate save
 */
void statistics_save_to_flash(void);

#endif // STATISTICS_H

