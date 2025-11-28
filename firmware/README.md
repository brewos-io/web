# BrewOS Pico Firmware

## Overview

Real-time control firmware for the RP2040 microcontroller (Raspberry Pi Pico) that manages:
- Dual PID temperature control (brew and steam boilers)
- Safety interlocks (water levels, over-temperature)
- Brew cycle automation (pump, solenoid)
- Communication with ESP32 display module
- Power monitoring via PZEM-004T

**Language:** C (Pico SDK)  
**Target:** Raspberry Pi Pico (RP2040)  
**Toolchain:** CMake + ARM GCC  

---

## Documentation

| Document | Purpose |
|----------|---------|
| [Requirements](Requirements.md) | What the firmware must do (functional & safety) |
| [Architecture](Architecture.md) | How it's structured (modules, dual-core, state machine) |
| [Communication Protocol](Communication_Protocol.md) | Binary protocol with ESP32 (includes OTA firmware streaming) |
| [Error Handling](Error_Handling.md) | Error tracking, validation, and reporting for all components |
| [Machine Configurations](Machine_Configurations.md) | Multi-machine support |
| [Environmental Configuration](Environmental_Configuration.md) | Voltage and current limit configuration |
| [Debugging](Debugging.md) | Debug strategies, Picoprobe setup, logging |
| [Feature Status](Feature_Status_Table.md) | Implementation status of all features |
| [Implementation Plan](Implementation_Plan.md) | High-level implementation phases and roadmap |
| [Quick Start](Quick_Start.md) | Quick reference for common tasks |
| [Setup Guide](../SETUP.md) | Development environment setup and OTA updates |
| [Versioning](Versioning.md) | Firmware versioning and release management |
| **Feature Documentation** | |
| [Cleaning Mode](Cleaning_Mode_Implementation.md) | Cleaning cycle implementation with brew counter |
| [Statistics Feature](Statistics_Feature.md) | Statistics and analytics implementation |
| [Water Management](Water_Management_Implementation.md) | Steam boiler auto-fill and water system |
| [Shot Timer Display](Shot_Timer_Display.md) | Shot timer and display integration |

---

## Quick Start

### Prerequisites

1. **Pico SDK** (v1.5.0 or later)
   ```bash
   git clone https://github.com/raspberrypi/pico-sdk.git
   cd pico-sdk
   git submodule update --init
   export PICO_SDK_PATH=$(pwd)
   ```

2. **ARM GCC Toolchain**
   ```bash
   # macOS
   brew install --cask gcc-arm-embedded
   
   # Ubuntu/Debian
   sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi
   ```

3. **CMake** (3.13 or later)
   ```bash
   # macOS
   brew install cmake
   
   # Ubuntu/Debian
   sudo apt install cmake
   ```

### Building

```bash
cd src/pico
mkdir build && cd build

# Configure for your machine type
cmake -DMACHINE_TYPE=DUAL_BOILER ..      # For dual boiler machines
cmake -DMACHINE_TYPE=SINGLE_BOILER ..    # For single boiler machines
cmake -DMACHINE_TYPE=HEAT_EXCHANGER ..   # For HX machines

# Build
make -j4

# Output: brewos_dual_boiler.uf2 (or other variant)
```

### Flashing

**Method 1: USB (Manual)**
1. Hold BOOTSEL button on Pico
2. Connect USB while holding button
3. Release button - Pico mounts as USB drive
4. Copy `brewos_*.uf2` to the drive
5. Pico reboots automatically

**Method 2: OTA via ESP32 (when wired)**
See [SETUP.md](../SETUP.md#method-2-ota-via-esp32-when-wired) for detailed instructions on over-the-air firmware updates via the ESP32 web interface.

---

## Project Structure

```
src/pico
├── CMakeLists.txt              # Build configuration
├── main.c                      # Entry point
├── config.h                    # Pin definitions, constants
│
├── /include                    # Header files
│   ├── machine_config.h        # Machine type definitions
│   ├── control.h               # Control system interface
│   └── ...
│
├── /src                        # Implementation
│   ├── control_dual_boiler.c   # Dual boiler control
│   ├── control_single_boiler.c # Single boiler control
│   ├── control_heat_exchanger.c# HX control
│   ├── safety.c                # Safety systems
│   ├── sensors.c               # Sensor drivers
│   ├── protocol.c              # ESP32 communication
│   └── ...
│
└── /examples                   # Example configurations
    └── hardware_example.c
```

---

## Dual-Core Architecture

The RP2040 has two cores, used as follows:

| Core | Responsibility | Timing |
|------|----------------|--------|
| **Core 0** | Safety, sensors, PID control, outputs | 100ms loop, deterministic |
| **Core 1** | UART communication, protocol handling | Async, interrupt-driven |

Thread-safe shared state is used for inter-core communication.

---

## Configuration

### Compile-Time (Machine Type)

Set via CMake:
```bash
cmake -DMACHINE_TYPE=DUAL_BOILER ..
```

This selects:
- Control algorithm (dual boiler, single boiler, or HX)
- Feature flags (number of boilers, sensors present)
- Safety thresholds

### Runtime (Tunable Parameters)

Stored in flash, modifiable via ESP32 commands:
- PID gains (Kp, Ki, Kd)
- Temperature setpoints
- Pre-infusion timing

---

## Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DMACHINE_TYPE=DUAL_BOILER ..
make
```

Debug build enables:
- UART printf logging
- Assertions
- Timing statistics

Connect to Pico's USB serial (115200 baud) to see debug output.

**Note:** The UART communication with ESP32 uses 921600 baud (see [Communication Protocol](Communication_Protocol.md)), while USB serial debug uses 115200 baud.

---

## Safety

**Critical:** Read [Requirements.md Section 2](Requirements.md#2-safety-requirements-critical) before modifying any code.

Key safety features:
- Watchdog timer (2s timeout)
- Water level interlocks
- Over-temperature shutdown
- Safe state on any fault

---

## Testing

See [Hardware Test Procedures](../hardware/Test_Procedures.md) for:
- Breadboard circuit validation
- GPIO and peripheral tests
- Full machine integration

