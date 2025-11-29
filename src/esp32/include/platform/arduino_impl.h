/**
 * Arduino/ESP32 Platform Implementation
 * 
 * Provides platform functions using Arduino framework.
 */

#ifndef ARDUINO_IMPL_H
#define ARDUINO_IMPL_H

#include <Arduino.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t platform_millis(void) {
    return millis();
}

static inline void platform_delay(uint32_t ms) {
    delay(ms);
}

#ifdef __cplusplus
}
#endif

// Use Serial logging on ESP32/Arduino
static inline void platform_log(log_level_t level, const char* fmt, ...) {
    const char* level_str;
    switch(level) {
        case LOG_LEVEL_DEBUG: level_str = "D"; break;
        case LOG_LEVEL_INFO:  level_str = "I"; break;
        case LOG_LEVEL_WARN:  level_str = "W"; break;
        case LOG_LEVEL_ERROR: level_str = "E"; break;
        default: level_str = "?"; break;
    }
    
    Serial.printf("[%lu][%s] ", millis(), level_str);
    
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    Serial.println(buf);
}

#endif // ARDUINO_IMPL_H

