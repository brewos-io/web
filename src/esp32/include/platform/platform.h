/**
 * BrewOS Platform Abstraction Layer
 * 
 * Provides a common interface for platform-specific functionality.
 * This allows UI code to compile for both ESP32 and native simulator.
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Log Levels (needed before impl headers)
// =============================================================================

typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} log_level_t;

// =============================================================================
// Platform Detection
// =============================================================================

#ifdef SIMULATOR
    #define PLATFORM_SIMULATOR 1
    #define PLATFORM_ESP32 0
#else
    #define PLATFORM_SIMULATOR 0
    #define PLATFORM_ESP32 1
#endif

#ifdef __cplusplus
}
#endif

// Include the appropriate implementation
#ifdef SIMULATOR
    #include "platform/native_impl.h"
#else
    #include "platform/arduino_impl.h"
#endif

// =============================================================================
// Logging Convenience Macros
// =============================================================================

#define LOG_D(fmt, ...) platform_log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) platform_log(LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) platform_log(LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) platform_log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

#endif // PLATFORM_H

