#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

// WiFi modes
enum class WiFiManagerMode {
    DISCONNECTED,
    AP_MODE,        // Access Point (setup mode)
    STA_MODE,       // Station (connected to router)
    STA_CONNECTING  // Trying to connect
};

// WiFi status for web UI
struct WiFiStatus {
    WiFiManagerMode mode;
    String ssid;
    String ip;
    int8_t rssi;
    bool configured;
};

// Time/NTP status
struct TimeStatus {
    bool ntpSynced;
    String currentTime;     // Formatted local time
    String timezone;        // Current timezone description
    int32_t utcOffset;      // Current UTC offset in seconds
};

class WiFiManager {
public:
    WiFiManager();
    
    // Initialize - tries STA if configured, falls back to AP
    void begin();
    
    // Update - call in loop
    void loop();
    
    // Configuration
    bool hasStoredCredentials();
    bool setCredentials(const String& ssid, const String& password);
    void clearCredentials();
    
    // Connect to stored WiFi
    bool connectToWiFi();
    
    // Start AP mode (setup)
    void startAP();
    
    // Get current status
    WiFiStatus getStatus();
    WiFiManagerMode getMode() { return _mode; }
    bool isAPMode() { return _mode == WiFiManagerMode::AP_MODE; }
    bool isConnected();
    String getIP();
    
    // NTP/Time configuration
    void configureNTP(const char* server, int16_t utcOffsetMinutes, bool dstEnabled, int16_t dstOffsetMinutes);
    void syncNTP();
    bool isTimeSynced();
    TimeStatus getTimeStatus();
    time_t getLocalTime();
    String getFormattedTime(const char* format = "%Y-%m-%d %H:%M:%S");
    
    // Events (set callbacks)
    void onConnected(std::function<void()> callback) { _onConnected = callback; }
    void onDisconnected(std::function<void()> callback) { _onDisconnected = callback; }
    void onAPStarted(std::function<void()> callback) { _onAPStarted = callback; }

private:
    WiFiManagerMode _mode;
    Preferences _prefs;
    
    String _storedSSID;
    String _storedPassword;
    
    unsigned long _lastConnectAttempt;
    unsigned long _connectStartTime;
    
    // NTP settings
    char _ntpServer[64] = "pool.ntp.org";
    int32_t _utcOffsetSec = 0;
    int32_t _dstOffsetSec = 0;
    bool _ntpConfigured = false;
    
    std::function<void()> _onConnected;
    std::function<void()> _onDisconnected;
    std::function<void()> _onAPStarted;
    
    void loadCredentials();
    void saveCredentials(const String& ssid, const String& password);
};

#endif // WIFI_MANAGER_H

