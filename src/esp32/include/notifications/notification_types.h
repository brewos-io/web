/**
 * BrewOS Notification Types
 * 
 * Simple notification system for user reminders and critical alerts.
 * Focus: Actionable notifications when user is away from machine.
 */

#ifndef NOTIFICATION_TYPES_H
#define NOTIFICATION_TYPES_H

#include <Arduino.h>
#include <cstdint>

// =============================================================================
// Notification Types (Keep it simple - only 7 types)
// =============================================================================

enum class NotificationType : uint8_t {
    // Reminders (1-9)
    MACHINE_READY  = 1,   // Machine reached brewing temp
    WATER_EMPTY    = 2,   // Water tank needs refill
    DESCALE_DUE    = 3,   // Time to descale
    SERVICE_DUE    = 4,   // Maintenance recommended
    BACKFLUSH_DUE  = 5,   // Backflush reminder
    
    // Alerts (10+)
    MACHINE_ERROR  = 10,  // Pico reported an error
    PICO_OFFLINE   = 11   // Control board disconnected
};

// =============================================================================
// Delivery Channels
// =============================================================================

enum NotificationChannel : uint8_t {
    CHANNEL_NONE      = 0x00,
    CHANNEL_WEBSOCKET = 0x01,  // Real-time UI
    CHANNEL_MQTT      = 0x02,  // Home automation
    CHANNEL_CLOUD     = 0x04,  // Push notifications
    CHANNEL_ALL       = 0x07
};

// =============================================================================
// Notification Structure
// =============================================================================

#define NOTIF_MESSAGE_LEN  64

struct Notification {
    NotificationType type;
    char message[NOTIF_MESSAGE_LEN];
    uint32_t timestamp;          // Unix timestamp
    bool is_alert;               // true = critical, requires acknowledgment
    bool acknowledged;           // User acknowledged this alert
    bool sent_push;              // Already sent to cloud/push
};

// =============================================================================
// Helper Functions
// =============================================================================

inline const char* getNotificationCode(NotificationType type) {
    switch (type) {
        case NotificationType::MACHINE_READY:  return "MACHINE_READY";
        case NotificationType::WATER_EMPTY:    return "WATER_EMPTY";
        case NotificationType::DESCALE_DUE:    return "DESCALE_DUE";
        case NotificationType::SERVICE_DUE:    return "SERVICE_DUE";
        case NotificationType::BACKFLUSH_DUE:  return "BACKFLUSH_DUE";
        case NotificationType::MACHINE_ERROR:  return "MACHINE_ERROR";
        case NotificationType::PICO_OFFLINE:   return "PICO_OFFLINE";
        default: return "UNKNOWN";
    }
}

inline bool isAlert(NotificationType type) {
    return static_cast<uint8_t>(type) >= 10;
}

#endif // NOTIFICATION_TYPES_H
