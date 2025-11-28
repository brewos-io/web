# BrewOS - Open Source Espresso Machine Firmware

## Project Overview

An open-source control system to replace factory controllers in espresso machines. Designed as a plug-and-play replacement with enhanced features for dual-boiler, single-boiler, and heat-exchanger machines.

**Architecture:** Raspberry Pi Pico (RP2040) + ESP32 Display Module  
**Status:** Development  

---

## Documentation Index

### Hardware

| Document | Description |
|----------|-------------|
| [Hardware Specification](hardware/Specification.md) | PCB design, component selection, electrical specs |
| [Test Procedures](hardware/Test_Procedures.md) | Breadboard, prototype, and integration testing |

### Firmware

| Document | Description |
|----------|-------------|
| [Firmware Overview](firmware/README.md) | Getting started, build instructions, toolchain |
| [Requirements](firmware/Requirements.md) | Functional and safety requirements |
| [Architecture](firmware/Architecture.md) | Module structure, dual-core design, state machine |
| [Communication Protocol](firmware/Communication_Protocol.md) | Binary protocol between Pico ↔ ESP32 |
| [Machine Configurations](firmware/Machine_Configurations.md) | Multi-machine support, pin mappings |
| [Debugging](firmware/Debugging.md) | Debug strategies, Picoprobe setup, remote logging |
| [Feature Status](firmware/Feature_Status_Table.md) | Implementation status of all features |

### Setup & Development

| Document | Description |
|----------|-------------|
| [Setup Guide](SETUP.md) | Development environment setup, build instructions, OTA updates |

### Schematics

| Document | Description |
|----------|-------------|
| [ECM Synchronika Schematic](schematics/ECM_Schematic_Reference.md) | Circuit diagrams for ECM Synchronika |
| [ECM Synchronika Netlist](schematics/ECM_Netlist.csv) | Component netlist for ECM Synchronika PCB |

---

## Quick Links

- **Setup Guide:** [SETUP.md](SETUP.md) - Development environment and OTA updates
- **Safety Requirements:** [firmware/Requirements.md#2-safety-requirements-critical](firmware/Requirements.md#2-safety-requirements-critical)
- **GPIO Pin Mapping:** [firmware/Requirements.md#31-gpio-pin-mapping](firmware/Requirements.md#31-gpio-pin-mapping)
- **Protocol Messages:** [firmware/Communication_Protocol.md](firmware/Communication_Protocol.md)
- **OTA Updates:** [SETUP.md#method-2-ota-via-esp32-when-wired](SETUP.md#method-2-ota-via-esp32-when-wired) - Over-the-air firmware updates

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              SYSTEM ARCHITECTURE                             │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│    ┌──────────────┐         ┌──────────────┐         ┌──────────────┐       │
│    │   SENSORS    │         │  PICO RP2040 │         │   ACTUATORS  │       │
│    │              │         │              │         │              │       │
│    │ • NTC ×2     │────────►│ • Safety     │────────►│ • SSR ×2     │       │
│    │ • Thermocouple│        │ • PID Control│         │ • Relay ×4   │       │
│    │ • Pressure   │         │ • State Mgmt │         │ • Buzzer     │       │
│    │ • Levels ×3  │         │              │         │ • LED        │       │
│    │ • Switches   │         └──────┬───────┘         └──────────────┘       │
│    └──────────────┘                │                                         │
│                                    │ Binary UART                             │
│                                    │ (921600 baud)                           │
│                                    ▼                                         │
│                           ┌──────────────┐                                   │
│    ┌──────────────┐       │    ESP32     │       ┌──────────────┐           │
│    │    PZEM      │──────►│   Display    │◄─────►│  External    │           │
│    │  Power Meter │ UART  │   Module     │ WiFi  │  (MQTT, App) │           │
│    └──────────────┘       └──────────────┘       └──────────────┘           │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Supported Machines

BrewOS supports multiple machine architectures through compile-time configuration:

| Machine Type | Status | Examples |
|--------------|--------|----------|
| Dual Boiler | ✅ Supported | ECM Synchronika, Profitec Pro 700 |
| Single Boiler | ✅ Supported | Rancilio Silvia, Gaggia Classic |
| Heat Exchanger | ✅ Supported | E61 HX machines |
| Thermoblock | 🔮 Future | - |

### Machine-Specific Resources

| Machine | Schematics | Notes |
|---------|------------|-------|
| ECM Synchronika | [Schematic](schematics/ECM_Schematic_Reference.md), [Netlist](schematics/ECM_Netlist.csv) | Dual boiler reference implementation |

Want to add support for your machine? See [Machine Configurations](firmware/Machine_Configurations.md).

---

## Building Firmware

```bash
# Clone the repository
git clone https://github.com/yourusername/brewos.git
cd brewos/src/pico

# Create build directory
mkdir build && cd build

# Build for your machine type
cmake -DMACHINE_TYPE=DUAL_BOILER ..   # For dual boiler machines
cmake -DMACHINE_TYPE=SINGLE_BOILER .. # For single boiler machines
cmake -DMACHINE_TYPE=HEAT_EXCHANGER .. # For HX machines

# Compile
make -j4

# Output: brewos_dual_boiler.uf2 (or other variant)
```

---

## Safety Notice

```
⚠️  WARNING: MAINS VOLTAGE

This project involves 100-240V AC mains electricity.
Improper handling can result in death or serious injury.

• Only qualified individuals should work on mains circuits
• Always use isolation transformers during development
• Never work alone on energized equipment
• Follow all safety procedures in the test documentation
```

---

## Contributing

Contributions are welcome! Please read the documentation before submitting PRs:

1. Check [Feature Status](firmware/Feature_Status_Table.md) for what's needed
2. Follow the existing code style
3. Add tests where applicable
4. Update documentation

---

## License

[TBD]

