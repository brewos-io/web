/**
 * ECM Coffee Machine Controller - ESP32-S3 Display Module
 * 
 * Main firmware for the ESP32-S3 that provides:
 * - WiFi connectivity (AP setup mode + STA mode)
 * - Web interface for monitoring and configuration
 * - UART bridge to Pico control board
 * - OTA firmware updates for Pico
 */

#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "pico_uart.h"

// Global instances
WiFiManager wifiManager;
PicoUART picoUart(Serial1);
WebServer webServer(wifiManager, picoUart);

// Timing
unsigned long lastPing = 0;
unsigned long lastStatusBroadcast = 0;

void setup() {
    // Initialize debug serial (USB)
    Serial.begin(DEBUG_BAUD);
    delay(1000);  // Wait for USB serial
    
    LOG_I("========================================");
    LOG_I("ECM Controller ESP32-S3 v%s", ESP32_VERSION);
    LOG_I("========================================");
    LOG_I("Free heap: %d bytes", ESP.getFreeHeap());
    
    // Initialize Pico UART
    picoUart.begin();
    
    // Set up packet handler
    picoUart.onPacket([](const PicoPacket& packet) {
        LOG_D("Pico packet: type=0x%02X, len=%d, seq=%d", 
              packet.type, packet.length, packet.seq);
        
        // Forward to WebSocket clients
        webServer.broadcastPicoMessage(packet.type, packet.payload, packet.length);
        
        // Handle specific message types
        switch (packet.type) {
            case MSG_BOOT:
                LOG_I("Pico booted!");
                webServer.broadcastLog("Pico booted", "info");
                break;
                
            case MSG_STATUS: {
                // Parse and forward status
                // Status payload is sent as-is to web clients
                break;
            }
            
            case MSG_ALARM: {
                // Alarm - log prominently
                uint8_t alarmCode = packet.payload[0];
                LOG_W("PICO ALARM: 0x%02X", alarmCode);
                webServer.broadcastLog("Pico ALARM: " + String(alarmCode, HEX), "error");
                break;
            }
            
            case MSG_CONFIG:
                LOG_I("Received config from Pico");
                webServer.broadcastLog("Config received from Pico", "info");
                break;
                
            case MSG_ENV_CONFIG: {
                // Environmental configuration (voltage, current limits)
                if (packet.length >= 18) {  // env_config_payload_t size
                    uint16_t voltage = packet.payload[0] | (packet.payload[1] << 8);
                    float max_current = 0;
                    memcpy(&max_current, &packet.payload[2], sizeof(float));
                    LOG_I("Env config: %dV, %.1fA max", voltage, max_current);
                    webServer.broadcastLog("Env config: " + String(voltage) + "V, " + String(max_current, 1) + "A max", "info");
                }
                break;
            }
                
            case MSG_DEBUG_RESP:
                LOG_D("Debug response from Pico");
                break;
                
            default:
                LOG_D("Unknown packet type: 0x%02X", packet.type);
        }
    });
    
    // Initialize WiFi
    wifiManager.onConnected([]() {
        LOG_I("WiFi connected!");
        webServer.broadcastLog("WiFi connected: " + wifiManager.getIP(), "info");
    });
    
    wifiManager.onDisconnected([]() {
        LOG_W("WiFi disconnected");
    });
    
    wifiManager.onAPStarted([]() {
        LOG_I("AP mode started - connect to: %s", WIFI_AP_SSID);
        LOG_I("Open http://%s to configure", WIFI_AP_IP.toString().c_str());
    });
    
    wifiManager.begin();
    
    // Start web server
    webServer.begin();
    
    LOG_I("Setup complete. Free heap: %d bytes", ESP.getFreeHeap());
}

void loop() {
    // Update WiFi manager
    wifiManager.loop();
    
    // Process Pico UART
    picoUart.loop();
    
    // Update web server
    webServer.loop();
    
    // Periodic ping to Pico
    if (millis() - lastPing > 5000) {
        lastPing = millis();
        if (picoUart.isConnected() || picoUart.getPacketsReceived() == 0) {
            picoUart.sendPing();
        }
    }
    
    // Periodic status broadcast to WebSocket clients
    if (millis() - lastStatusBroadcast > 1000) {
        lastStatusBroadcast = millis();
        
        // Only if we have connected clients
        if (webServer.getClientCount() > 0) {
            // Build status JSON
            String status = "{\"type\":\"esp_status\",";
            status += "\"uptime\":" + String(millis()) + ",";
            status += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
            status += "\"wifi\":\"" + wifiManager.getIP() + "\",";
            status += "\"picoConnected\":" + String(picoUart.isConnected() ? "true" : "false") + ",";
            status += "\"picoPackets\":" + String(picoUart.getPacketsReceived()) + ",";
            status += "\"picoErrors\":" + String(picoUart.getPacketErrors()) + "}";
            
            webServer.broadcastStatus(status);
        }
    }
}

