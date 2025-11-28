# ESP32 Firmware Implementation Plan

> **Document Version:** 1.0  
> **Last Updated:** 2025-11-28  
> **Status:** Planning / In Development

## Overview

The ESP32-S3 serves as the connectivity and UI hub for the BrewOS coffee machine controller. It bridges the Raspberry Pi Pico (which handles real-time control) with the user, providing WiFi, web interface, mobile app connectivity, cloud integration, and advanced features like brew-by-weight.

## Current Implementation Status

| Feature | Status | Notes |
|---------|--------|-------|
| WiFi AP Mode | ✅ Complete | `BrewOS-Setup` access point |
| WiFi STA Mode | ✅ Complete | Connect to home network |
| UART Bridge to Pico | ✅ Complete | 921600 baud, CRC-16 packets |
| Basic Web Server | ✅ Complete | LittleFS static files |
| WebSocket Status | ✅ Complete | Real-time status updates |
| OTA Pico Update | ✅ Complete | Firmware streaming |
| Basic Dashboard UI | ✅ Complete | Temperature, pressure display |

---

## 1. Hardware Integration

### 1.1 Display Module Specifications

**Model:** UEDX48480021-MD80E (ESP32-S3 Knob Display)

| Specification | Value |
|---------------|-------|
| Screen Size | 2.1" Round (φ80mm) |
| Resolution | 480 × 480 pixels |
| Panel Type | IPS |
| Brightness | 300 cd/m² |
| MCU | ESP32-S3 |
| CPU Frequency | 160 MHz |
| RAM | 8 MB |
| Flash | 16 MB |
| WiFi | ✅ Supported |
| Bluetooth | ✅ Supported |
| Interface | UART |
| Input | Rotary Encoder (Knob) + Push Button |
| Graphics | LVGL |
| Voltage | 5V |
| Current | 100 mA |
| Operating Temp | -20°C to 70°C |

### 1.2 Pin Configuration

| GPIO | Function | Status |
|------|----------|--------|
| 17 | UART TX → Pico RX | ✅ Working |
| 18 | UART RX ← Pico TX | ✅ Working |
| 8 | PICO_RUN (Reset) | ✅ Working |
| 9 | PICO_BOOTSEL | ✅ Working |
| 10 | WEIGHT_STOP | ✅ Defined |
| TBD | Encoder A (CLK) | 🔲 To Configure |
| TBD | Encoder B (DT) | 🔲 To Configure |
| TBD | Encoder Button (SW) | 🔲 To Configure |
| TBD | Display SPI (MOSI/CLK/CS/DC) | 🔲 Built-in |
| TBD | Backlight PWM | 🔲 To Configure |

### 1.3 Display Features

The round 480x480 IPS display is perfect for:
- **Circular gauges** (temperature, pressure)
- **Radial menus** navigated by rotary encoder
- **Shot timer** with large central display
- **Status ring** around the perimeter

### 1.4 Input Controls

```
┌─────────────────────────────────────┐
│                                     │
│    ┌───────────────────────┐        │
│    │                       │        │
│    │    480×480 Round      │  ◄──── Rotary Knob
│    │    IPS Display        │        (Rotate: Navigate)
│    │                       │        (Push: Select/Confirm)
│    └───────────────────────┘        │
│                                     │
└─────────────────────────────────────┘

Controls:
• Rotate Clockwise     → Next item / Increase value
• Rotate Counter-CW    → Previous item / Decrease value  
• Short Press          → Select / Confirm
• Long Press (2s)      → Back / Cancel
• Double Press         → Quick action (e.g., tare scale)
```

### 1.5 Implementation Tasks

```
[ ] HW-1: Configure LVGL for 480x480 round display
[ ] HW-2: Initialize display driver (likely GC9A01 or ST7701)
[ ] HW-3: Implement rotary encoder with debouncing
[ ] HW-4: Button press detection (short/long/double)
[ ] HW-5: Backlight PWM control (brightness/dimming)
[ ] HW-6: Display sleep mode (dim after idle)
[ ] HW-7: ESP32 OTA self-update capability
[ ] HW-8: Hardware watchdog configuration
```

---

## 2. MQTT Integration

### 2.1 Overview

MQTT provides lightweight pub/sub messaging for home automation integration (Home Assistant, Node-RED, etc.).

### 2.2 Topic Structure

```
brewos/{device_id}/status          # Machine status (retained)
brewos/{device_id}/sensors         # Real-time sensor data
brewos/{device_id}/command         # Incoming commands
brewos/{device_id}/config          # Configuration (retained)
brewos/{device_id}/availability    # Online/offline (LWT)
brewos/{device_id}/shot            # Shot data (brew events)
brewos/{device_id}/statistics      # Usage statistics
```

### 2.3 Home Assistant Discovery

Auto-discovery for seamless Home Assistant integration:

```json
{
  "device": {
    "identifiers": ["brewos_XXXX"],
    "name": "BrewOS Coffee Machine",
    "model": "ECM Dual Boiler",
    "manufacturer": "BrewOS"
  },
  "availability_topic": "brewos/XXXX/availability",
  "state_topic": "brewos/XXXX/status"
}
```

### 2.4 Features

| Feature | Description | Priority |
|---------|-------------|----------|
| Status Publishing | Temps, pressure, state | High |
| Command Subscription | Set temps, machine mode | High |
| Home Assistant Discovery | Auto-configuration | High |
| Shot Data Publishing | Brew metrics per shot | Medium |
| QoS 1 for Commands | Guaranteed delivery | Medium |
| TLS/SSL Support | Secure connection | Medium |
| Last Will Testament | Offline detection | High |

### 2.5 Implementation Tasks

```
[ ] MQTT-1: Add PubSubClient or AsyncMqttClient library
[ ] MQTT-2: MQTT broker configuration (host, port, credentials)
[ ] MQTT-3: Implement status publishing (1s interval)
[ ] MQTT-4: Implement command subscription
[ ] MQTT-5: Home Assistant MQTT discovery
[ ] MQTT-6: TLS/SSL support for secure brokers
[ ] MQTT-7: Reconnection with exponential backoff
[ ] MQTT-8: Shot data publishing (brew events)
```

### 2.6 Configuration (NVS Storage)

```cpp
struct MQTTConfig {
    bool enabled;
    char broker[64];
    uint16_t port;
    char username[32];
    char password[64];
    char client_id[32];
    char topic_prefix[32];
    bool use_tls;
    bool ha_discovery;
};
```

---

## 3. Web API

### 3.1 Current Endpoints

| Method | Path | Description | Status |
|--------|------|-------------|--------|
| GET | `/api/status` | Machine status | ✅ |
| GET | `/api/wifi/networks` | Scan WiFi | ✅ |
| POST | `/api/wifi/connect` | Connect to WiFi | ✅ |
| GET | `/api/config` | Get config | ✅ |
| POST | `/api/command` | Send command | ✅ |
| POST | `/api/ota/upload` | Upload firmware | ✅ |
| POST | `/api/ota/start` | Flash Pico | ✅ |
| POST | `/api/pico/reset` | Reset Pico | ✅ |

### 3.2 TODO: New Endpoints

#### Machine Control
```
POST /api/temp/brew             # Set brew temperature
POST /api/temp/steam            # Set steam temperature
POST /api/mode                  # Set machine mode (idle/standby)
```

> **Note:** Brew start/stop is controlled by the physical lever on the machine, not via API.
> The ESP32 monitors brewing state from the Pico but does not control it.

#### Configuration
```
GET  /api/config/all            # Full configuration
POST /api/config/pid            # Set PID parameters
POST /api/config/preinfusion    # Pre-infusion settings
POST /api/config/environmental  # Voltage/current config
POST /api/config/schedule       # Power-on schedule
```

#### Statistics & History
```
GET  /api/stats                 # Usage statistics
GET  /api/shots                 # Recent shot history
GET  /api/shots/{id}            # Specific shot details
DELETE /api/shots               # Clear shot history
```

#### MQTT Configuration
```
GET  /api/mqtt/config           # Get MQTT settings
POST /api/mqtt/config           # Set MQTT settings
POST /api/mqtt/test             # Test MQTT connection
```

#### Cloud Integration
```
GET  /api/cloud/status          # Cloud connection status
POST /api/cloud/link            # Link to cloud account
POST /api/cloud/unlink          # Unlink from cloud
```

#### Brew by Weight
```
GET  /api/scale/status          # Scale connection status
POST /api/scale/pair            # Pair BLE scale
POST /api/scale/tare            # Tare scale
POST /api/weight/target         # Set target weight
```

### 3.3 Authentication

```
[ ] API-AUTH-1: API key authentication (header or query param)
[ ] API-AUTH-2: Session-based authentication for web UI
[ ] API-AUTH-3: Rate limiting
[ ] API-AUTH-4: CORS configuration
```

### 3.4 Implementation Tasks

```
[ ] API-1: Temperature setpoint endpoints
[ ] API-2: Full configuration GET/POST
[ ] API-3: Statistics and shot history endpoints
[ ] API-4: MQTT configuration endpoints
[ ] API-5: API authentication middleware
[ ] API-6: Request validation and error responses
[ ] API-7: API documentation (OpenAPI/Swagger)
```

> **Note:** Brew control is via physical lever - ESP32 only monitors brewing state.

---

## 4. Display UI (LVGL)

### 4.1 LVGL Framework

The device uses **LVGL (Light and Versatile Graphics Library)** for the embedded display UI. This is separate from the web UI and runs directly on the ESP32-S3.

### 4.2 Round Display Design

The 480×480 circular display lends itself to unique UI patterns:

```
         ┌─────────────────┐
        ╱                   ╲
       │   ╭─────────────╮   │
       │   │  93.5°C     │   │  ← Main value (large)
       │   │  BREW       │   │  ← Label
       │   ╰─────────────╯   │
       │                     │
       │  ○ ○ ○ ● ○ ○ ○     │  ← Page indicators
        ╲                   ╱
         └─────────────────┘
              ▲
         Status Ring (temperature/progress arc)
```

### 4.3 Display Screens

| Screen | Description | Navigation |
|--------|-------------|------------|
| **Home** | Brew/Steam temps, pressure, status | Default view |
| **Brew** | Shot timer, weight, flow rate | During brewing |
| **Temperature** | Temp adjustment with arc gauge | Rotate to adjust |
| **Settings** | WiFi, MQTT, Cloud, Scale config | Radial menu |
| **Statistics** | Shot count, usage graphs | Scroll through |
| **Maintenance** | Cleaning mode, reminders | Submenus |

### 4.4 Home Screen Layout

```
        ╭──────────────────────╮
       ╱  ▓▓▓▓▓▓▓▓░░░░░░░░░░░░  ╲   ← Status arc (temp progress)
      │                          │
      │      🔥 93.5°C           │   ← Brew temperature
      │        BREW              │
      │                          │
      │      💨 145°C            │   ← Steam temperature
      │        STEAM             │
      │                          │
      │    ══════════════════    │   ← Pressure bar
      │         9.1 bar          │
       ╲                        ╱
        ╰──────────────────────╯
              ● ○ ○ ○              ← Page indicator
```

### 4.5 Brewing Screen

```
        ╭──────────────────────╮
       ╱ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░ ╲   ← Weight progress arc
      │                          │
      │         ⏱️                │
      │       00:25              │   ← Shot timer (large)
      │                          │
      │    ⚖️ 28.4g / 36g        │   ← Weight / Target
      │                          │
      │    💧 2.1 ml/s           │   ← Flow rate
      │                          │
       ╲      [STOP]            ╱    ← Press knob to stop
        ╰──────────────────────╯
```

### 4.6 Radial Settings Menu

```
        ╭──────────────────────╮
       ╱         WiFi           ╲
      │    ╱           ╲         │
      │ Scale    ●    MQTT       │   ← Rotate to select
      │    ╲           ╱         │
      │        Cloud             │
       ╲                        ╱
        ╰──────────────────────╯
         Press to enter submenu
```

### 4.7 LVGL Implementation Tasks

```
[ ] LVGL-1: LVGL initialization and display driver
[ ] LVGL-2: Custom theme (coffee colors, dark mode)
[ ] LVGL-3: Circular arc gauge component
[ ] LVGL-4: Home screen with dual temp display
[ ] LVGL-5: Brewing screen with timer and weight
[ ] LVGL-6: Radial menu navigation
[ ] LVGL-7: Temperature adjustment screen
[ ] LVGL-8: Settings screens (WiFi, MQTT, etc.)
[ ] LVGL-9: Statistics/history screen
[ ] LVGL-10: Encoder input integration
[ ] LVGL-11: Screen transitions and animations
[ ] LVGL-12: Idle timeout and screen dimming
```

### 4.8 Color Theme

```cpp
// Coffee-inspired dark theme
#define COLOR_BG_DARK       0x1A1512  // Dark brown/black
#define COLOR_BG_CARD       0x2D2420  // Card background
#define COLOR_ACCENT_AMBER  0xD4A574  // Warm amber
#define COLOR_ACCENT_ORANGE 0xE85D04  // Brewing orange
#define COLOR_TEMP_BREW     0x3B82F6  // Blue for brew temp
#define COLOR_TEMP_STEAM    0xEF4444  // Red for steam temp
#define COLOR_SUCCESS       0x22C55E  // Green (ready)
#define COLOR_WARNING       0xF59E0B  // Yellow (heating)
#define COLOR_ERROR         0xDC2626  // Red (alarm)
#define COLOR_TEXT_PRIMARY  0xFAFAFA  // White text
#define COLOR_TEXT_MUTED    0x9CA3AF  // Gray text
```

---

## 5. Web UI (Remote Access)

### 5.1 Overview

The web UI provides remote access via WiFi, complementing the on-device LVGL display.

### 5.2 Current Features

- Dashboard with temperature display
- WiFi setup interface
- Pico status and control
- OTA firmware update
- Console log viewer

### 5.3 Web UI Enhancements

#### Visual Design
- Modern, coffee-themed aesthetic matching device UI
- Dark mode with warm amber accents
- Responsive design (mobile-first)
- Smooth animations and transitions

#### Dashboard Features
- [ ] Real-time temperature graphs
- [ ] Pressure gauge visualization
- [ ] Shot timer display
- [ ] Brew progress indicator
- [ ] Scale weight display

#### Configuration Pages
- [ ] PID tuning interface
- [ ] Temperature profiles
- [ ] Pre-infusion settings
- [ ] Network settings (WiFi, MQTT, Cloud)
- [ ] Scale pairing

#### Statistics Dashboard
- [ ] Shot history with graphs
- [ ] Usage patterns
- [ ] Maintenance reminders

### 5.4 Technology Stack

```
- HTML5 / CSS3 (CSS Variables)
- Vanilla JavaScript (lightweight)
- Chart.js for graphs
- PWA support (offline capable)
```

### 5.5 Implementation Tasks

```
[ ] WEB-1: Design system matching LVGL theme
[ ] WEB-2: Real-time temperature chart
[ ] WEB-3: Shot timer component
[ ] WEB-4: Settings pages
[ ] WEB-5: Statistics dashboard
[ ] WEB-6: PWA manifest and service worker
[ ] WEB-7: Mobile responsiveness
```

---

## 6. Cloud Integration

### 6.1 Overview

Connect to a dedicated BrewOS cloud server for:
- Remote monitoring and control
- Shot history backup
- Firmware update notifications
- Community features (sharing profiles)
- Multi-device management

### 6.2 Connection Architecture

```
┌─────────────┐     HTTPS/WSS     ┌─────────────────┐
│   ESP32     │◄──────────────────│  BrewOS Cloud   │
│  (Device)   │                   │    Server       │
└─────────────┘                   └─────────────────┘
       │                                  │
       │  WebSocket                       │  REST API
       │  (real-time)                     │  (mobile app)
       ▼                                  ▼
┌─────────────┐                   ┌─────────────────┐
│    Pico     │                   │   Mobile App    │
│ (Controller)│                   │   (Future)      │
└─────────────┘                   └─────────────────┘
```

### 6.3 Cloud Features

| Feature | Description | Priority |
|---------|-------------|----------|
| Device Registration | Link device to account | High |
| Real-time Status | Live machine status | High |
| Remote Control | Control from anywhere | High |
| Shot History Sync | Backup shot data | Medium |
| Firmware Updates | Check & notify updates | Medium |
| Profile Sharing | Community shot profiles | Low |
| Multi-device | Manage multiple machines | Low |

### 6.4 Security

```
- TLS 1.3 for all connections
- Device certificate authentication
- JWT tokens for API access
- End-to-end encryption for sensitive data
- OAuth2 for cloud account linking
```

### 6.5 Protocol

```json
// Device → Cloud: Status Update
{
  "type": "status",
  "device_id": "brewos_XXXX",
  "timestamp": 1234567890,
  "data": {
    "brew_temp": 93.5,
    "steam_temp": 145.2,
    "pressure": 9.1,
    "state": "ready",
    "wifi_rssi": -45
  }
}

// Cloud → Device: Command
{
  "type": "command",
  "cmd": "set_temp",
  "params": {
    "boiler": "brew",
    "temp": 94.0
  }
}
```

### 6.6 Implementation Tasks

```
[ ] CLOUD-1: HTTPS client with certificate pinning
[ ] CLOUD-2: WebSocket client for real-time comms
[ ] CLOUD-3: Device registration flow
[ ] CLOUD-4: Authentication token management
[ ] CLOUD-5: Status publishing to cloud
[ ] CLOUD-6: Command reception from cloud
[ ] CLOUD-7: Shot history sync
[ ] CLOUD-8: Firmware update check & notification
[ ] CLOUD-9: Connection resilience (reconnect, offline queue)
[ ] CLOUD-10: Cloud configuration UI
```

### 6.7 Cloud Server Endpoints (ESP32 Client)

```
POST /api/devices/register      # Register new device
POST /api/devices/auth          # Authenticate device
WS   /ws/device                 # Real-time WebSocket
POST /api/shots                 # Upload shot data
GET  /api/firmware/check        # Check for updates
```

---

## 7. Brew by Weight (Bluetooth Scale)

### 7.1 Overview

Integrate Bluetooth Low Energy (BLE) scales for precise brew-by-weight control. When target weight is reached, ESP32 signals Pico via `WEIGHT_STOP_PIN` (GPIO10).

### 7.2 Supported Scales

| Scale | Protocol | Status |
|-------|----------|--------|
| Acaia Lunar | Acaia BLE | Planned |
| Acaia Pearl | Acaia BLE | Planned |
| Felicita Arc | Felicita BLE | Planned |
| Decent Scale | Open protocol | Planned |
| Generic HX711 | Custom BLE | Planned |

### 7.3 BLE Architecture

```
┌─────────────┐     BLE GATT      ┌─────────────┐
│   ESP32     │◄──────────────────│   Scale     │
│   (Central) │    Notifications   │(Peripheral) │
└─────────────┘                   └─────────────┘
       │
       │ GPIO10 (WEIGHT_STOP)
       ▼
┌─────────────┐
│    Pico     │ ──► Stops pump when HIGH
└─────────────┘
```

### 7.4 Features

| Feature | Description | Priority |
|---------|-------------|----------|
| Scale Discovery | Scan and find scales | High |
| Scale Pairing | Remember paired scale | High |
| Weight Reading | Real-time weight (10Hz) | High |
| Tare Function | Zero the scale | High |
| Target Weight | Auto-stop at target | High |
| Flow Rate | Calculate ml/s | Medium |
| Weight Curve | Log weight over time | Medium |
| Battery Level | Scale battery status | Low |
| Multi-scale | Support multiple scales | Low |

### 7.5 Brew-by-Weight Flow

```
1. User sets target weight (e.g., 36g)
2. User starts brew (pulls physical lever)
3. ESP32 detects brewing state from Pico and starts monitoring scale weight
4. Weight data streamed to UI in real-time
5. When weight >= (target - offset), ESP32 sets WEIGHT_STOP HIGH
6. Pico receives signal and stops pump
7. Shot data logged with final weight
```

### 7.6 Weight Stop Timing

```
- Compensate for drip time after pump stops
- Configurable offset (default: 2-3g before target)
- Learn offset from previous shots
- Account for scale response latency
```

### 7.7 BLE Service UUIDs (Acaia Example)

```cpp
// Acaia Scale Service
#define ACAIA_SERVICE_UUID     "00001820-0000-1000-8000-00805f9b34fb"
#define ACAIA_CHAR_UUID        "00002a80-0000-1000-8000-00805f9b34fb"

// Commands
#define ACAIA_CMD_TARE         0x07
#define ACAIA_CMD_START_TIMER  0x0D
#define ACAIA_CMD_STOP_TIMER   0x0E
```

### 7.8 Implementation Tasks

```
[ ] SCALE-1: BLE scanning for known scale manufacturers
[ ] SCALE-2: Acaia protocol implementation
[ ] SCALE-3: Felicita protocol implementation
[ ] SCALE-4: Scale pairing and NVS storage
[ ] SCALE-5: Real-time weight reading (notifications)
[ ] SCALE-6: Tare command implementation
[ ] SCALE-7: Target weight stop logic with offset
[ ] SCALE-8: WEIGHT_STOP_PIN signal to Pico
[ ] SCALE-9: Weight curve logging
[ ] SCALE-10: Flow rate calculation
[ ] SCALE-11: UI integration (weight display, target input)
[ ] SCALE-12: Scale status in web UI
```

### 7.9 Configuration

```cpp
struct ScaleConfig {
    bool enabled;
    char paired_address[18];    // "XX:XX:XX:XX:XX:XX"
    uint8_t scale_type;         // ACAIA, FELICITA, etc.
    float target_weight;        // grams
    float stop_offset;          // grams before target
    bool auto_tare;             // Tare when brew starts
    bool learn_offset;          // Adjust offset from history
};
```

---

## Implementation Priority Matrix

### Phase 1: Core Connectivity (Weeks 1-2)
- [ ] MQTT-1 through MQTT-5 (Basic MQTT)
- [ ] API-1 through API-3 (Control endpoints)
- [ ] UI-1 through UI-4 (Dashboard improvements)

### Phase 2: Advanced Features (Weeks 3-4)
- [ ] SCALE-1 through SCALE-8 (Brew by Weight core)
- [ ] MQTT-6 through MQTT-8 (MQTT polish)
- [ ] API-4 through API-6 (Stats, auth)

### Phase 3: Cloud Integration (Weeks 5-6)
- [ ] CLOUD-1 through CLOUD-6 (Cloud connectivity)
- [ ] UI-5 through UI-8 (Settings, stats pages)
- [ ] SCALE-9 through SCALE-12 (Scale polish)

### Phase 4: Polish & Optimization (Weeks 7-8)
- [ ] CLOUD-7 through CLOUD-10 (Cloud features)
- [ ] UI-9 through UI-11 (UX polish)
- [ ] HW-1 through HW-5 (Hardware enhancements)
- [ ] API-7, API-8 (Documentation)

---

## Memory Budget

The UEDX48480021-MD80E has **8MB PSRAM** and **16MB Flash**, providing ample resources.

| Component | Estimated RAM | Notes |
|-----------|--------------|-------|
| LVGL Core | ~64 KB | Graphics library |
| LVGL Display Buffer | ~461 KB | 480×480×2 (16-bit, 1 buffer) |
| LVGL Widgets | ~20 KB | UI components |
| WiFi Stack | ~40 KB | ESP-IDF managed |
| Web Server | ~20 KB | AsyncWebServer + WebSocket |
| MQTT Client | ~5 KB | PubSubClient |
| BLE Stack | ~50 KB | NimBLE preferred |
| JSON Buffers | ~8 KB | ArduinoJson |
| UART Buffers | ~2 KB | Pico communication |
| Cloud Client | ~10 KB | HTTPS + WebSocket |
| **Total** | **~680 KB** | Uses PSRAM for display buffer |

### PSRAM Usage

```cpp
// Allocate LVGL display buffer in PSRAM
#define LV_MEM_CUSTOM      1
#define LV_MEM_SIZE        (128 * 1024)  // 128KB for LVGL heap

// Display buffer in PSRAM (allows full frame buffer)
static lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(
    LV_HOR_RES_MAX * LV_VER_RES_MAX * sizeof(lv_color_t), 
    MALLOC_CAP_SPIRAM
);
```

---

## File Structure

```
src/esp32/
├── include/
│   ├── config.h              # Pin definitions, constants
│   ├── wifi_manager.h        # WiFi AP/STA management
│   ├── web_server.h          # HTTP + WebSocket server
│   ├── pico_uart.h           # Pico communication
│   ├── mqtt_client.h         # MQTT integration (NEW)
│   ├── cloud_client.h        # Cloud server client (NEW)
│   ├── ble_scale.h           # BLE scale integration (NEW)
│   ├── shot_logger.h         # Shot data logging (NEW)
│   ├── nvs_config.h          # NVS configuration storage (NEW)
│   │
│   ├── display/              # Display subsystem (NEW)
│   │   ├── display.h         # Display driver interface
│   │   ├── lvgl_port.h       # LVGL porting layer
│   │   ├── encoder.h         # Rotary encoder driver
│   │   └── theme.h           # Coffee theme colors
│   │
│   └── ui/                   # LVGL UI screens (NEW)
│       ├── ui.h              # UI manager
│       ├── screen_home.h     # Home dashboard
│       ├── screen_brew.h     # Brewing screen
│       ├── screen_settings.h # Settings menu
│       └── screen_stats.h    # Statistics screen
│
├── src/
│   ├── main.cpp              # Entry point, main loop
│   ├── wifi_manager.cpp
│   ├── web_server.cpp
│   ├── pico_uart.cpp
│   ├── mqtt_client.cpp       # (NEW)
│   ├── cloud_client.cpp      # (NEW)
│   ├── ble_scale.cpp         # (NEW)
│   ├── shot_logger.cpp       # (NEW)
│   ├── nvs_config.cpp        # (NEW)
│   │
│   ├── display/              # Display subsystem (NEW)
│   │   ├── display.cpp       # Display driver init
│   │   ├── lvgl_port.cpp     # LVGL tick, flush, input
│   │   ├── encoder.cpp       # Encoder handling
│   │   └── theme.cpp         # Theme setup
│   │
│   └── ui/                   # LVGL UI screens (NEW)
│       ├── ui.cpp            # Screen manager
│       ├── screen_home.cpp   # Home implementation
│       ├── screen_brew.cpp   # Brew screen
│       ├── screen_settings.cpp
│       └── screen_stats.cpp
│
├── data/                      # LittleFS web files
│   ├── index.html
│   ├── style.css
│   ├── app.js
│   ├── settings.html         # (NEW)
│   ├── stats.html            # (NEW)
│   └── icons/                # PWA icons (NEW)
│
├── lv_conf.h                  # LVGL configuration (NEW)
└── platformio.ini
```

---

## Dependencies

### Current
- `ESP Async WebServer` - HTTP server
- `AsyncTCP` - Async TCP for ESP32
- `ArduinoJson` - JSON parsing
- `LittleFS` (built-in) - File system
- `Preferences` (built-in) - NVS storage

### To Add - Display & UI
- `lvgl/lvgl` - LVGL graphics library (v8.3+)
- `lovyan03/LovyanGFX` - Display driver (GC9A01/ST7701)
- Display driver for round 480x480 IPS

### To Add - Connectivity
- `PubSubClient` or `AsyncMqttClient` - MQTT client
- `NimBLE-Arduino` - BLE (lighter than default, for scales)
- `WiFiClientSecure` - HTTPS (built-in)
- `ArduinoWebsockets` - Cloud WebSocket client

### LVGL Configuration

```cpp
// lv_conf.h key settings for round display
#define LV_HOR_RES_MAX          480
#define LV_VER_RES_MAX          480
#define LV_COLOR_DEPTH          16
#define LV_USE_GPU              0
#define LV_USE_PERF_MONITOR     1  // Debug: show FPS
#define LV_FONT_MONTSERRAT_14   1
#define LV_FONT_MONTSERRAT_24   1
#define LV_FONT_MONTSERRAT_48   1  // Large numbers
```

---

## Testing Checklist

### Unit Tests
- [ ] MQTT message formatting
- [ ] JSON serialization
- [ ] Weight stop offset calculation
- [ ] API request parsing

### Integration Tests
- [ ] MQTT broker connection
- [ ] Scale pairing and reading
- [ ] Cloud registration flow
- [ ] Pico communication roundtrip

### System Tests
- [ ] Full brew-by-weight cycle
- [ ] Cloud monitoring while brewing
- [ ] MQTT command execution
- [ ] OTA update process

---

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2025-11-28 | Initial implementation plan |

---

## References

- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [Acaia Scale Protocol](https://github.com/lucapinello/pyacaia)
- [Home Assistant MQTT Discovery](https://www.home-assistant.io/integrations/mqtt/#discovery)
- [ESP-IDF BLE Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/ble/index.html)

