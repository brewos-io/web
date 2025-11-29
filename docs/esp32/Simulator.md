# UI Simulator

The BrewOS UI Simulator runs the **actual LVGL UI screens** on your desktop for development without flashing hardware.

## Prerequisites

- **macOS:** Install SDL2 via Homebrew
  ```bash
  brew install sdl2
  ```

- **Linux:** Install SDL2 dev package
  ```bash
  sudo apt install libsdl2-dev
  ```

- **PlatformIO CLI** installed and available in PATH

## Quick Start

```bash
./src/scripts/run_simulator.sh
```

This will:
1. Check for SDL2 (install if missing on macOS)
2. Build the simulator with actual UI screens
3. Launch a 480×480 round display window

## Controls (Encoder Simulation)

| Input | Action |
|-------|--------|
| **Scroll wheel** | Rotate knob |
| **↑ / ← arrows** | Rotate CCW |
| **↓ / → arrows** | Rotate CW |
| **Click / Enter / Space** | Button press |
| **Hold 1 second** | Long press |
| **ESC** | Exit |

### Quick Screen Switching (Development)

| Key | Screen |
|-----|--------|
| **0** | Setup (WiFi) |
| **1** | Idle |
| **2** | Home |
| **3** | Brewing |
| **4** | Complete |
| **5** | Settings |
| **6** | Alarm |

The terminal shows encoder events and state updates in real-time.

## Manual Build & Run

```bash
cd src/esp32
pio run -e simulator
.pio/build/simulator/program
```

## What's Simulated

The simulator runs the **actual UI code** from `src/esp32/src/ui/`:

| Screen | Status |
|--------|--------|
| **Idle** | ✅ Implemented |
| **Home** | ✅ Implemented |
| **Brewing** | ✅ Implemented |
| **Complete** | ✅ Implemented |
| **Settings** | ✅ Implemented |
| **Setup** | ✅ Implemented |
| **Alarm** | ✅ Implemented |
| **BBW Settings** | ❌ Excluded (has BLE deps) |

### Mock Machine State

The simulator provides mock data that updates every 500ms:

| Value | Range | Notes |
|-------|-------|-------|
| Brew Temp | 90-96°C | Random fluctuations |
| Steam Temp | 140-150°C | Random fluctuations |
| Pressure | 9.0 bar | Static |
| Setpoints | 94°C / 145°C | Fixed |

### Round Display Mask

The window shows corner masks to simulate the actual 2.1" round display - content in the corners won't be visible on real hardware.

## Architecture

The simulator compiles the **same UI code** as ESP32 by using a platform abstraction layer:

```
src/esp32/
├── include/
│   └── platform/
│       ├── platform.h      ← Common interface
│       ├── arduino_impl.h  ← ESP32 implementation
│       └── native_impl.h   ← Simulator implementation
├── src/
│   ├── ui/                 ← Shared UI screens
│   │   ├── screen_home.cpp
│   │   ├── screen_idle.cpp
│   │   └── ...
│   └── simulator/
│       └── main.cpp        ← Simulator entry point
```

### Making UI Code Portable

UI files use `#include "platform/platform.h"` instead of `<Arduino.h>`:

```cpp
// Before (Arduino-dependent)
#include <Arduino.h>
LOG_I("message");  // Uses Serial

// After (platform-agnostic)
#include "platform/platform.h"
LOG_I("message");  // Uses platform_log()
```

## Editing Theme Colors

Theme colors are defined in `src/esp32/include/display/theme.h`:

```c
#define COLOR_BG_DARK           lv_color_hex(0x1A0F0A)  // Background
#define COLOR_ACCENT_PRIMARY    lv_color_hex(0xD5A071)  // Caramel accent
#define COLOR_TEXT_PRIMARY      lv_color_hex(0xFBFCF8)  // Text
```

After editing any UI file, rebuild:

```bash
pio run -e simulator && .pio/build/simulator/program
```

## Simulator Files

| File | Purpose |
|------|---------|
| `src/simulator/main.cpp` | SDL setup, encoder input, mock state |
| `include/platform/platform.h` | Platform abstraction |
| `include/platform/native_impl.h` | Native implementations |

## Limitations

- **BBW Screen excluded** - Has BLE scale dependencies
- **No actual hardware** - Uses mock machine state
- **No WiFi/MQTT** - Connectivity features not simulated

## Troubleshooting

### Build Fails: "SDL.h not found"

SDL2 is not installed or not in include path.

**macOS:**
```bash
brew install sdl2
```

**Linux:**
```bash
sudo apt install libsdl2-dev
```

### Build Fails: "Arduino.h not found"

You're building the wrong environment. Use:
```bash
pio run -e simulator
```
Not `pio run -e esp32s3` which targets real hardware.

### Window Doesn't Appear

Check if another process is using the display or if you're in a headless SSH session. The simulator requires a graphical environment.

