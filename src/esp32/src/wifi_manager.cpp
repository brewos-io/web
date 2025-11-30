#include "wifi_manager.h"
#include "config.h"

WiFiManager::WiFiManager() 
    : _mode(WiFiManagerMode::DISCONNECTED)
    , _lastConnectAttempt(0)
    , _connectStartTime(0)
    , _onConnected(nullptr)
    , _onDisconnected(nullptr)
    , _onAPStarted(nullptr) {
}

void WiFiManager::begin() {
    LOG_I("WiFi Manager starting...");
    
    // Load stored credentials
    loadCredentials();
    
    // Try to connect if we have credentials
    if (hasStoredCredentials()) {
        LOG_I("Found stored WiFi credentials for: %s", _storedSSID.c_str());
        connectToWiFi();
    } else {
        LOG_I("No stored credentials, starting AP mode");
        startAP();
    }
}

void WiFiManager::loop() {
    switch (_mode) {
        case WiFiManagerMode::STA_CONNECTING:
            // Check connection status
            if (WiFi.status() == WL_CONNECTED) {
                _mode = WiFiManagerMode::STA_MODE;
                LOG_I("WiFi connected! IP: %s", WiFi.localIP().toString().c_str());
                if (_onConnected) _onConnected();
            } 
            else if (millis() - _connectStartTime > WIFI_CONNECT_TIMEOUT_MS) {
                LOG_W("WiFi connection timeout, starting AP mode");
                startAP();
            }
            break;
            
        case WiFiManagerMode::STA_MODE:
            // Check if still connected
            if (WiFi.status() != WL_CONNECTED) {
                LOG_W("WiFi disconnected");
                _mode = WiFiManagerMode::DISCONNECTED;
                if (_onDisconnected) _onDisconnected();
                
                // Try to reconnect after interval
                if (millis() - _lastConnectAttempt > WIFI_RECONNECT_INTERVAL) {
                    connectToWiFi();
                }
            }
            break;
            
        case WiFiManagerMode::AP_MODE:
            // AP mode is stable, nothing to do
            break;
            
        case WiFiManagerMode::DISCONNECTED:
            // Try to reconnect if we have credentials
            if (hasStoredCredentials() && 
                millis() - _lastConnectAttempt > WIFI_RECONNECT_INTERVAL) {
                connectToWiFi();
            }
            break;
    }
}

bool WiFiManager::hasStoredCredentials() {
    return _storedSSID.length() > 0 && _storedPassword.length() > 0;
}

bool WiFiManager::setCredentials(const String& ssid, const String& password) {
    if (ssid.length() == 0 || password.length() < 8) {
        LOG_E("Invalid credentials");
        return false;
    }
    
    saveCredentials(ssid, password);
    _storedSSID = ssid;
    _storedPassword = password;
    
    LOG_I("Credentials saved for: %s", ssid.c_str());
    return true;
}

void WiFiManager::clearCredentials() {
    _prefs.begin("wifi", false);
    _prefs.clear();
    _prefs.end();
    
    _storedSSID = "";
    _storedPassword = "";
    
    LOG_I("Credentials cleared");
}

bool WiFiManager::connectToWiFi() {
    if (!hasStoredCredentials()) {
        LOG_E("No credentials to connect with");
        return false;
    }
    
    LOG_I("Connecting to WiFi: %s", _storedSSID.c_str());
    
    // Stop AP if running
    WiFi.softAPdisconnect(true);
    
    // Set mode and connect
    WiFi.mode(WIFI_STA);
    WiFi.begin(_storedSSID.c_str(), _storedPassword.c_str());
    
    _mode = WiFiManagerMode::STA_CONNECTING;
    _connectStartTime = millis();
    _lastConnectAttempt = millis();
    
    return true;
}

void WiFiManager::startAP() {
    LOG_I("Starting AP mode: %s", WIFI_AP_SSID);
    
    // Disconnect from any station
    WiFi.disconnect(true);
    
    // Configure AP
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(WIFI_AP_IP, WIFI_AP_GATEWAY, WIFI_AP_SUBNET);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL, false, WIFI_AP_MAX_CONNECTIONS);
    
    _mode = WiFiManagerMode::AP_MODE;
    
    LOG_I("AP started. IP: %s", WiFi.softAPIP().toString().c_str());
    
    if (_onAPStarted) _onAPStarted();
}

WiFiStatus WiFiManager::getStatus() {
    WiFiStatus status;
    status.mode = _mode;
    status.configured = hasStoredCredentials();
    
    switch (_mode) {
        case WiFiManagerMode::AP_MODE:
            status.ssid = WIFI_AP_SSID;
            status.ip = WiFi.softAPIP().toString();
            status.rssi = 0;
            break;
            
        case WiFiManagerMode::STA_MODE:
            status.ssid = WiFi.SSID();
            status.ip = WiFi.localIP().toString();
            status.rssi = WiFi.RSSI();
            break;
            
        case WiFiManagerMode::STA_CONNECTING:
            status.ssid = _storedSSID;
            status.ip = "Connecting...";
            status.rssi = 0;
            break;
            
        default:
            status.ssid = "";
            status.ip = "";
            status.rssi = 0;
    }
    
    return status;
}

bool WiFiManager::isConnected() {
    return _mode == WiFiManagerMode::STA_MODE || _mode == WiFiManagerMode::AP_MODE;
}

String WiFiManager::getIP() {
    if (_mode == WiFiManagerMode::AP_MODE) {
        return WiFi.softAPIP().toString();
    } else if (_mode == WiFiManagerMode::STA_MODE) {
        return WiFi.localIP().toString();
    }
    return "";
}

void WiFiManager::loadCredentials() {
    _prefs.begin("wifi", true);  // Read-only
    _storedSSID = _prefs.getString("ssid", "");
    _storedPassword = _prefs.getString("password", "");
    _prefs.end();
}

void WiFiManager::saveCredentials(const String& ssid, const String& password) {
    _prefs.begin("wifi", false);  // Read-write
    _prefs.putString("ssid", ssid);
    _prefs.putString("password", password);
    _prefs.end();
}

// =============================================================================
// NTP/Time Functions
// =============================================================================

void WiFiManager::configureNTP(const char* server, int16_t utcOffsetMinutes, bool dstEnabled, int16_t dstOffsetMinutes) {
    strncpy(_ntpServer, server, sizeof(_ntpServer) - 1);
    _utcOffsetSec = utcOffsetMinutes * 60;
    _dstOffsetSec = dstEnabled ? (dstOffsetMinutes * 60) : 0;
    _ntpConfigured = true;
    
    LOG_I("NTP configured: server=%s, UTC offset=%d min, DST=%s (%d min)",
          _ntpServer, utcOffsetMinutes, dstEnabled ? "on" : "off", dstOffsetMinutes);
    
    // Apply configuration immediately if WiFi connected
    if (_mode == WiFiManagerMode::STA_MODE) {
        syncNTP();
    }
}

void WiFiManager::syncNTP() {
    if (!_ntpConfigured) {
        // Use defaults if not configured
        configureNTP("pool.ntp.org", 0, false, 0);
    }
    
    LOG_I("Configuring NTP: %s (UTC%+d)", _ntpServer, _utcOffsetSec / 3600);
    
    // Configure time with timezone offset
    // ESP32 uses POSIX timezone format: GMT-offset (inverted from what you'd expect)
    // For UTC+2: "UTC-2" because POSIX uses the opposite sign
    char tzStr[32];
    int hours = _utcOffsetSec / 3600;
    int mins = abs((_utcOffsetSec % 3600) / 60);
    
    if (_dstOffsetSec > 0) {
        // With DST - format: STD-offset DST for simple rule
        snprintf(tzStr, sizeof(tzStr), "UTC%+d", -hours);
    } else {
        snprintf(tzStr, sizeof(tzStr), "UTC%+d", -hours);
    }
    
    // Set timezone and NTP server
    configTzTime(tzStr, _ntpServer);
    
    LOG_I("NTP sync started, timezone: %s", tzStr);
}

bool WiFiManager::isTimeSynced() {
    time_t now = time(nullptr);
    // If time is after 2020, consider it synced (NTP provides epoch time)
    return now > 1577836800;  // Jan 1, 2020
}

TimeStatus WiFiManager::getTimeStatus() {
    TimeStatus status;
    status.ntpSynced = isTimeSynced();
    status.utcOffset = _utcOffsetSec + _dstOffsetSec;
    
    if (status.ntpSynced) {
        status.currentTime = getFormattedTime();
        
        // Format timezone string
        int totalOffsetMin = status.utcOffset / 60;
        int hours = totalOffsetMin / 60;
        int mins = abs(totalOffsetMin % 60);
        char tz[16];
        if (mins > 0) {
            snprintf(tz, sizeof(tz), "UTC%+d:%02d", hours, mins);
        } else {
            snprintf(tz, sizeof(tz), "UTC%+d", hours);
        }
        status.timezone = tz;
    } else {
        status.currentTime = "Not synced";
        status.timezone = "Unknown";
    }
    
    return status;
}

time_t WiFiManager::getLocalTime() {
    return time(nullptr);
}

String WiFiManager::getFormattedTime(const char* format) {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    if (!timeinfo) {
        return "Invalid";
    }
    
    char buffer[64];
    strftime(buffer, sizeof(buffer), format, timeinfo);
    return String(buffer);
}

