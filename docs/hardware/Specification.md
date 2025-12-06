# ECM Synchronika Custom Control Board

## Production Design Specification v2.0

**Document Purpose:** Complete technical specification for PCB design and manufacturing  
**Target:** Plug & play replacement for GICAR control board and PID controller  
**Revision:** 2.21  
**Date:** December 2025

---

## ⚠️ DESIGN UPDATE IN v2.21 (Dec 2025)

### ✅ External Power Metering Interface (Universal Module Support)

- **REMOVED:** PZEM-004T-100A-D-P embedded daughterboard (J17, J24 connectors eliminated)
- **NEW:** Universal external power meter interface via **J17 (JST-XH 6-pin)** for UART/RS485 communication
- **Supports multiple modules:** PZEM-004T, JSY-MK-163T, JSY-MK-194T, Eastron SDM, and more
- **Off-board metering:** External modules handle their own HV connections (no 220V through control PCB for metering)
- **RS485 support:** On-board MAX3485 transceiver with jumper-selectable termination resistor
- **Auto-baud scanning:** Firmware supports 2400, 4800, 9600, 19200 baud auto-detection
- **Benefits:** Superior safety (HV isolated from control PCB), mechanical flexibility, future-proof design

### ✅ Unified Low-Voltage Screw Terminal Block (J26 - 22 Position)

- **ALL low-voltage connections** consolidated into **single 22-position screw terminal block**
- **J26 includes:** Switch inputs (S1-S4), Analog sensors (NTC×2, Thermocouple, Pressure), SSR control outputs
- **6.3mm spades retained ONLY for 220V AC:** Mains input (J1: L, N, PE), K1 LED (J2), K2 Pump (J3), K3 Solenoid (J4)
- **Benefits:** Single terminal block for ALL low-voltage wiring, professional appearance, easier assembly

### ✅ Spare Relay K4 Removed

- **Simplified to 3 relays:** K1 (Water LED), K2 (Pump), K3 (Solenoid) - no spare relay
- **Removed components:** K4, Q4, D4, LED4, R13, R23, R33, R83, C53, J9
- **GPIO20 available:** Test point TP1 added for future expansion access
- **Benefits:** Reduced BOM cost, smaller PCB footprint, simplified design

```
┌────────────────────────────────────────────────────────────────────────────────────────────────┐
│                    J26 - UNIFIED LOW-VOLTAGE SCREW TERMINAL (22-pos)                           │
│                           Phoenix MKDS 1/22-5.08 (5.08mm pitch)                                │
│                                  ⚠️ LOW VOLTAGE ONLY ⚠️                                        │
├────────────────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                                │
│  SWITCHES (S1-S4)          ANALOG SENSORS                    SSR OUTPUTS            SPARE     │
│  ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐    │
│  │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │10 │11 │12 │13 │14 │15 │16 │17 │18 │19 │20 │21 │22 │    │
│  │S1 │S1G│S2 │S2G│S3 │S4 │S4G│T1 │T1G│T2 │T2G│TC+│TC-│P5V│PGD│PSG│SR+│SR-│SR+│SR-│GND│GND│    │
│  └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘    │
│   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │           │
│   └─S1─┘   └─S2─┘  S3  └─S4─┘   └─Brew─┘   └Steam┘ └─TC──┘ └─Pressure─┘ └SSR1─┘ └SSR2─┘ Spare │
│   Water    Tank   Lvl  Brew     NTC      NTC     Thermo    Transducer   Brew    Steam         │
│   Res.     Level  Prb  Handle                    couple    (YD4060)     Heater  Heater        │
│                                                                                                │
│  ⚠️ NOTE: CT Clamp connections REMOVED - now handled by external power meter module           │
│                                                                                                │
└────────────────────────────────────────────────────────────────────────────────────────────────┘

220V AC RELAY OUTPUTS (6.3mm Spade - Unchanged):
┌────────────────────────────────────────────────────────────────┐
│  J2-NO: K1 Water LED (220V, ≤100mA)                           │
│  J3-NO: K2 Pump Motor (220V, 5A) - HIGH POWER                 │
│  J4-NO: K3 Solenoid Valve (220V, ~0.5A)                       │
└────────────────────────────────────────────────────────────────┘
```

---

## ⚠️ DESIGN UPDATE IN v2.17 (Nov 2025)

### ✅ Brew-by-Weight Support (J15 Expanded to 8-pin)

- **J15 ESP32 connector expanded** from 6-pin to 8-pin JST-XH
- **GPIO21:** WEIGHT_STOP signal - ESP32 → Pico for brew-by-weight
- **GPIO22:** SPARE - Reserved for future expansion
- **Enables Bluetooth scale integration** via ESP32 (e.g., Acaia, Decent)
- **No interrupt needed** - ESP32 signals Pico when target weight reached

```
┌─────────────────┐                    ┌─────────────────┐
│   ESP32 + BLE   │                    │   Pico (Main)   │
│   + Display     │     J15 Pin 7      │                 │
│                 │   (WEIGHT_STOP)    │                 │
│  Scale Weight ──┼────────────────────┼──► GPIO21       │
│  Reached?       │                    │     │           │
│                 │                    │     ▼           │
│                 │                    │  Stop Pump!     │
└─────────────────┘                    └─────────────────┘
```

---

## ⚠️ MAJOR DESIGN UPDATE IN v2.15 (Nov 2025)

**This specification has been significantly simplified with two major component changes:**

### ✅ AC Level Sensing Circuit (OPA342 + TLV3201)

- **Custom oscillator + comparator circuit** for liquid level detection
- **AC excitation** (~1kHz) - prevents electrolysis and probe corrosion
- **Uses commonly available, modern components** (OPA342, TLV3201)
- **Highly reliable** with clean digital output to GPIO4
- **Logic entirely on PCB** - no external modules

### ✅ Universal Power Metering Interface (External Modules)

- **Design:** External power metering modules with universal interface connector
- **Connector:** J17 (JST-XH 6-pin) for low-voltage UART/RS485 communication
- **No HV on control PCB:** External modules handle their own mains voltage connections
- **Dual-mode support:** TTL UART + RS485 via on-board MAX3485 transceiver
- **Compatible modules:** PZEM-004T, JSY-MK-163T/194T, Eastron SDM, ATM90E32, and others
- **Auto-detection:** Firmware scans baud rates (9600, 4800, 2400, 19200) on startup
- **Register map architecture:** Data-driven driver supports multiple meter types via configuration

**Why External is Better:**

- **Superior Safety:** High voltage physically separated from control PCB
- **Mechanical Flexibility:** Modules mount anywhere in enclosure (DIN rail, wall, etc.)
- **Future-Proof:** Support new modules without PCB redesign
- **Better Thermal:** Heat-generating modules kept away from sensitive electronics

---

# TABLE OF CONTENTS

1. [System Overview](#1-system-overview)
2. [Electrical Specifications](#2-electrical-specifications)
3. [Component Summary](#3-component-summary)
4. [Microcontroller & GPIO Allocation](#4-microcontroller--gpio-allocation)
5. [Power Supply Design](#5-power-supply-design)
6. [Relay & SSR Output Circuits](#6-relay--ssr-output-circuits)
7. [Sensor Input Circuits](#7-sensor-input-circuits)
8. [Communication Interfaces](#8-communication-interfaces)
9. [User Interface Components](#9-user-interface-components)
10. [Power Metering Circuit](#10-power-metering-circuit)
11. [Safety & Protection](#11-safety--protection)
12. [PCB Design Requirements](#12-pcb-design-requirements)
13. [Connector Specifications](#13-connector-specifications)
14. [Bill of Materials](#14-bill-of-materials)
15. [Testing & Validation](#15-testing--validation)
16. [Deliverables](#16-deliverables)

---

# 1. System Overview

## 1.1 Project Description

This specification defines a custom control PCB to replace the factory GICAR control board and PID controller in an ECM Synchronika dual-boiler espresso machine. The design must be:

- **Plug & Play**: Direct replacement using existing machine wiring
- **Reversible**: Original equipment can be reinstalled without modification
- **Universal Voltage**: Support 100-240V AC, 50/60Hz worldwide operation
- **Production Ready**: Suitable for small-batch or volume manufacturing

## 1.2 Design Goals

| Requirement         | Description                                        |
| ------------------- | -------------------------------------------------- |
| Temperature Control | Dual PID control for brew and steam boilers        |
| Pressure Monitoring | Real-time pressure display and profiling           |
| Safety Interlocks   | Water level, over-temperature, watchdog protection |
| Connectivity        | ESP32 display module for WiFi, MQTT, Web API       |
| Power Monitoring    | Total machine power consumption metering           |
| User Feedback       | Status LEDs, buzzer for alerts                     |
| Serviceability      | Debug port, accessible test points                 |

## 1.3 System Architecture

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                        ECM SYNCHRONIKA CONTROL SYSTEM                           │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│   ┌─────────────────┐     ┌─────────────────────────────────────────────────┐   │
│   │   MAINS INPUT   │     │              CONTROL PCB                        │   │
│   │   100-240V AC   │     │  ┌─────────────────────────────────────────┐   │    │
│   │   50/60 Hz      │────►│  │  ISOLATED AC/DC         LOW VOLTAGE     │   │    │
│   │                 │     │  │  POWER SUPPLY    ║      SECTION         │   │    │
│   └─────────────────┘     │  │  (HLK-PM05)      ║                      │   │    │
│                           │  │       │          ║   ┌──────────────┐   │   │    │
│   ┌─────────────────┐     │  │       ▼          ║   │ Raspberry Pi │   │   │    │
│   │  POWER METER    │     │  │    5V Rail ──────╫──►│    Pico      │   │   │    │
│   │  (PZEM-004T)    │◄────│  │       │          ║   │   RP2040     │   │   │    │
│   │  External + CT  │     │  │       ▼          ║   └──────┬───────┘   │   │    │
│   └─────────────────┘     │  │   3.3V Rail ─────╫──────────┘           │   │    │
│                           │  │                  ║                      │   │    │
│   ┌─────────────────┐     │  │   RELAY DRIVERS  ║   SENSOR INPUTS      │   │    │
│   │   RELAYS (4x)   │◄────│  │   + INDICATOR    ║   + PROTECTION       │   │  │
│   │   Pump          │     │  │   LEDs           ║                      │   │  │
│   │   Solenoid      │     │  │                  ║                      │   │  │
│   │   Water LED     │     │  ╠══════════════════╬══════════════════════╣   │  │
│   │   Spare         │     │  │  HIGH VOLTAGE    ║    LOW VOLTAGE       │   │  │
│   └─────────────────┘     │  │  SECTION         ║    SECTION           │   │  │
│                           │  │  (ISOLATED)      ║    (SAFE)            │   │  │
│   ┌─────────────────┐     │  └─────────────────────────────────────────┘   │  │
│   │  EXTERNAL SSRs  │◄────│                                                 │  │
│   │  Brew Heater    │     │  ┌─────────────────────────────────────────┐   │  │
│   │  Steam Heater   │     │  │  CONNECTORS                             │   │  │
│   └─────────────────┘     │  │  • 6.3mm Spade terminals (machine)     │   │  │
│                           │  │  • JST-XH 8-pin (ESP32 display+weight) │   │  │
│   ┌─────────────────┐     │  │  • 2.54mm headers (service/debug)      │   │  │
│   │  ESP32 DISPLAY  │◄────│  │  • Screw terminals (sensors)           │   │  │
│   │  MODULE         │     │  └─────────────────────────────────────────┘   │  │
│   │  (External)     │     │                                                 │  │
│   └─────────────────┘     └─────────────────────────────────────────────────┘  │
│                                                                                  │
│   ┌─────────────────┐     ┌─────────────────────────────────────────────────┐  │
│   │  MACHINE LOADS  │     │  SENSORS                                        │  │
│   │  • Pump Motor   │     │  • NTC Thermistors (Brew/Steam boilers)        │  │
│   │  • 3-Way Valve  │     │  • K-Type Thermocouple (Brew head)             │  │
│   │  • Brew Heater  │     │  • Pressure Transducer (0.5-4.5V)              │  │
│   │  • Steam Heater │     │  • Water Level Switches                         │  │
│   │  • Water LED    │     │  • Steam Boiler Level Probe                     │  │
│   └─────────────────┘     └─────────────────────────────────────────────────┘  │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

---

# 2. Electrical Specifications

## 2.1 Input Power

| Parameter       | Specification                            |
| --------------- | ---------------------------------------- |
| Input Voltage   | 100-240V AC ±10%                         |
| Frequency       | 50/60 Hz                                 |
| Maximum Current | 16A (total machine load through relays)  |
| Power Factor    | >0.9 (machine dependent)                 |
| Inrush Current  | Limited by machine's existing protection |

## 2.2 Output Power Rails

| Rail    | Voltage  | Current Capacity | Source                | Purpose                          |
| ------- | -------- | ---------------- | --------------------- | -------------------------------- |
| 5V DC   | 5.0V ±5% | **3A minimum**   | Isolated AC/DC module | Pico, relays, ESP32, SSR drivers |
| 3.3V DC | 3.3V ±3% | 500mA minimum    | LDO from 5V           | Sensors, logic                   |

## 2.3 Isolation Requirements

| Boundary              | Isolation Type | Requirement                         |
| --------------------- | -------------- | ----------------------------------- |
| Mains → 5V DC         | Reinforced     | 3000V AC for 1 minute               |
| Relay Contacts → Coil | Basic          | 2500V AC                            |
| Power Meter → Logic   | Functional     | Via opto-isolated UART in PZEM-004T |

## 2.4 Environmental

| Parameter             | Specification                 |
| --------------------- | ----------------------------- |
| Operating Temperature | 0°C to +50°C                  |
| Storage Temperature   | -20°C to +70°C                |
| Humidity              | 20% to 90% RH, non-condensing |
| Altitude              | Up to 2000m                   |

---

# 3. Component Summary

## 3.1 Inputs (Sensors & Switches)

| ID  | Component                    | Type                  | Signal              | Connection    |
| --- | ---------------------------- | --------------------- | ------------------- | ------------- |
| S1  | Water Reservoir Switch       | SPST N.O.             | Digital, Active Low | J26 Pin 1-2   |
| S2  | Tank Level Sensor            | 2-wire Magnetic Float | Digital, Active Low | J26 Pin 3-4   |
| S3  | Steam Boiler Level Probe     | Conductivity Probe    | Digital/Analog      | J26 Pin 5     |
| S4  | Brew Handle Switch           | SPST N.O./N.C.        | Digital, Active Low | J26 Pin 6-7   |
| T1  | Brew Boiler Temp             | NTC 3.3kΩ @ 25°C      | Analog (ADC)        | J26 Pin 8-9   |
| T2  | Steam Boiler Temp            | NTC 3.3kΩ @ 25°C      | Analog (ADC)        | J26 Pin 10-11 |
| T3  | Brew Head Temp               | K-Type Thermocouple   | SPI (MAX31855)      | J26 Pin 12-13 |
| P1  | Pressure Transducer (YD4060) | 0.5-4.5V, 0-16 bar    | Analog (ADC)        | J26 Pin 14-16 |

## 3.2 Outputs (Actuators)

| ID   | Component              | Load Rating         | Control             | Connection       |
| ---- | ---------------------- | ------------------- | ------------------- | ---------------- |
| K1   | Water Status LED       | 100-240V AC, ≤100mA | Onboard Relay       | J2 (6.3mm spade) |
| K2   | Pump Motor             | 100-240V AC, 65W    | Onboard Relay (16A) | J3 (6.3mm spade) |
| K3   | Solenoid Valve (3-way) | 100-240V AC, 15W    | Onboard Relay (10A) | J4 (6.3mm spade) |
| SSR1 | Brew Boiler Heater     | 100-240V AC, 1400W  | External SSR 40A    | J26 Pin 19-20    |
| SSR2 | Steam Boiler Heater    | 100-240V AC, 1400W  | External SSR 40A    | J26 Pin 21-22    |

## 3.3 Communication Interfaces

| Interface     | Purpose                                  | Connector           |
| ------------- | ---------------------------------------- | ------------------- |
| ESP32 Display | Main UI, WiFi, MQTT, OTA, Brew-by-Weight | JST-XH 8-pin        |
| Service/Debug | Firmware debug, emergency access         | 2.54mm 4-pin header |
| Power Meter   | Universal external modules (TTL/RS485)   | JST-XH 6-pin (J17)  |

## 3.4 User Interface (Onboard)

| Component          | Purpose                               |
| ------------------ | ------------------------------------- |
| Status LED (Green) | System state indication               |
| 6× Indicator LEDs  | Relay/SSR status (4 Green + 2 Orange) |
| Buzzer (Passive)   | Audio alerts (PWM tones)              |
| Reset Button (SMD) | Hardware reset                        |
| Boot Button (SMD)  | Bootloader mode entry                 |

---

# 4. Microcontroller & GPIO Allocation

## 4.1 Raspberry Pi Pico Specifications

| Parameter         | Value                                    |
| ----------------- | ---------------------------------------- |
| MCU               | RP2040 Dual-core ARM Cortex-M0+ @ 133MHz |
| Flash             | 2MB onboard (W25Q16JV)                   |
| SRAM              | 264KB                                    |
| GPIO              | 26 multi-function pins                   |
| ADC               | 3 channels, 12-bit, 500 ksps             |
| UART              | 2× peripherals                           |
| SPI               | 2× peripherals                           |
| I2C               | 2× peripherals                           |
| PWM               | 8× slices (16 channels)                  |
| PIO               | 2× programmable I/O blocks               |
| Operating Voltage | 1.8V - 5.5V (via VSYS), 3.3V logic       |
| Temperature Range | -20°C to +85°C                           |

## 4.2 Complete GPIO Allocation

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                         RASPBERRY PI PICO GPIO MAP                              │
│                          (All 26 GPIOs Allocated)                               │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  ACTIVE ANALOG INPUTS (ADC)                                              │  │
│  │  ├── GPIO26 (ADC0) ─── Brew Boiler NTC (via voltage divider)            │  │
│  │  ├── GPIO27 (ADC1) ─── Steam Boiler NTC (via voltage divider)           │  │
│  │  └── GPIO28 (ADC2) ─── Pressure Transducer (scaled 0.5-4.5V → 0.34-3.06V) │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  SPI BUS (Thermocouple Amplifier)                                        │  │
│  │  ├── GPIO16 (SPI0 RX/MISO) ─── MAX31855 DO (Data Out)                   │  │
│  │  ├── GPIO17 (SPI0 CSn) ─────── MAX31855 CS (Chip Select)                │  │
│  │  └── GPIO18 (SPI0 SCK) ─────── MAX31855 CLK (Clock)                     │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  DIGITAL INPUTS (Switches & Sensors)                                     │  │
│  │  ├── GPIO2 ─── Water Reservoir Switch (internal pull-up, active low)   │  │
│  │  ├── GPIO3 ─── Tank Level Sensor (internal pull-up, active low)        │  │
│  │  ├── GPIO4 ─── Steam Boiler Level (TLV3201 comparator out, active low) │  │
│  │  └── GPIO5 ─── Brew Handle Switch (internal pull-up, active low)       │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  DIGITAL OUTPUTS (Relay & SSR Drivers)                                   │  │
│  │  ├── GPIO10 ─── Water LED Relay (K1) + Green Indicator LED              │  │
│  │  ├── GPIO11 ─── Pump Relay (K2) + Green Indicator LED                   │  │
│  │  ├── GPIO12 ─── Solenoid Relay (K3) + Green Indicator LED               │  │
│  │  ├── GPIO13 ─── Brew SSR Trigger (SSR1) + Orange Indicator LED          │  │
│  │  └── GPIO14 ─── Steam SSR Trigger (SSR2) + Orange Indicator LED         │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  UART0 - ESP32 DISPLAY MODULE (8-pin JST-XH)                            │  │
│  │  ├── GPIO0 (UART0 TX) ─── ESP32 RX (33Ω series protection)             │  │
│  │  ├── GPIO1 (UART0 RX) ─── ESP32 TX (33Ω series protection)             │  │
│  │  ├── PICO RUN ◄──────── ESP32 GPIO (ESP32 resets Pico)                 │  │
│  │  └── PICO BOOTSEL ◄──── ESP32 GPIO (ESP32 controls bootloader entry)   │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  SERVICE/DEBUG PORT (4-pin header) - Shared with ESP32 on GPIO0/1       │  │
│  │  ├── GPIO0 (UART0 TX) ─── J15 (ESP32) + J16 (Service) - 33Ω protected  │  │
│  │  └── GPIO1 (UART0 RX) ─── J15 (ESP32) + J16 (Service) - 33Ω protected  │  │
│  │  ⚠️ Disconnect ESP32 cable when using service port for flashing         │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  I2C0 - ACCESSORY PORT (4-pin header)                                   │  │
│  │  ├── GPIO8 (I2C0 SDA) ─── Accessory data (4.7kΩ pull-up)               │  │
│  │  └── GPIO9 (I2C0 SCL) ─── Accessory clock (4.7kΩ pull-up)              │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  USER INTERFACE                                                          │  │
│  │  ├── GPIO15 ─── Status LED (Green, active high, 330Ω series)           │  │
│  │  └── GPIO19 ─── Buzzer (Passive piezo, PWM output, 100Ω series)        │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  POWER METERING (Universal External via UART/RS485)                     │  │
│  │  ├── GPIO6 ─── METER TX (UART to meter RX / RS485 DI)                  │  │
│  │  ├── GPIO7 ─── METER RX (UART from meter TX / RS485 RO)                │  │
│  │  └── GPIO20 ── RS485 DE/RE (Direction control for MAX3485)             │  │
│  │  Supports: PZEM-004T, JSY-MK-163T/194T, Eastron SDM, and more          │  │
│  │  NO high voltage through control PCB - external modules handle mains.   │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  ┌─────────────────────────────────────────────────────────────────────────┐  │
│  │  HARDWARE CONTROL (Direct to Pico pins, not GPIO)                       │  │
│  │  ├── RUN Pin ─── Reset Button (SMD tactile, to GND)                    │  │
│  │  └── BOOTSEL ─── Boot Button (SMD tactile, directly to QSPI_SS)        │  │
│  └─────────────────────────────────────────────────────────────────────────┘  │
│                                                                                 │
│  GPIO UTILIZATION: 26/26 (100% allocated)                                      │
│  REMAINING: None                                                                │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 4.3 GPIO Summary Table

| GPIO | Function                        | Direction | Type    | Pull          | Protection                        |
| ---- | ------------------------------- | --------- | ------- | ------------- | --------------------------------- |
| 0    | UART0 TX → ESP32                | Output    | Digital | None          | 33Ω series                        |
| 1    | UART0 RX ← ESP32                | Input     | Digital | None          | 33Ω series                        |
| 2    | Water Reservoir Switch          | Input     | Digital | Internal PU   | ESD clamp                         |
| 3    | Tank Level Sensor               | Input     | Digital | Internal PU   | ESD clamp                         |
| 4    | Steam Boiler Level (Comparator) | Input     | Digital | None          | TLV3201 output                    |
| 5    | Brew Handle Switch              | Input     | Digital | Internal PU   | ESD clamp                         |
| 6    | Meter TX (UART1)                | Output    | Digital | None          | 33Ω series (to meter RX/RS485 DI) |
| 7    | Meter RX (UART1)                | Input     | Digital | None          | 33Ω series (from meter TX/RS485)  |
| 8    | I2C0 SDA (Accessory)            | I/O       | Digital | 4.7kΩ ext. PU | Accessory expansion               |
| 9    | I2C0 SCL (Accessory)            | Output    | Digital | 4.7kΩ ext. PU | Accessory expansion               |
| 10   | Relay K1 + LED                  | Output    | Digital | None          | -                                 |
| 11   | Relay K2 + LED                  | Output    | Digital | None          | -                                 |
| 12   | Relay K3 + LED                  | Output    | Digital | None          | -                                 |
| 13   | SSR1 Trigger + LED              | Output    | Digital | None          | -                                 |
| 14   | SSR2 Trigger + LED              | Output    | Digital | None          | -                                 |
| 15   | Status LED                      | Output    | Digital | None          | -                                 |
| 16   | SPI0 MISO                       | Input     | Digital | None          | -                                 |
| 17   | SPI0 CS                         | Output    | Digital | None          | -                                 |
| 18   | SPI0 SCK                        | Output    | Digital | None          | -                                 |
| 19   | Buzzer PWM                      | Output    | PWM     | None          | -                                 |
| 20   | RS485 DE/RE                     | Output    | Digital | Pull-down     | MAX3485 direction (TTL mode: NC)  |
| 21   | WEIGHT_STOP (ESP32→Pico)        | Input     | Digital | Pull-down     | Brew-by-weight signal (J15 Pin 7) |
| 22   | SPARE (ESP32)                   | I/O       | Digital | None          | Reserved for future (J15 Pin 8)   |
| 23   | (Spare)                         | -         | -       | -             | Available for expansion           |
| 26   | ADC0 - Brew NTC                 | Input     | Analog  | None          | RC filter                         |
| 27   | ADC1 - Steam NTC                | Input     | Analog  | None          | RC filter                         |
| 28   | ADC2 - Pressure                 | Input     | Analog  | None          | RC filter, divider                |

---

# 5. Power Supply Design

## 5.1 AC/DC Isolated Power Module

Use an integrated isolated AC/DC converter module for safety and simplicity.

### Power Budget Analysis

| Consumer            | Typical    | Peak        | Notes                      |
| ------------------- | ---------- | ----------- | -------------------------- |
| Raspberry Pi Pico   | 50mA       | 100mA       | Via VSYS                   |
| Relay coils (×3)    | 120mA      | 240mA       | ~80mA each when active     |
| SSR drivers (×2)    | 10mA       | 20mA        | Transistor current         |
| ESP32 module        | 150mA      | 500mA       | **WiFi TX spikes!**        |
| Indicator LEDs (×6) | 30mA       | 60mA        | 3 relay + 2 SSR + 1 status |
| Buzzer              | 5mA        | 30mA        | When active                |
| 3.3V LDO load       | 30mA       | 50mA        | Sensors, MAX31855          |
| **TOTAL**           | **~395mA** | **~1000mA** |                            |

**Minimum: 1.5A, Selected: Hi-Link HLK-5M05 (3A)** - 3× headroom over 1A peak, compact size

### AC/DC Module Selection

| Parameter          | **HLK-5M05**    | Mean Well IRM-20-5 | Mean Well IRM-10-5 |
| ------------------ | --------------- | ------------------ | ------------------ |
| Output Voltage     | 5V DC ±2%       | 5V DC ±5%          | 5V DC ±5%          |
| **Output Current** | **3A**          | 4A                 | 2A                 |
| Input Voltage      | 100-240V AC     | 85-264V AC         | 85-264V AC         |
| Isolation          | 3000V AC        | 3000V AC           | 3000V AC           |
| Efficiency         | 80%             | 87%                | 84%                |
| **Package**        | **50×27×16mm**  | 52×27×24mm         | 45×25×21mm         |
| Safety             | CE              | UL, CE, CB         | UL, CE, CB         |
| **Recommendation** | **Best choice** | Overkill           | Insufficient       |

**Selected: Hi-Link HLK-5M05** (3A, compact 16mm height, adequate for ~1.1A peak load)

### Power Module Schematic

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                           AC/DC POWER SUPPLY SECTION                            │
│                        ⚠️  HIGH VOLTAGE - MAINS CONNECTED ⚠️                     │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│                                                                                 │
│    MAINS INPUT                    ISOLATED MODULE                    OUTPUT    │
│    ───────────                    ───────────────                    ──────    │
│                                                                                 │
│    L (Live) ────┬────[F1]────┬────────────────────────────────┐               │
│                 │   (10A)    │                                │               │
│            ┌────┴────┐  ┌────┴────┐     ┌─────────────────┐  │               │
│            │  MOV    │  │   X2    │     │                 │  │               │
│            │ 275V    │  │  100nF  │     │   HLK-PM05      │  │               │
│            │ (RV1)   │  │  (C1)   │     │   or similar    │  │               │
│            └────┬────┘  └────┬────┘     │                 │  │               │
│                 │            │          │  L ─────────────┼──┘               │
│                 │            │          │                 │                   │
│    N (Neutral) ─┴────────────┴──────────┤  N ─────────────┼──┐               │
│                                         │                 │  │               │
│                                         │      ═══════    │  │    ┌────────┐ │
│                                         │   (Isolation)   │  │    │  5V    │ │
│                                         │      ═══════    │  │    │  Rail  │ │
│                                         │                 │  │    │        │ │
│                                         │  +5V ───────────┼──┼───►│ +5V DC │ │
│    PE (Earth) ─────[To Chassis]         │                 │  │    │        │ │
│                                         │  GND ───────────┼──┴───►│  GND   │ │
│                                         │                 │       │        │ │
│                                         └─────────────────┘       └────────┘ │
│                                                                                 │
│    Component Details:                                                          │
│    ─────────────────                                                           │
│    F1: Fuse, 10A 250V, 5x20mm glass, slow-blow (relay-switched loads only)   │
│    RV1: MOV/Varistor, 275V AC, 14mm disc (surge protection)                   │
│    C1: X2 safety capacitor, 100nF 275V AC (EMI filter)                        │
│                                                                                 │
│    Optional EMI Filter (for CE compliance):                                    │
│    ─────────────────────────────────────────                                   │
│    Add common-mode choke (CMC) between mains input and HLK module             │
│                                                                                 │
│         L ──┬──[CMC]──┬── L (to module)                                       │
│             │    ║    │                                                        │
│         N ──┴────╫────┴── N (to module)                                       │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 5.2 Secondary Power Rails

### 5V to 3.3V LDO Regulator

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                          3.3V LDO REGULATOR CIRCUIT                             │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│                                                                                 │
│      5V Rail                                                     3.3V Rail     │
│         │                                                            │         │
│         │      ┌──────────────────────────────────────┐             │         │
│    ┌────┴────┐ │                                      │        ┌────┴────┐    │
│    │  100µF  │ │     ┌────────────────────┐          │        │  47µF   │    │
│    │ 10V     │ │     │    AMS1117-3.3     │          │        │  6.3V   │    │
│    │ Alum.   │ │     │    or AP2112K-3.3  │          │        │ Tant.   │    │
│    └────┬────┘ │     │                    │          │        └────┬────┘    │
│         │      │     │  VIN         VOUT  │──────────┴─────────────┤         │
│         │      └────►│                    │                        │         │
│         │            │        GND         │                   ┌────┴────┐    │
│         │            └─────────┬──────────┘                   │  100nF  │    │
│         │                      │                              │ Ceramic │    │
│         │                      │                              └────┬────┘    │
│        GND                    GND                                 GND         │
│                                                                                 │
│    Regulator Selection:                                                        │
│    ────────────────────                                                        │
│    Selected: AP2112K-3.3 (SOT-23-5)                                           │
│      - Dropout: 0.4V @ 600mA                                                  │
│      - Max output: 600mA                                                       │
│      - Quiescent: 55µA                                                         │
│      - Low thermal dissipation                                                 │
│                                                                                 │
│    Load Budget (3.3V Rail):                                                    │
│    ────────────────────────                                                    │
│    Pico (including internal regulators): ~50mA typical                        │
│    MAX31855: ~1mA                                                              │
│    NTC dividers: ~1mA brew (3.3kΩ), ~3mA steam (1.2kΩ)                       │
│    ESP32 Module (if 3.3V powered): 150-350mA                                  │
│    Total: ~400mA maximum                                                       │
│                                                                                 │
│    RECOMMENDATION: Power ESP32 from 5V rail instead                           │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 5.3 Power Distribution and Filtering

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                          POWER DISTRIBUTION NETWORK                             │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│                                                                                 │
│    From HLK-PM05                                                               │
│         │                                                                       │
│         │    ┌─────────────────────────────────────────────────────────────┐  │
│         │    │                    5V DISTRIBUTION                          │  │
│         ▼    │                                                              │  │
│    ┌────────┐│    ┌──────┐        ┌──────┐        ┌──────┐                │  │
│    │ 100µF  ├┼───►│ Pico │───────►│Relays│───────►│ SSR  │                │  │
│    │ 16V    ││    │ VSYS │        │Driver│        │Driver│                │  │
│    │ Elect. ││    └───┬──┘        └──────┘        └──────┘                │  │
│    └────┬───┘│        │                                                    │  │
│         │    │   ┌────┴────┐      ┌─────────────────────────────────────┐ │  │
│         │    │   │  100nF  │      │ Place 100nF ceramic at each load   │ │  │
│         │    │   │ Ceramic │      │ Decoupling capacitors              │ │  │
│         │    │   └────┬────┘      └─────────────────────────────────────┘ │  │
│         │    │        │                                                    │  │
│        GND   │       GND                                                   │  │
│         │    └─────────────────────────────────────────────────────────────┘  │
│         │                                                                       │
│         │    ┌─────────────────────────────────────────────────────────────┐  │
│         │    │                 3.3V ANALOG SECTION                          │  │
│         ▼    │      (Isolated from digital noise)                          │  │
│    ┌────────┐│                                                              │  │
│    │ LDO    ││    3.3V_A (Analog)                                          │  │
│    │3.3V    │├────────┬───────────────────────────────────────────────────┐│  │
│    └────┬───┘│        │                                                    ││  │
│         │    │   ┌────┴────┐     ┌────────┐     ┌────────┐               ││  │
│         │    │   │ Ferrite │     │ 22µF   │     │ 100nF  │               ││  │
│         │    │   │  Bead   │     │ Tant.  │     │Ceramic │               ││  │
│         │    │   │ 600Ω    │     │        │     │        │               ││  │
│         │    │   └────┬────┘     └───┬────┘     └───┬────┘               ││  │
│         │    │        │              │              │                     ││  │
│         │    │        └──────────────┴──────────────┴───────► ADC_VREF   ││  │
│         │    │                                                            ││  │
│         │    │   NTC Thermistors ─────────────────────► ADC0, ADC1       ││  │
│         │    │   Pressure Transducer ─────────────────► ADC2             ││  │
│         │    │   MAX31855 ────────────────────────────► SPI              ││  │
│         │    │                                                            ││  │
│        GND   │                                                            ││  │
│         │    └────────────────────────────────────────────────────────────┘│  │
│         │                                                                       │
│         │    ┌─────────────────────────────────────────────────────────────┐  │
│         │    │                 3.3V DIGITAL SECTION                         │  │
│         ▼    │                                                              │  │
│    ┌────────┐│    3.3V_D (Digital)                                         │  │
│    │ LDO    ││────────┬───────────────────────────────────────────────────┐│  │
│    │ Same   ││   ┌────┴────┐                                               ││  │
│    └────┬───┘│   │ 100nF   │   Digital I/O, LEDs, etc.                    ││  │
│         │    │   │ Ceramic │                                               ││  │
│         │    │   └────┬────┘                                               ││  │
│        GND   │       GND                                                   ││  │
│              └─────────────────────────────────────────────────────────────┘│  │
│                                                                                 │
│    Note: 3.3V_A and 3.3V_D can share the same LDO, but use a ferrite bead    │
│    or small inductor to isolate the analog section from digital switching     │
│    noise. Connect grounds at a single point near the Pico ADC_GND pin.        │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 5.4 Decoupling Capacitor Placement

| Location                | Capacitor | Type             | Notes                    |
| ----------------------- | --------- | ---------------- | ------------------------ |
| 5V rail main            | 100µF     | Electrolytic     | Near HLK-PM05 output     |
| 5V at Pico VSYS         | 100nF     | Ceramic (0805)   | Adjacent to pin          |
| 5V at each relay driver | 100nF     | Ceramic (0805)   | Suppress switching noise |
| 3.3V LDO output         | 47µF      | Tantalum/Ceramic | Low ESR required         |
| 3.3V at Pico 3V3        | 100nF     | Ceramic (0805)   | Adjacent to pin          |
| 3.3V at MAX31855        | 100nF     | Ceramic (0805)   | Adjacent to VCC pin      |
| 3.3V at each ADC input  | 100nF     | Ceramic (0603)   | Filter network           |
| AGND/DGND star point    | 10µF      | Ceramic          | Optional, reduces noise  |

---

# 6. Relay & SSR Output Circuits

## 6.1 Relay Driver Circuit

All relays use identical driver circuits with integrated indicator LEDs.

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                           RELAY DRIVER CIRCUIT                                  │
│                    (Identical for K1, K2, K3)                                  │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│                                5V Rail                                          │
│                                  │                                              │
│              ┌───────────────────┴───────────────────┐                         │
│              │                                       │                         │
│         ┌────┴────┐                             ┌────┴────┐                    │
│         │  Relay  │                             │  470Ω   │ ← R30+n           │
│         │  Coil   │                             │  (LED)  │   LED resistor    │
│         │  5V DC  │                             └────┬────┘                    │
│         │         │                                  │                         │
│         └────┬────┘                             ┌────┴────┐                    │
│              │                                  │   LED   │                    │
│         ┌────┴────┐                             │  Green  │  ← Indicator      │
│         │  1N4007 │  ← Flyback diode            │  0805   │                    │
│         │   (D)   │    Catches coil spike       └────┬────┘                    │
│         └────┬────┘                                  │                         │
│              │                                       │                         │
│              └───────────────────┬───────────────────┤                         │
│                                  │                   │                         │
│                             ┌────┴────┐              │                         │
│                             │    C    │              │                         │
│                             │ MMBT2222│              │                         │
│                             │  (Q)    │              │                         │
│     GPIO ─────────[1kΩ]────►│    B    │              │                         │
│    (10-14)        R20+n     │    E    ├──────────────┘                         │
│              │              └────┬────┘                                        │
│         ┌────┴────┐              │                                             │
│         │  10kΩ   │             GND                                            │
│         │R10+n    │  ← Pull-down (relay OFF at boot)                          │
│         └────┬────┘                                                            │
│              │                                                                  │
│             GND                                                                │
│                                                                                 │
│    OPERATION:                                                                  │
│    ──────────                                                                  │
│    GPIO LOW  → Transistor OFF → Relay OFF, LED OFF                            │
│    GPIO HIGH → Transistor ON  → Relay ON, LED ON                              │
│                                                                                 │
│    Relay coil current: 5V / ~70Ω = ~70mA (through transistor)                │
│    LED current: (5V - 2.0V) / 470Ω = 6.4mA (bright indicator)                │
│                                                                                 │
│    Component Specifications:                                                   │
│    ─────────────────────────                                                   │
│    Q: MMBT2222A (SOT-23) - NPN transistor, Ic(max)=600mA, Vce(sat)<0.4V       │
│    Relay: HF46F-G/005-HS1 or SRD-05VDC-SL-C                                   │
│      - Coil: 5V DC, ~75mA                                                      │
│      - Contacts: 10A @ 250V AC (16A for K2 pump relay)                        │
│    1N4007: Flyback diode, 1A, 1000V (DO-41 or MINIMELF)                       │
│    LED: Green 0805, Vf=2.0V, If=6mA                                           │
│    R20+n: 1kΩ 5% 0805 (base resistor)                                         │
│    R30+n: 470Ω 5% 0805 (LED resistor - brighter than 1kΩ)                     │
│    R10+n: 10kΩ 5% 0805 (pull-down)                                            │
│                                                                                 │
│    For Pump Relay (K2) - use higher rated relay:                              │
│    Omron G5LE-1A4 DC5: 16A @ 250V AC, 5V coil                                 │
│                                                                                 │
│    ⚠️  CRITICAL: RC SNUBBER PROTECTION (Inductive Loads)                      │
│    ────────────────────────────────────────────────────                       │
│    The Pump (Ulka) and Solenoid Valves are INDUCTIVE loads. When the relay   │
│    contacts open, they generate high-voltage arcs that cause:                 │
│    • Premature relay contact welding/pitting                                  │
│    • EMI spikes that can reset the RP2040 or freeze I2C/SPI                  │
│    • Even with opto-isolation, RF noise couples into PCB traces              │
│                                                                                 │
│    SOLUTION: Add RC Snubber across relay CONTACTS (HV side)                  │
│    ──────────────────────────────────────────────────────────                │
│                                                                                │
│         Relay Contact (NO)                                                    │
│              │                                                                │
│              ├───────────┐                                                    │
│              │           │                                                    │
│         To  Load     ┌───┴───┐      ┌───────┐                                │
│        (Pump/Valve)  │ 100nF │──────┤ 100Ω  │                                │
│                      │  X2   │      │ 2W    │                                │
│                      │ 275V  │      │ Metal │                                │
│                      └───┬───┘      └───┬───┘                                │
│                          │              │                                     │
│                          └──────┬───────┘                                     │
│                                 │                                             │
│                          Relay COM (or Load return)                           │
│                                                                                │
│    Snubber Component Specifications:                                         │
│    ─────────────────────────────────                                         │
│    C: 100nF X2 capacitor, 275V AC (MUST be X2 safety rated)                 │
│       Example: Vishay MKP1840, TDK B32922                                    │
│    R: 100Ω resistor, 2W, metal film or wirewound                            │
│       Power dissipation: P = V²/R = (340V)²/100Ω = 1.15W (2W provides margin)│
│                                                                                │
│    Snubber Placement on PCB:                                                 │
│    ─────────────────────────                                                 │
│    • Add footprints for C+R in parallel across K2 and K3 contact terminals  │
│    • Place as close as possible to relay contact pads                        │
│    • Snubber is on HIGH VOLTAGE side - maintain clearance to LV section     │
│                                                                                │
│    Which Relays Need Snubbers?                                               │
│    ────────────────────────────                                              │
│    • K2 (Pump) - MANDATORY (Ulka pump generates severe EMI spikes)          │
│    • K3 (3-Way Valve) - MANDATORY (Solenoid back-EMF can crash RP2040)      │
│    • K1 (Water LED) - OPTIONAL (depends on load type)                       │
│    • SSRs (heaters) - NOT NEEDED (resistive load)                           │
│                                                                                │
│    ⚠️  WARNING: Unsnubbed inductive loads (pump/solenoid) generate EMI      │
│    spikes that can cause RP2040 hangs, USB disconnects, and erratic         │
│    behavior. K2 and K3 snubbers are NOT optional for reliable operation.    │
│                                                                                │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 6.2 SSR Trigger Circuit

External SSRs require 4-30V DC trigger input. Pico's 3.3V GPIO cannot drive SSR directly.
Solution: NPN transistor as low-side switch provides full 5V to SSR.

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                           SSR TRIGGER CIRCUIT                                   │
│                    (For SSR1 Brew Heater, SSR2 Steam Heater)                   │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    IMPORTANT: SSR has internal current limiting. No series resistor needed!   │
│    SSR input spec: 4-30V DC, ~10mA (check your SSR datasheet)                 │
│                                                                                 │
│    ⚠️  BEST PRACTICE: Indicator LED is on LOGIC SIDE (GPIO-driven)           │
│    This ensures known current to SSR and avoids current splitting issues.     │
│                                                                                 │
│                                5V Rail                                          │
│                                  │                                              │
│                                  │                                              │
│                                  ▼                                              │
│                        ┌──────────────────┐                                    │
│                        │  To External     │                                    │
│                        │  SSR Input (+)   │                                    │
│                        │                  │                                    │
│                        │ (J26 Pin 19/21)  │                                    │
│                        └────────┬─────────┘                                    │
│                                 │                                              │
│                                 │              ┌───────────────────┐           │
│                                 │              │                   │           │
│                                 │              │  External SSR     │           │
│                                 │              │  (has internal    │           │
│                                 │              │   LED + resistor) │           │
│                                 │              │                   │           │
│                                 │              └───────────────────┘           │
│                                 │                                              │
│                        ┌────────┴─────────┐                                    │
│                        │  To External     │                                    │
│                        │  SSR Input (-)   │                                    │
│                        │                  │                                    │
│                        │ (J26 Pin 20/22)  │                         ┌────────┐ │
│                        └────────┬─────────┘                         │   C    │ │
│                                 │                                   │MMBT2222│ │
│                                 │                                   │ Q5/Q6  │ │
│                                 └──────────────────────────────────►│   E    │ │
│                                                                     └───┬────┘ │
│                                                                         │      │
│    GPIO ────────[1kΩ R24/25]───┬────────────────────────────────────►│B      │
│    (13/14)                      │                                      │      │
│                                 │                                     ─┴─     │
│                            ┌────┴────┐                                GND     │
│                            │  10kΩ   │                                        │
│                            │ R14/15  │                                        │
│                            │Pull-down│                                        │
│                            └────┬────┘                                        │
│                                ─┴─                                            │
│                                GND                                            │
│                                                                                 │
│    INDICATOR LED (Logic Side - separate from SSR drive):                     │
│    ──────────────────────────────────────────────────────                    │
│                                                                                 │
│    GPIO ─────────────[1kΩ]─────┬────────► (shared with transistor base)     │
│    (13/14)                      │                                             │
│                            ┌────┴────┐                                        │
│                            │  330Ω   │  ← LED resistor (separate)            │
│                            │  R34/35 │                                        │
│                            └────┬────┘                                        │
│                                 │                                             │
│                            ┌────┴────┐                                        │
│                            │   LED   │                                        │
│                            │ Orange  │  ← Heater Active Indicator            │
│                            │  0805   │     (logic-side, not SSR-side)        │
│                            └────┬────┘                                        │
│                                 │                                             │
│                                GND                                            │
│                                                                                 │
│    OPERATION:                                                                  │
│    ───────────                                                                 │
│    GPIO LOW  → Transistor OFF → SSR- floating  → SSR OFF (no current path)   │
│              → LED OFF (on logic side)                                        │
│    GPIO HIGH → Transistor ON  → SSR- to GND    → SSR ON  (~5V across SSR)    │
│              → LED ON (on logic side)                                         │
│                                                                                 │
│    Voltage at SSR = 5V - Vce(sat) = 5V - 0.2V = 4.8V  ✓ (exceeds 4V min)     │
│                                                                                 │
│    WHY LED ON LOGIC SIDE?                                                     │
│    ───────────────────────                                                    │
│    • Ensures known, stable current to SSR (not split with LED)               │
│    • SSR current: 5V / R_internal ≈ 10mA (determined by SSR only)           │
│    • LED current: (3.3V - 2.0V) / 330Ω ≈ 4mA (separate path)                │
│    • No current splitting = predictable SSR triggering                        │
│    • Easier troubleshooting (LED and SSR independently testable)             │
│                                                                                 │
│    SSR Specifications (External, user-supplied):                              │
│    ─────────────────────────────────────────────                              │
│    Model: KS15 D-24Z25-LQ (or equivalent)                                     │
│      - Input: 4-32V DC (we provide ~4.8V ✓)                                   │
│      - Input current: ~10mA (handled by SSR internal limiting)                │
│      - Output: 24-280V AC @ 25A (machine is 220-240V)                         │
│      - Zero-cross switching (reduces EMI)                                      │
│                                                                                 │
│    ⚠️  SSRs MUST be mounted on adequate heatsink!                             │
│    ⚠️  Dissipation: ~1W per amp at full load                                  │
│                                                                                 │
│    Connector: J26 Pin 19-20 (SSR1), Pin 21-22 (SSR2)                          │
│                                                                                 │
│    Component Values:                                                           │
│    ─────────────────                                                           │
│    Q5-Q6:   MMBT2222A (SOT-23), Vce(sat) < 0.3V @ 100mA                       │
│    R24-25:  1kΩ 5% 0805 (base drive, ~3mA)                                    │
│    R14-15:  10kΩ 5% 0805 (pull-down, keeps SSR OFF at boot)                   │
│    R34-35:  330Ω 5% 0805 (indicator LED, ~4mA - brighter)                     │
│    LED5-6:  Orange 0805, Vf~2.0V                                              │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 6.3 Relay Contact Wiring

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                          RELAY OUTPUT CONNECTIONS                               │
│                    (6.3mm Spade Terminal Connections)                          │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    Each relay has 3 contacts: COM (Common), NO (Normally Open), NC (Closed)   │
│    This design uses only COM and NO for each relay.                           │
│                                                                                 │
│    K1 - WATER STATUS LED                                                       │
│    ─────────────────────                                                       │
│         COM ──[6.3mm]──► To machine LED power (typically 12V or 24V)          │
│         NO  ──[6.3mm]──► To LED (+)                                           │
│         NC  ── Not connected                                                   │
│                                                                                 │
│    K2 - PUMP MOTOR (Use 16A relay!)                                           │
│    ────────────────────────────────                                           │
│         COM ──[6.3mm]──► From mains Live (via machine wiring)                 │
│         NO  ──[6.3mm]──► To pump motor                                        │
│         NC  ── Not connected                                                   │
│                                                                                 │
│    K3 - SOLENOID VALVE (3-Way)                                                │
│    ───────────────────────────                                                 │
│         COM ──[6.3mm]──► From mains Live (via machine wiring)                 │
│         NO  ──[6.3mm]──► To solenoid coil                                     │
│         NC  ── Not connected                                                   │
│                                                                                 │
│                                                                                 │
│    ⚠️  HIGH VOLTAGE WARNING:                                                  │
│    ─────────────────────────                                                   │
│    Relay contacts switch mains voltage (100-240V AC).                         │
│    Maintain 6mm minimum creepage between contacts and control circuits.       │
│    Use PCB slots if necessary to achieve required clearance.                  │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

---

# 7. Sensor Input Circuits

## 7.1 NTC Thermistor Interface

For **50kΩ @ 25°C NTC thermistors** (typical B25/85 = 3950K) - ECM Synchronika standard

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                         NTC THERMISTOR INPUT CIRCUITS                           │
│          ⚠️  EACH SENSOR OPTIMIZED FOR ITS TARGET TEMPERATURE RANGE            │
│          ⚠️  CONFIGURED FOR 50kΩ NTC SENSORS (NOT 3.3kΩ!)                      │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    BREW BOILER (GPIO26/ADC0)              STEAM BOILER (GPIO27/ADC1)           │
│    Target: 90-96°C                        Target: 125-150°C                    │
│                                                                                 │
│         3.3V (Analog)                          3.3V (Analog)                   │
│             │                                      │                           │
│        ┌────┴────┐                            ┌────┴────┐                      │
│        │  3.3kΩ  │  ← R1                      │  1.2kΩ  │  ← R2                │
│        │  ±1%    │    Optimized for           │  ±1%    │    Optimized for     │
│        │  0805   │    93°C target             │  0805   │    135°C target      │
│        └────┬────┘                            └────┬────┘                      │
│             │                                      │                           │
│    NTC     ┌┴─────┬────────┐          NTC        ┌┴─────┬────────┐            │
│    50kΩ    │      │        │          50kΩ       │      │        │            │
│    @25°C ──┤   ┌──┴──┐  ┌──┴──┐       @25°C ─────┤   ┌──┴──┐  ┌──┴──┐         │
│            │   │ 1kΩ │  │100nF│                  │   │ 1kΩ │  │100nF│         │
│            │   │ R5  │  │     │                  │   │ R6  │  │     │         │
│            │   └──┬──┘  └──┬──┘                  │   └──┬──┘  └──┬──┘         │
│            │      ▼        │                     │      ▼        │            │
│            │   ADC0        │                     │   ADC1        │            │
│            │               │                     │               │            │
│            └───────────────┴─► GND               └───────────────┴─► GND      │
│                                                                                 │
│    ═══════════════════════════════════════════════════════════════════════    │
│                                                                                 │
│    WHY DIFFERENT PULL-UP RESISTORS?                                           │
│    ─────────────────────────────────                                          │
│    Maximum ADC sensitivity occurs when R_pullup ≈ R_NTC at target temp.       │
│    • Brew at 93°C: NTC ≈ 4.3kΩ → optimal pull-up ~3.3kΩ                      │
│    • Steam at 135°C: NTC ≈ 1.4kΩ → optimal pull-up ~1.2kΩ                    │
│                                                                                 │
│    Using the same resistor for both would compromise one or the other.        │
│                                                                                 │
│    ═══════════════════════════════════════════════════════════════════════    │
│                                                                                 │
│    BREW NTC with 3.3kΩ PULL-UP (R1) - Optimized for 90-100°C:                 │
│    ─────────────────────────────────────────────────────────                  │
│    │  Temp  │ R_NTC  │  Vout   │ ADC Count │ ADC % │ Δ/°C  │                 │
│    │──────────────────────────────────────────────────────────│                 │
│    │  25°C  │ 50.0kΩ │  3.10V  │   3843    │  94%  │   -   │  ← Room temp   │
│    │  85°C  │  5.5kΩ │  2.06V  │   2555    │  62%  │   -   │                 │
│    │  90°C  │  4.7kΩ │  1.94V  │   2408    │  59%  │  29   │  ← TARGET      │
│    │  93°C  │  4.3kΩ │  1.87V  │   2315    │  57%  │  31   │  ← TARGET      │
│    │  96°C  │  3.9kΩ │  1.79V  │   2217    │  54%  │  33   │  ← TARGET      │
│    │ 100°C  │  3.5kΩ │  1.70V  │   2104    │  51%  │  28   │                 │
│                                                                                 │
│    Resolution at brew temps (90-96°C): ~31 ADC counts/°C → 0.032°C           │
│    Voltage swing centered near 55% ADC = excellent usable range               │
│                                                                                 │
│    ═══════════════════════════════════════════════════════════════════════    │
│                                                                                 │
│    STEAM NTC with 1.2kΩ PULL-UP (R2) - Optimized for 125-150°C:               │
│    ─────────────────────────────────────────────────────────                  │
│    │  Temp  │ R_NTC  │  Vout   │ ADC Count │ ADC % │ Δ/°C  │                 │
│    │──────────────────────────────────────────────────────────│                 │
│    │  25°C  │ 50.0kΩ │  3.22V  │   3996    │  98%  │   -   │  ← Room temp   │
│    │ 125°C  │  1.8kΩ │  1.98V  │   2455    │  60%  │   -   │                 │
│    │ 130°C  │  1.6kΩ │  1.89V  │   2341    │  57%  │  23   │                 │
│    │ 135°C  │  1.4kΩ │  1.78V  │   2203    │  54%  │  28   │  ← TARGET      │
│    │ 140°C  │  1.25kΩ│  1.68V  │   2086    │  51%  │  23   │  ← TARGET      │
│    │ 145°C  │  1.1kΩ │  1.58V  │   1957    │  48%  │  26   │                 │
│    │ 150°C  │  1.0kΩ │  1.50V  │   1861    │  45%  │  19   │  ← MAX TEMP    │
│                                                                                 │
│    Resolution at steam temps (130-145°C): ~25 ADC counts/°C → 0.04°C         │
│                                                                                 │
│    ═══════════════════════════════════════════════════════════════════════    │
│                                                                                 │
│    COMPONENT VALUES:                                                           │
│    ─────────────────                                                           │
│    R1:  3.3kΩ ±1% 0805 (Brew NTC pull-up) - NOT same as R2!                  │
│    R2:  1.2kΩ ±1% 0805 (Steam NTC pull-up) - NOT same as R1!                 │
│    R5:  1kΩ ±5% 0805 (ADC series protection, brew)                           │
│    R6:  1kΩ ±5% 0805 (ADC series protection, steam)                          │
│    NTC: 50kΩ @ 25°C, B25/85 ≈ 3950K (ECM Synchronika standard)               │
│                                                                                 │
│    NOTE: At 25°C the voltage is near 3.3V (high) but this is normal.         │
│          We optimize for operating temperature, not room temp.                 │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 7.2 K-Type Thermocouple Interface

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                      K-TYPE THERMOCOUPLE INTERFACE                              │
│                         (MAX31855K SPI Amplifier)                              │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    K-Type                                                                      │
│  Thermocouple                          MAX31855KASA+                           │
│  (From brew head)                     ┌─────────────┐                          │
│        ║                           1  │             │  8                       │
│     (+)╠══════════════════════════════│ T+      Vcc ├────────► 3.3V           │
│        ║                   10nF    2  │             │  7                       │
│        ║              ┌────┼──────────│ T-      SCK ├────────► GPIO18         │
│     (-)╠══════════════┤    │       3  │             │  6                       │
│        ║              │   GND      NC─│ NC       CS ├────────► GPIO17         │
│                       │            4  │             │  5                       │
│                       │       GND ────│ GND      DO ├────────► GPIO16         │
│                       │               └─────────────┘                          │
│                       │                                                        │
│                      GND                                                       │
│                                                                                 │
│    Component: MAX31855KASA+ (SOIC-8)                                          │
│    ─────────────────────────────────                                          │
│    - K-type thermocouple specific                                             │
│    - 14-bit, 0.25°C resolution                                                │
│    - Range: -200°C to +1350°C                                                 │
│    - Accuracy: ±2°C (-20°C to +125°C ambient)                                │
│    - Cold junction compensation included                                       │
│    - SPI interface (read-only, 32-bit frame)                                  │
│    - Fault detection (open, short to GND, short to VCC)                       │
│                                                                                 │
│    PCB Layout Notes:                                                           │
│    ──────────────────                                                          │
│    - Place MAX31855 near thermocouple connector                               │
│    - Keep T+ and T- traces short and symmetric                                │
│    - Guard ring around T+ and T- (connected to GND)                           │
│    - Add 10nF ceramic between T+ and T- (close to IC)                         │
│    - Use shielded cable for thermocouple wires                                │
│    - Ground shield at PCB end only (avoid ground loops)                       │
│    - Keep away from power traces and relay coils                              │
│                                                                                 │
│    Thermocouple Connector: 2-pin screw terminal (+ and -)                     │
│    Use proper thermocouple connector if long cable run is needed              │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 7.3 Pressure Transducer Interface

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                     PRESSURE TRANSDUCER INPUT CIRCUIT                           │
│                       (0.5-4.5V Ratiometric Output)                            │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    Pressure Transducer                                                         │
│    (3-wire: Vcc, GND, Signal)                                                 │
│                                                                                 │
│         5V ─────────────────────────────────────┬────► Transducer Vcc         │
│                                                 │      (Red wire)              │
│                                                 │                              │
│        GND ─────────────────────────────────────┼────► Transducer GND         │
│                                                 │      (Black wire)            │
│                                                 │                              │
│    Transducer Signal ───────────────────────────┘      (Yellow/White wire)    │
│    (0.5V - 4.5V)                                                               │
│         │                                                                      │
│         │       Voltage Divider (5V → 3.3V range)                             │
│         │                                                                      │
│    ┌────┴────┐                                                                 │
│    │  4.7kΩ  │  ← R4: Series resistor, 1% precision                           │
│    │  ±1%    │                                                                 │
│    └────┬────┘                                                                 │
│         │                                                                      │
│         ├───────────────────────────┬───────────────────────► GPIO28 (ADC2)   │
│         │                           │                                          │
│    ┌────┴────┐                 ┌────┴────┐                                    │
│    │  10kΩ   │                 │  100nF  │  ← RC filter                       │
│    │  ±1%    │  ← R3: To GND   │ Ceramic │    fc ≈ 100 Hz                     │
│    └────┬────┘                 └────┬────┘                                    │
│         │                           │                                          │
│        GND                         GND                                         │
│                                                                                 │
│    Voltage Divider Calculation (OPTIMIZED for ADC range):                      │
│    ──────────────────────────────────────────────────────                      │
│    Ratio = R3 / (R3 + R4) = 10k / (10k + 4.7k) = 0.68                         │
│                                                                                 │
│    Input 0.5V → Output: 0.5V × 0.68 = 0.34V → ADC: 422                        │
│    Input 2.5V → Output: 2.5V × 0.68 = 1.70V → ADC: 2109 (midscale)           │
│    Input 4.5V → Output: 4.5V × 0.68 = 3.06V → ADC: 3795                       │
│                                                                                 │
│    ADC utilization: 91% of full scale                                         │
│    Safe margin: 3.06V still below 3.3V ADC maximum                            │
│                                                                                 │
│    Connector: 3-pin screw terminal (5V, GND, Signal)                          │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

### Selected Transducer: YD4060

| Parameter             | Specification                                          |
| --------------------- | ------------------------------------------------------ |
| Model                 | YD4060 Series                                          |
| **Pressure Range**    | **0-1.6 MPa (0-16 bar)** ⚠️ Order correct range!       |
| Output Signal         | 0.5-4.5V ratiometric                                   |
| Supply Voltage        | 5VDC                                                   |
| Operating Current     | ≤3mA                                                   |
| Accuracy              | ±1.0% FS                                               |
| Operating Temperature | -40°C to +105°C                                        |
| Response Time         | ≤3ms                                                   |
| Thread                | 1/8" NPT (use G1/8 or M6 adapter for espresso machine) |
| Housing               | 304 Stainless Steel, IP65                              |
| CE Certified          | Yes                                                    |

**Wiring (3-wire cable):**

| Wire Color   | Function          | Connect To |
| ------------ | ----------------- | ---------- |
| Red          | Vcc (+5V)         | J26 Pin 14 |
| Black        | GND               | J26 Pin 15 |
| Yellow/White | Signal (0.5-4.5V) | J26 Pin 16 |

**Pressure to Voltage/ADC Mapping (0-16 bar range, optimized divider):**

| Pressure             | Voltage Out | After Divider | ADC Count |
| -------------------- | ----------- | ------------- | --------- |
| 0 bar                | 0.5V        | 0.34V         | 422       |
| 4 bar                | 1.5V        | 1.02V         | 1266      |
| 8 bar                | 2.5V        | 1.70V         | 2109      |
| 9 bar (typical brew) | 2.75V       | 1.87V         | 2320      |
| 12 bar               | 3.5V        | 2.38V         | 2953      |
| 16 bar               | 4.5V        | 3.06V         | 3795      |

**Resolution:** 16 bar / (3795 - 422) = **0.0047 bar per ADC count**

**Firmware Conversion:**

```python
def adc_to_pressure(adc_count, range_bar=16):
    """Convert ADC reading to pressure in bar"""
    ADC_MIN = 422   # 0 bar (0.5V input, after 0.68 divider)
    ADC_MAX = 3795  # 16 bar (4.5V input, after 0.68 divider)

    # Clamp to valid range
    adc_count = max(ADC_MIN, min(ADC_MAX, adc_count))

    # Linear interpolation
    pressure = (adc_count - ADC_MIN) / (ADC_MAX - ADC_MIN) * range_bar
    return pressure
```

⚠️ **Important:** When ordering, specify:

- Pressure range: **0-1.6 MPa (16 bar)** - NOT 0-60 MPa
- Output: **0.5-4.5V**
- Supply: **5VDC**
- Thread: **G1/8** or request M6 adapter

## 7.4 Digital Switch Inputs

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                         DIGITAL SWITCH INPUT CIRCUIT                            │
│              (Water Reservoir, Tank Level, Brew Handle Switches)               │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│                          3.3V                                                  │
│                           │                                                     │
│                      ┌────┴────┐                                               │
│                      │  10kΩ   │  ← External pull-up (or use Pico internal)   │
│                      │         │                                               │
│                      └────┬────┘                                               │
│                           │                                                     │
│    From Switch            ├─────────────────────────────────► GPIO (2,3,5)    │
│    (6.3mm spade)         │                                                     │
│         │                 │                                                     │
│         │            ┌────┴────┐                                               │
│    ┌────┴────┐       │  ESD    │  ← ESD protection                            │
│    │  Switch │       │ Clamp   │    PESD5V0S1BL or similar                    │
│    │   N.O.  │       │ Bi-dir  │                                               │
│    └────┬────┘       └────┬────┘                                               │
│         │                 │                                                     │
│         └─────────────────┴───────────────────────────────────► GND           │
│                                                                                 │
│    Logic:                                                                      │
│    ───────                                                                     │
│    Switch OPEN   → GPIO reads HIGH (3.3V) → Switch inactive                   │
│    Switch CLOSED → GPIO reads LOW  (0V)   → Switch active                     │
│                                                                                 │
│    Debouncing: Implement in software (50-100ms debounce time)                 │
│                                                                                 │
│    Optional hardware debounce (add if EMI is problematic):                    │
│    ───────────────────────────────────────────────────────                    │
│         │                                                                      │
│         ├───[10kΩ]───┬───► GPIO                                               │
│         │            │                                                         │
│    ┌────┴────┐  ┌────┴────┐                                                   │
│    │ Switch  │  │  100nF  │  ← RC time constant ~1ms                          │
│    └────┬────┘  └────┬────┘                                                   │
│         │            │                                                         │
│        GND          GND                                                        │
│                                                                                 │
│    Connector: 6.3mm spade terminal (2 per switch: Signal, GND)               │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 7.5 Steam Boiler Level Probe (OPA342 + TLV3201 AC Sensing)

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                       STEAM BOILER LEVEL PROBE CIRCUIT                          │
│                    (AC Sensing with OPA342 + TLV3201)                          │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    ⚠️  CRITICAL: DC-based level detection causes ELECTROLYSIS and probe       │
│    corrosion. This circuit uses AC excitation to prevent electrode damage.     │
│                                                                                 │
│    This design uses commonly available, modern components:                     │
│    • OPA342 (or OPA207): Rail-to-rail op-amp for AC oscillator generation     │
│    • TLV3201: Precision comparator for level detection with hysteresis        │
│    • All components readily available from major distributors                  │
│                                                                                 │
│    ═══════════════════════════════════════════════════════════════════════    │
│                                                                                 │
│                         +3.3V                     +3.3V                        │
│                           │                         │                          │
│                      ┌────┴────┐               ┌────┴────┐                     │
│                      │  100nF  │ C60           │  100nF  │ C63                 │
│                      └────┬────┘               └────┬────┘                     │
│                           │                         │                          │
│    ┌──────────────────────┴─────────────────────────┴───────────────────────┐  │
│    │                                                                         │  │
│    │    STAGE 1: WIEN BRIDGE OSCILLATOR (~1kHz)                             │  │
│    │    ───────────────────────────────────────                              │  │
│    │                                                                         │  │
│    │                      +3.3V                                              │  │
│    │                        │                                                │  │
│    │                   ┌────┴────┐                                           │  │
│    │              VCC──┤         │                                           │  │
│    │                   │ OPA342  │ U6                                        │  │
│    │    ┌────[R91]─────┤-  (A)   ├──┬────────────────────► AC_OUT            │  │
│    │    │    10kΩ      │         │  │                      (to probe)        │  │
│    │    │         ┌────┤+        │  │                                        │  │
│    │    │         │    │   GND   │  │                                        │  │
│    │    │         │    └────┬────┘  │                                        │  │
│    │    │         │         │       │                                        │  │
│    │    │         │        ─┴─      │                                        │  │
│    │    │         │        GND      │                                        │  │
│    │    │         │                 │                                        │  │
│    │    │    ┌────┴────┐       ┌────┴────┐                                   │  │
│    │    │    │   10kΩ  │       │   10kΩ  │                                   │  │
│    │    │    │   R92   │       │   R93   │                                   │  │
│    │    │    └────┬────┘       └────┬────┘                                   │  │
│    │    │         │                 │                                        │  │
│    │    │         ├─────────────────┤                                        │  │
│    │    │         │                 │                                        │  │
│    │    │    ┌────┴────┐       ┌────┴────┐                                   │  │
│    │    │    │  100nF  │       │  100nF  │                                   │  │
│    │    │    │   C61   │       │   C62   │                                   │  │
│    │    │    └────┬────┘       └────┬────┘                                   │  │
│    │    │         │                 │                                        │  │
│    │    │        ─┴─               ─┴─                                       │  │
│    │    │        GND               GND                                       │  │
│    │    │                                                                    │  │
│    │    └───────────────────────────────────────────────────────────────┐    │  │
│    │                                                                     │    │  │
│    │    Oscillator frequency: f = 1/(2π × R × C) ≈ 160 Hz               │    │  │
│    │    (with 10kΩ and 100nF, adjust R92/R93 for different frequencies) │    │  │
│    │                                                                     │    │  │
│    └─────────────────────────────────────────────────────────────────────┘    │  │
│                                                                                 │
│    ┌─────────────────────────────────────────────────────────────────────────┐│
│    │                                                                          ││
│    │    STAGE 2: PROBE & SIGNAL CONDITIONING                                 ││
│    │    ─────────────────────────────────────────                            ││
│    │                                                                          ││
│    │    AC_OUT ───[100Ω R94]───┬────────────────► J26 Pin 5 (Level Probe)    ││
│    │           (current limit) │                  Screw terminal             ││
│    │                           │                       │                     ││
│    │                      ┌────┴────┐             ┌────┴────┐                ││
│    │                      │  1µF    │             │  Probe  │                ││
│    │                      │  C64    │             │   ~~~   │                ││
│    │                      │ (AC     │             │  Water  │                ││
│    │                      │coupling)│             │  Level  │                ││
│    │                      └────┬────┘             └────┬────┘                ││
│    │                           │                       │                     ││
│    │                           │                  Boiler Body                ││
│    │                           │                  (Ground via PE)            ││
│    │                           │                      ─┴─                    ││
│    │                           ▼                      GND                    ││
│    │                       AC_SENSE                                          ││
│    │                           │                                             ││
│    │    When water present: AC signal coupled through water conductivity    ││
│    │    When dry: No AC signal returned (open circuit)                      ││
│    │                                                                          ││
│    └──────────────────────────────────────────────────────────────────────────┘│
│                                                                                 │
│    ┌─────────────────────────────────────────────────────────────────────────┐│
│    │                                                                          ││
│    │    STAGE 3: RECTIFIER & COMPARATOR                                      ││
│    │    ───────────────────────────────                                      ││
│    │                                                                          ││
│    │    AC_SENSE ────────────────┬─────────────────────┐                     ││
│    │                             │                     │                     ││
│    │                        ┌────┴────┐           ┌────┴────┐                ││
│    │                        │   10kΩ  │           │  100nF  │                ││
│    │                        │   R95   │           │   C65   │                ││
│    │                        │ (Bias)  │           │(Filter) │                ││
│    │                        └────┬────┘           └────┬────┘                ││
│    │                             │                     │                     ││
│    │                             ├─────────────────────┤                     ││
│    │                             │                     │                     ││
│    │                             │                    ─┴─                    ││
│    │                             │                    GND                    ││
│    │                             │                                           ││
│    │                             ▼                                           ││
│    │                         DC_LEVEL                                        ││
│    │                             │                                           ││
│    │    ┌────────────────────────┴───────────────────────────────────────┐  ││
│    │    │                                                                 │  ││
│    │    │                          +3.3V                                  │  ││
│    │    │                            │                                    │  ││
│    │    │                       ┌────┴────┐                               │  ││
│    │    │                  VCC──┤         │                               │  ││
│    │    │                       │TLV3201  │ U7                            │  ││
│    │    │    DC_LEVEL ──────────┤+        ├──────────────────► GPIO4      │  ││
│    │    │                       │   (Cmp) │                   (Digital)   │  ││
│    │    │    VREF ──────────────┤-        │                               │  ││
│    │    │    (from divider)     │   GND   │                               │  ││
│    │    │                       └────┬────┘                               │  ││
│    │    │                            │                                    │  ││
│    │    │                           ─┴─                                   │  ││
│    │    │                           GND                                   │  ││
│    │    │                                                                 │  ││
│    │    │    REFERENCE VOLTAGE DIVIDER (sets threshold):                  │  ││
│    │    │                                                                 │  ││
│    │    │    +3.3V ────[100kΩ R96]───┬───[100kΩ R97]──── GND              │  ││
│    │    │                            │                                    │  ││
│    │    │                            └───► VREF (~1.65V)                  │  ││
│    │    │                                                                 │  ││
│    │    │    HYSTERESIS (via positive feedback):                          │  ││
│    │    │    GPIO4 ────[1MΩ R98]────► + input of TLV3201                  │  ││
│    │    │    (provides ~50mV hysteresis for clean switching)              │  ││
│    │    │                                                                 │  ││
│    │    └─────────────────────────────────────────────────────────────────┘  ││
│    │                                                                          ││
│    └──────────────────────────────────────────────────────────────────────────┘│
│                                                                                 │
│    ═══════════════════════════════════════════════════════════════════════    │
│                                                                                 │
│    OPERATION:                                                                  │
│    ───────────                                                                 │
│    1. OPA342 generates ~160Hz AC oscillation (Wien bridge)                    │
│    2. AC signal excites probe through C64 (DC blocking capacitor)            │
│    3. If water present: AC signal conducts through water to boiler (GND)     │
│    4. Return AC signal is rectified/filtered, raising DC_LEVEL               │
│    5. TLV3201 compares DC_LEVEL to VREF threshold                            │
│    6. Output goes LOW when water detected (DC_LEVEL > VREF)                  │
│    7. Hysteresis resistor R98 prevents output chatter                        │
│                                                                                 │
│    Logic:                                                                      │
│    ───────                                                                     │
│    Water below probe → GPIO4 reads HIGH → Refill needed (UNSAFE for heater)  │
│    Water at/above probe → GPIO4 reads LOW → Level OK                         │
│                                                                                 │
│    Benefits of OPA342 + TLV3201 Design:                                       │
│    ─────────────────────────────────────                                      │
│    • Uses commonly available, modern components                              │
│    • AC excitation prevents electrolysis and probe corrosion                 │
│    • All logic on PCB - no external modules                                  │
│    • Low power consumption (<1mA total)                                      │
│    • Adjustable threshold via R96/R97 divider                                │
│    • Clean hysteresis via R98                                                │
│    • Works with any conductive liquid                                        │
│                                                                                │
│    ⚠️  OSCILLATOR WAVEFORM NOTE (For Testing):                               │
│    ─────────────────────────────────────────────                              │
│    A classic Wien Bridge requires AGC (non-linear feedback) to produce a     │
│    clean sine wave. Without AGC, the op-amp saturates to the supply rails,   │
│    producing a clipped sine or square wave. THIS IS ACCEPTABLE for water     │
│    conductivity sensing - we only need AC excitation, not a pure sine.       │
│                                                                                │
│    During testing: If the oscillator outputs a ~160Hz rail-to-rail square    │
│    wave, the circuit is working correctly. Do NOT attempt to tune for a      │
│    perfect sine wave - it is unnecessary and may destabilize the circuit.    │
│                                                                                 │
│    Component Values:                                                          │
│    ──────────────────                                                         │
│    U6:  OPA342UA (SOIC-8) or OPA2342UA (dual, use one section)              │
│         Alt: OPA207 (lower noise, higher precision)                          │
│    U7:  TLV3201AIDBVR (SOT-23-5) - rail-to-rail comparator                  │
│    R91: 10kΩ 1% 0805 (oscillator feedback)                                   │
│    R92: 10kΩ 1% 0805 (Wien bridge)                                           │
│    R93: 10kΩ 1% 0805 (Wien bridge)                                           │
│    R94: 100Ω 5% 0805 (probe current limit)                                   │
│    R95: 10kΩ 5% 0805 (AC bias)                                               │
│    R96: 100kΩ 1% 0805 (reference divider)                                    │
│    R97: 100kΩ 1% 0805 (reference divider)                                    │
│    R98: 1MΩ 5% 0805 (hysteresis)                                             │
│    C60: 100nF 25V ceramic 0805 (OPA342 VCC decoupling)                       │
│    C61: 100nF 25V ceramic 0805 (Wien bridge timing)                          │
│    C62: 100nF 25V ceramic 0805 (Wien bridge timing)                          │
│    C63: 100nF 25V ceramic 0805 (TLV3201 VCC decoupling)                      │
│    C64: 1µF 25V ceramic 0805 (AC coupling to probe)                          │
│    C65: 100nF 25V ceramic 0805 (sense filter)                                │
│                                                                                 │
│    Threshold Adjustment:                                                      │
│    ─────────────────────                                                      │
│    • Adjust R96/R97 ratio to change detection threshold                      │
│    • For more sensitive detection: increase R97 (lower VREF)                 │
│    • For less sensitive detection: decrease R97 (higher VREF)                │
│    • Typical tap water works well with 1:1 ratio (VREF = 1.65V)              │
│                                                                                 │
│    Firmware Implementation:                                                   │
│    ─────────────────────────                                                  │
│    • Configure GPIO4 as digital input (no PWM needed!)                       │
│    • Read GPIO4 state directly                                               │
│    • Implement simple debouncing (3-5 consecutive readings)                  │
│                                                                                 │
│    ⚠️  CRITICAL SAFETY:                                                       │
│    ────────────────────                                                        │
│    If GPIO4 reads HIGH, the steam boiler heater MUST be disabled immediately.│
│    Running the heater without water will destroy the heating element.         │
│    Implement as highest-priority interlock in firmware.                        │
│                                                                                 │
│    Connector: 6.3mm spade terminal (single wire - probe only)                │
│    Ground return is through boiler body via machine chassis (PE)              │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

---

# 8. Communication Interfaces

## 8.1 ESP32 Display Module Interface (UART0)

**8-pin JST-XH connector** for external ESP32-based display module.

- ESP32 has full control over Pico: RUN (reset) and BOOTSEL (bootloader entry) for OTA
- **NEW: Brew-by-Weight support** via WEIGHT_STOP signal (Pin 7)
- **SPARE pin** (Pin 8) reserved for future expansion

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                       ESP32 DISPLAY MODULE INTERFACE                            │
│                         (J15 - JST-XH 8-Pin 2.54mm)                            │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    Pin Assignment:                                                             │
│    ───────────────                                                             │
│    ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐                          │
│    │  1  │  2  │  3  │  4  │  5  │  6  │  7  │  8  │                          │
│    │ 5V  │ GND │ TX  │ RX  │ RUN │BOOT │WGHT │SPARE│                          │
│    └──┬──┴──┬──┴──┬──┴──┬──┴──┬──┴──┬──┴──┬──┴──┬──┘                          │
│       │     │     │     │     │     │     │     │                              │
│       │     │     │     │     │     │     │     └──► GPIO22 (Future use)      │
│       │     │     │     │     │     │     └────────► GPIO21 (WEIGHT_STOP)     │
│       │     │     │     │     │     └──────────────► Pico BOOTSEL (bootloader)│
│       │     │     │     │     └────────────────────► Pico RUN pin (reset)     │
│       │     │     │     └──────────────────────────► GPIO1 (UART0 RX←ESP TX)  │
│       │     │     └────────────────────────────────► GPIO0 (UART0 TX→ESP RX)  │
│       │     └──────────────────────────────────────► Ground                   │
│       └────────────────────────────────────────────► 5V Power to ESP32        │
│                                                                                 │
│    ════════════════════════════════════════════════════════════════════════   │
│    ✅ ESP32 CONTROLS PICO FOR OTA FIRMWARE UPDATES                            │
│    ════════════════════════════════════════════════════════════════════════   │
│    • ESP32 updates ITSELF via WiFi OTA (standard ESP-IDF)                     │
│    • ESP32 updates PICO via UART + RUN/BOOTSEL control                        │
│    • Pico has no WiFi - relies on ESP32 as update gateway                     │
│    ════════════════════════════════════════════════════════════════════════   │
│                                                                                 │
│    UART Communication Circuit:                                                 │
│    ───────────────────────────                                                 │
│                                                                                 │
│    GPIO0 (TX) ────[33Ω]────┬────────────────────────► J15 Pin 3 (TX→ESP RX)  │
│                            │                                                   │
│                       ┌────┴────┐                                             │
│                       │  100pF  │  ← Optional HF filter                       │
│                       └────┬────┘                                             │
│                           GND                                                  │
│                                                                                 │
│    GPIO1 (RX) ◄───[33Ω]────┬────────────────────────── J15 Pin 4 (RX←ESP TX) │
│                            │                                                   │
│                       ┌────┴────┐                                             │
│                       │  100pF  │                                             │
│                       └────┬────┘                                             │
│                           GND                                                  │
│                                                                                 │
│    ────────────────────────────────────────────────────────────────────────   │
│    ESP32 → PICO RUN (Pin 5): ESP32 resets Pico                                │
│    ────────────────────────────────────────────────────────────────────────   │
│                                                                                 │
│                          3.3V                                                  │
│                           │                                                     │
│                      ┌────┴────┐                                               │
│                      │  10kΩ   │  ← Pull-up (Pico has internal, but add ext.) │
│                      └────┬────┘                                               │
│                           │                                                     │
│    Pico RUN pin ◄─────────┴────────────────────────── J15 Pin 5 (RUN)         │
│                                                                                 │
│    ESP32 GPIO: Open-drain output. Pull LOW to reset, release to run.         │
│                                                                                 │
│    ────────────────────────────────────────────────────────────────────────   │
│    ESP32 → PICO BOOTSEL (Pin 6): ESP32 controls bootloader entry              │
│    ────────────────────────────────────────────────────────────────────────   │
│                                                                                 │
│                          3.3V                                                  │
│                           │                                                     │
│                      ┌────┴────┐                                               │
│                      │  10kΩ   │  ← Pull-up (BOOTSEL normally high)           │
│                      └────┬────┘                                               │
│                           │                                                     │
│    Pico BOOTSEL ◄─────────┴────────────────────────── J15 Pin 6 (BOOT)        │
│                                                                                 │
│    ESP32 GPIO: Open-drain output. Hold LOW during reset to enter bootloader. │
│                                                                                 │
│    ════════════════════════════════════════════════════════════════════════   │
│    ESP32 OTA UPDATE SEQUENCE (Pico firmware update via WiFi):                 │
│    ════════════════════════════════════════════════════════════════════════   │
│                                                                                 │
│    METHOD 1: Serial Bootloader (✅ IMPLEMENTED)                                │
│    ──────────────────────────────────────────                                  │
│    1. ESP32 downloads new Pico firmware via WiFi                              │
│    2. ESP32 sends MSG_CMD_BOOTLOADER (0x1F) command via UART                  │
│    3. Pico serial bootloader acknowledges, enters bootloader mode             │
│    4. ESP32 streams firmware binary via UART (921600 baud)                    │
│    5. Pico bootloader receives chunks, verifies checksums, writes to flash     │
│    6. Pico resets automatically after successful update                       │
│    7. Pico boots with new firmware                                            │
│                                                                                 │
│    METHOD 2: Hardware Bootloader Entry (recovery)                             │
│    ───────────────────────────────────────────────                             │
│    1. ESP32 holds BOOTSEL (Pin 6) LOW                                         │
│    2. ESP32 pulses RUN (Pin 5) LOW then releases                              │
│    3. Pico enters USB bootloader mode (BOOTSEL held during reset)             │
│    4. ESP32 releases BOOTSEL                                                  │
│    5. Pico appears as USB mass storage OR custom UART bootloader activates    │
│    6. ESP32 streams firmware via UART                                         │
│                                                                                 │
│    RECOVERY (if Pico firmware is completely corrupted):                       │
│    ─────────────────────────────────────────────────────                       │
│    • Method 1 (serial bootloader) is fully implemented and preferred          │
│    • Method 2 (hardware bootloader entry) available as fallback/recovery      │
│    • OR use J16 service port with USB-UART adapter + BOOTSEL button          │
│                                                                                 │
│    ════════════════════════════════════════════════════════════════════════   │
│    BREW-BY-WEIGHT SIGNAL (Pin 7 - WEIGHT_STOP → GPIO21)                       │
│    ════════════════════════════════════════════════════════════════════════   │
│                                                                                 │
│    Purpose: Allows ESP32 (connected to Bluetooth scale) to signal Pico        │
│             to stop the brew when target weight is reached.                    │
│                                                                                 │
│                          3.3V                                                  │
│                           │                                                     │
│                      ┌────┴────┐                                               │
│                      │  10kΩ   │  ← Pull-down (normally LOW)                   │
│                      └────┬────┘                                               │
│                           │                                                     │
│    GPIO21 ◄───────────────┴────────────────────────── J15 Pin 7 (WEIGHT_STOP) │
│                                                                                 │
│    Logic:                                                                       │
│    ───────                                                                      │
│    • ESP32 monitors Bluetooth scale (e.g., Acaia, Decent, Felicita)           │
│    • When target weight reached → ESP32 sets Pin 7 HIGH                        │
│    • Pico reads GPIO21 HIGH → Immediately stops pump (K2)                     │
│    • ESP32 releases Pin 7 → Returns LOW (ready for next brew)                 │
│                                                                                 │
│    Timing: ESP32 should hold HIGH for at least 100ms for reliable detection.  │
│    Alternative: Pico can also poll scale weight via UART and make decision.   │
│                                                                                 │
│    ────────────────────────────────────────────────────────────────────────   │
│    SPARE PIN (Pin 8 - GPIO22)                                                  │
│    ────────────────────────────────────────────────────────────────────────   │
│                                                                                 │
│    GPIO22 ◄───────────────────────────────────────── J15 Pin 8 (SPARE)        │
│                                                                                 │
│    Reserved for future expansion. No pull-up/down by default.                 │
│    Suggested uses: Additional sensor input, flow sensor, etc.                 │
│                                                                                 │
│    ════════════════════════════════════════════════════════════════════════   │
│                                                                                 │
│    Power Notes:                                                                │
│    ────────────                                                                │
│    ESP32 + display draws 300-500mA. Power from 5V (Pin 1).                    │
│    ESP32 modules have onboard 3.3V LDO. Do not power from Pico's 3.3V.       │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 8.2 Service/Debug Port (UART0 - Shared with ESP32)

4-pin header for development, debugging, and emergency firmware flashing.
**Shares GPIO0/1 with ESP32 display connector (J15).**

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                           SERVICE/DEBUG PORT                                    │
│                (J16 - 2.54mm 4-Pin Header, SHARED with ESP32)                  │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    ⚠️  IMPORTANT: J16 shares GPIO0/1 with J15 (ESP32 Display)                 │
│    ─────────────────────────────────────────────────────────                   │
│    • DISCONNECT ESP32 CABLE (J15) before using service port                   │
│    • Both connectors cannot be used simultaneously                            │
│                                                                                 │
│    Pin Assignment:                                                             │
│    ───────────────                                                             │
│    ┌─────┬─────┬─────┬─────┐                                                  │
│    │  1  │  2  │  3  │  4  │                                                  │
│    │ 3V3 │ GND │ TX  │ RX  │                                                  │
│    └──┬──┴──┬──┴──┬──┴──┬──┘                                                  │
│       │     │     │     │                                                      │
│       │     │     │     └──────────────► GPIO1 (UART0 RX ← External TX)       │
│       │     │     └────────────────────► GPIO0 (UART0 TX → External RX)       │
│       │     └──────────────────────────► Ground                               │
│       └────────────────────────────────► 3.3V (reference only, <100mA)        │
│                                                                                 │
│    Shared UART0 Wiring:                                                        │
│    ────────────────────                                                        │
│                          ┌───────────────► J15 Pin (ESP32 RX)                 │
│    GPIO0 (TX) ──[33Ω]───┤                                                     │
│                          └───────────────► J16 Pin 3 (Service TX)             │
│                                                                                 │
│                          ┌───────────────► J15 Pin (ESP32 TX)                 │
│    GPIO1 (RX) ◄─[33Ω]───┤                                                     │
│                          └───────────────► J16 Pin 4 (Service RX)             │
│                                                                                 │
│    Use Cases:                                                                  │
│    ───────────                                                                 │
│    1. Emergency firmware flashing (UART bootloader on GPIO0/1)                │
│    2. Debug console when ESP32 disconnected                                   │
│    3. Production testing and calibration                                      │
│    4. Recovery if USB port is damaged                                         │
│                                                                                 │
│    ESP32 OTA Firmware Updates:                                                 │
│    ───────────────────────────                                                 │
│    The ESP32 display can update Pico firmware over-the-air:                   │
│    1. ESP32 downloads new firmware via WiFi                                   │
│    2. ESP32 puts Pico into bootloader mode (via RUN pin or BOOTSEL)          │
│    3. ESP32 sends firmware to Pico via UART0 (GPIO0/1)                        │
│    4. Pico reboots with new firmware                                          │
│    This enables remote updates without physical access!                        │
│                                                                                 │
│    Default Configuration: 921600 baud, 8N1 (normal communication)            │
│    USB Serial Debug: 115200 baud, 8N1                                        │
│                                                                                 │
│    Silkscreen Labels:                                                          │
│    ──────────────────                                                          │
│    J16: "SERVICE" with pin labels "3V3 GND TX RX"                             │
│    Warning: "⚠️ DISCONNECT DISPLAY BEFORE USE"                                │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 8.3 I2C Accessory Port

4-pin header for I2C accessories (sensors, displays, expansion modules).

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                           I2C ACCESSORY PORT                                    │
│                         (J23 - 2.54mm 4-Pin Header)                            │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    Pin Assignment:                                                             │
│    ───────────────                                                             │
│    ┌─────┬─────┬─────┬─────┐                                                  │
│    │  1  │  2  │  3  │  4  │                                                  │
│    │ 3V3 │ GND │ SDA │ SCL │                                                  │
│    └──┬──┴──┬──┴──┬──┴──┬──┘                                                  │
│       │     │     │     │                                                      │
│       │     │     │     └──────────────► GPIO9 (I2C0 SCL)                     │
│       │     │     └────────────────────► GPIO8 (I2C0 SDA)                     │
│       │     └──────────────────────────► Ground                               │
│       └────────────────────────────────► 3.3V (max 100mA for accessories)     │
│                                                                                 │
│    Circuit:                                                                    │
│    ─────────                                                                   │
│                     3.3V                                                       │
│                      │                                                          │
│               ┌──────┼──────┐                                                  │
│               │      │      │                                                  │
│            [4.7kΩ] [4.7kΩ]  │                                                  │
│               │      │      │                                                  │
│    GPIO8 ─────┴──────│──────┼───────────► J23 Pin 3 (SDA)                     │
│    GPIO9 ────────────┴──────┼───────────► J23 Pin 4 (SCL)                     │
│                             │                                                  │
│                            GND                                                 │
│                                                                                 │
│    Compatible Accessories:                                                     │
│    ───────────────────────                                                     │
│    • OLED displays (SSD1306, SH1106)                                          │
│    • Additional temperature sensors (TMP117, MCP9808)                         │
│    • Pressure sensors (BMP280, BME280)                                        │
│    • Real-time clock (DS3231)                                                 │
│    • EEPROM for settings backup (24LC256)                                     │
│    • I2C GPIO expanders (PCF8574, MCP23017)                                   │
│                                                                                 │
│    I2C Configuration:                                                          │
│    ──────────────────                                                          │
│    • Bus: I2C0                                                                 │
│    • Default speed: 100kHz (standard mode)                                    │
│    • Max speed: 400kHz (fast mode)                                            │
│    • Pull-ups: 4.7kΩ on-board                                                 │
│                                                                                 │
│    Silkscreen Label: "I2C" with pin labels "3V3 GND SDA SCL"                  │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

---

# 9. User Interface Components

## 9.1 Reset and Boot Buttons

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                        SMD TACTILE BUTTONS                                      │
│                      (Reset and Bootloader Entry)                              │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    RESET BUTTON (SW1)                                                          │
│    ──────────────────                                                          │
│                                                                                 │
│                          3.3V                                                  │
│                           │                                                     │
│                      ┌────┴────┐                                               │
│                      │  10kΩ   │  ← Pull-up (Pico has internal on RUN)        │
│                      └────┬────┘                                               │
│                           │                                                     │
│                           ├────────────────────────► Pico RUN Pin             │
│                           │                                                     │
│                      ┌────┴────┐                                               │
│                      │   SW1   │  ← SMD Tactile 6x6mm                         │
│                      │ (RESET) │    Panasonic EVQP7A01P or similar            │
│                      └────┬────┘                                               │
│                           │                                                     │
│                          GND                                                   │
│                                                                                 │
│    Operation: Press to reset Pico. Release to run.                            │
│                                                                                 │
│    ─────────────────────────────────────────────────────────────────────────  │
│                                                                                 │
│    BOOT BUTTON (SW2)                                                           │
│    ─────────────────                                                           │
│                                                                                 │
│    Connect in parallel with Pico's onboard BOOTSEL button.                    │
│    Access via TP6 test pad on Pico module, or solder to BOOTSEL net.         │
│                                                                                 │
│                      ┌─────────┐                                               │
│                      │   SW2   │  ← SMD Tactile 6x6mm                         │
│                      │ (BOOT)  │                                               │
│                      └────┬────┘                                               │
│                           │                                                     │
│            BOOTSEL ───────┴────────────────────────► GND                      │
│            (From Pico TP6)                                                     │
│                                                                                 │
│    Operation:                                                                  │
│    1. Hold BOOT button                                                        │
│    2. Press and release RESET button                                          │
│    3. Release BOOT button                                                     │
│    4. Pico appears as USB mass storage device                                 │
│    5. Drag-drop UF2 firmware file to flash                                    │
│                                                                                 │
│    Button: Panasonic EVQP7A01P (6x6x4.3mm, 2.55N force)                       │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 9.2 Status LED

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                            STATUS LED (Green)                                   │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    GPIO15 ────────[330Ω]────────┬────────────────────────────────────────────│
│                                 │                                              │
│                            ┌────┴────┐                                         │
│                            │   LED   │  ← Green 0805 SMD                      │
│                            │  D1     │    Vf=2.0V, If=10mA                    │
│                            └────┬────┘                                         │
│                                 │                                              │
│                                GND                                             │
│                                                                                 │
│    Current: (3.3V - 2.0V) / 330Ω ≈ 3.9mA (clearly visible)                   │
│                                                                                 │
│    NOTE: Green LED chosen over blue because:                                  │
│    - Blue LEDs have Vf=3.0-3.4V, leaving only 0-0.3V margin with 3.3V supply │
│    - Green LEDs have Vf=1.8-2.2V, providing reliable ~4mA current            │
│    - Green also matches relay indicator LEDs for consistency                  │
│                                                                                 │
│    Firmware States (Example):                                                  │
│    ──────────────────────────                                                  │
│    • Solid ON: System ready, at temperature                                   │
│    • Slow blink (1Hz): Heating up                                             │
│    • Fast blink (4Hz): Error condition                                        │
│    • Double blink: Communication active                                        │
│    • Off: Standby/sleep mode                                                  │
│                                                                                 │
│    Placement: Near board edge, visible when installed                         │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 9.3 Buzzer

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                          BUZZER (Passive Piezo)                                 │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    Using PASSIVE buzzer for variable tones via PWM:                           │
│                                                                                 │
│                       ┌───────────────┐                                        │
│    GPIO19 ───[100Ω]───┤    Passive    │                                        │
│      (PWM)            │    Buzzer     │  ← CUI CEM-1203(42) or similar        │
│                       │    (BZ1)      │    12mm, 3-5V, 85dB @ 10cm            │
│                       └───────┬───────┘                                        │
│                               │                                                │
│                              GND                                               │
│                                                                                 │
│    PWM Frequencies for Alerts:                                                 │
│    ───────────────────────────                                                 │
│    • 2000 Hz: Ready beep (short)                                              │
│    • 2500 Hz: Temperature reached                                              │
│    • 1500 Hz: Shot complete                                                    │
│    • 1000 Hz: Warning (low water)                                             │
│    • 500 Hz: Error                                                             │
│    • Melody: Startup jingle                                                   │
│                                                                                 │
│    Buzzer: CUI CEM-1203(42), 12mm passive piezo, 3-5V, 85dB @ 10cm           │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

---

# 10. Power Metering Circuit

## 10.1 Universal External Power Meter Interface

**⚠️ KEY DESIGN: EXTERNAL MODULES - NO HIGH VOLTAGE THROUGH CONTROL PCB!**

The control PCB provides a universal interface for connecting external power metering modules. The external module handles its own high-voltage connections (mains L/N and CT clamp), while the control PCB provides only low-voltage communication and power.

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                    UNIVERSAL POWER METER INTERFACE                              │
│                    (External Module - OFF-BOARD Design)                         │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    DESIGN PHILOSOPHY:                                                          │
│    ──────────────────                                                          │
│    • HIGH VOLTAGE stays entirely on external metering module                   │
│    • Control PCB provides ONLY low-voltage communication interface             │
│    • Supports TTL UART (direct) or RS485 (differential) via single connector  │
│    • User wires mains (L/N) directly to external module's screw terminals     │
│    • CT clamp connects directly to external module (not routed through PCB)   │
│                                                                                 │
│    ═══════════════════════════════════════════════════════════════════════    │
│                                                                                 │
│    WIRING TOPOLOGY:                                                            │
│    ────────────────                                                            │
│                                                                                 │
│    ┌──────────────────────┐          ┌──────────────────────────────────────┐ │
│    │    CONTROL PCB       │          │    EXTERNAL POWER METER              │ │
│    │                      │          │    (PZEM, JSY, Eastron, etc.)        │ │
│    │   ┌───────────┐      │          │                                      │ │
│    │   │  J17      │      │          │   ┌─────────────┐  ┌─────────────┐   │ │
│    │   │  JST-XH   │      │  4-wire  │   │   LV UART   │  │   HV MAINS  │   │ │
│    │   │  6-pin    │──────┼──────────┼───│   (JST)     │  │   (Screw)   │   │ │
│    │   │           │      │          │   │ 5V,GND,TX,RX│  │   L, N, CT  │   │ │
│    │   └───────────┘      │          │   └─────────────┘  └──────┬──────┘   │ │
│    │                      │          │                           │          │ │
│    │   GPIO6 → TX ────────┼──────────┼───► RX                    │          │ │
│    │   GPIO7 ← RX ────────┼──────────┼───◄ TX                    │          │ │
│    │   GPIO20 → DE/RE ────┼──────────┼───► (RS485 only)          │          │ │
│    │                      │          │                    ┌──────┴──────┐   │ │
│    └──────────────────────┘          │                    │    CT       │   │ │
│                                      │                    │   Clamp     │   │ │
│                                      │                    │    ~~~      │   │ │
│    MACHINE MAINS ────────────────────┼──────────────────► │   (Live)    │   │ │
│       L ─────────────────────────────┼───► L Terminal     └─────────────┘   │ │
│       N ─────────────────────────────┼───► N Terminal                       │ │
│                                      └──────────────────────────────────────┘ │
│                                                                                 │
│    ⚠️ NO 220V TRACES ON CONTROL PCB FOR POWER METERING                        │
│    ⚠️ USER WIRES MAINS DIRECTLY TO EXTERNAL MODULE                            │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 10.2 Supported Power Metering Modules

| Module           | Default Baud | Protocol   | Physical Layer | Notes                    |
| ---------------- | ------------ | ---------- | -------------- | ------------------------ |
| **PZEM-004T V3** | 9600         | Modbus RTU | TTL (Logic)    | Most common, 100A CT     |
| **JSY-MK-163T**  | 4800         | Modbus RTU | TTL (Logic)    | Bi-directional, compact  |
| **JSY-MK-194T**  | 4800         | Modbus RTU | TTL (Logic)    | Dual-channel metering    |
| **Eastron SDM**  | 2400 or 9600 | Modbus RTU | RS485          | DIN-rail, industrial     |
| **ATM90E32**     | -            | SPI        | SPI            | High precision (via I2C) |
| **BL0937**       | -            | Pulse/CF   | GPIO           | Budget option            |

## 10.3 J17 Universal Power Meter Connector (JST-XH 6-pin)

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                    J17 - POWER METER INTERFACE CONNECTOR                        │
│                          JST-XH 6-pin (2.54mm pitch)                           │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    PINOUT:                                                                     │
│    ┌─────┬─────┬─────┬─────┬─────┬─────┐                                      │
│    │  1  │  2  │  3  │  4  │  5  │  6  │                                      │
│    │ 3V3 │ 5V  │ GND │ RX  │ TX  │DE/RE│                                      │
│    └─────┴─────┴─────┴─────┴─────┴─────┘                                      │
│                                                                                 │
│    PIN DEFINITIONS:                                                            │
│    │ Pin │ Label │ Direction │ Signal    │ Notes                            │ │
│    │─────│───────│───────────│───────────│──────────────────────────────────│ │
│    │  1  │ 3V3   │ Power Out │ +3.3V     │ For 3.3V logic modules           │ │
│    │  2  │ 5V    │ Power Out │ +5V       │ For 5V modules (PZEM, JSY)       │ │
│    │  3  │ GND   │ Ground    │ GND       │ System ground                    │ │
│    │  4  │ RX    │ Input     │ GPIO7     │ Data from meter (via 33Ω)        │ │
│    │  5  │ TX    │ Output    │ GPIO6     │ Data to meter (via 33Ω)          │ │
│    │  6  │ DE/RE │ Output    │ GPIO20    │ RS485 direction (TTL: NC)        │ │
│                                                                                 │
│    TTL UART MODE (PZEM, JSY):                                                  │
│    ──────────────────────────                                                  │
│    • Connect Pin 2 (5V), Pin 3 (GND), Pin 4 (RX), Pin 5 (TX)                  │
│    • Leave Pin 6 (DE/RE) unconnected                                          │
│    • Direct connection to meter's UART pins                                   │
│                                                                                 │
│    RS485 MODE (Eastron, Industrial):                                          │
│    ──────────────────────────────────                                          │
│    • Uses on-board MAX3485 transceiver                                        │
│    • Pin 4/5 become RS485 A/B differential pair                               │
│    • Pin 6 controls transceiver direction                                     │
│    • JP1 jumper enables 120Ω termination resistor                             │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

| Pin | Label | Direction | TTL Mode       | RS485 Mode            |
| --- | ----- | --------- | -------------- | --------------------- |
| 1   | 3V3   | Power Out | 3.3V for logic | 3.3V for logic        |
| 2   | 5V    | Power Out | 5V for module  | 5V for module         |
| 3   | GND   | Ground    | System ground  | System ground         |
| 4   | RX    | Input     | Meter TX→GPIO7 | RS485 A (via MAX3485) |
| 5   | TX    | Output    | GPIO6→Meter RX | RS485 B (via MAX3485) |
| 6   | DE/RE | Output    | Not connected  | GPIO20→MAX3485 DE/RE  |

## 10.4 RS485 Transceiver Circuit (Optional - For Industrial Meters)

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                    RS485 TRANSCEIVER CIRCUIT (MAX3485)                          │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│                                      +3.3V                                     │
│                                        │                                       │
│                                   ┌────┴────┐                                  │
│                                   │  100nF  │  C70 (Decoupling)                │
│                                   └────┬────┘                                  │
│                                        │                                       │
│                                   VCC──┤                                       │
│                                        │                                       │
│                                        │  U8: MAX3485                          │
│                              ┌─────────┴─────────┐                             │
│    GPIO6 (TX) ──────────────►│ DI            A   │◄────────────► J17 Pin 4     │
│                              │                   │               (RS485-A)     │
│    GPIO7 (RX) ◄──────────────│ RO            B   │◄────────────► J17 Pin 5     │
│                              │                   │               (RS485-B)     │
│    GPIO20 (DE/RE) ──────────►│ DE/RE         GND │                             │
│                              │                   │                             │
│                              └─────────┬─────────┘                             │
│                                        │                                       │
│                                       ─┴─                                      │
│                                       GND                                      │
│                                                                                 │
│    TERMINATION RESISTOR (Jumper Selectable):                                  │
│    ─────────────────────────────────────────                                   │
│                                                                                 │
│    J17 Pin 4 (A) ────┬────[120Ω R99]────┬──── J17 Pin 5 (B)                   │
│                      │                  │                                      │
│                      └───── JP1 ────────┘                                      │
│                         (Solder jumper)                                        │
│                                                                                 │
│    • JP1 CLOSED: 120Ω termination enabled (for long cable runs)               │
│    • JP1 OPEN: No termination (for short cables < 10m)                        │
│                                                                                 │
│    Component Values:                                                           │
│    ─────────────────                                                           │
│    U8:  MAX3485 or SP3485 (3.3V, SOT-23-8 or SOIC-8)                          │
│    C70: 100nF 25V Ceramic, 0805                                               │
│    R99: 120Ω 1%, 0805 (termination, via JP1)                                  │
│                                                                                 │
│    OPERATION:                                                                  │
│    ──────────                                                                  │
│    DE/RE = HIGH: Transmit mode (DI → A/B)                                     │
│    DE/RE = LOW:  Receive mode (A/B → RO)                                      │
│                                                                                 │
│    Firmware handles DE/RE timing automatically during Modbus transactions.    │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 10.5 Firmware Notes

**Communication Protocol:**

- UART1 @ configurable baud (9600, 4800, 2400, or 19200)
- Modbus-RTU protocol for all supported meters
- GPIO20 controls RS485 DE/RE direction (HIGH=TX, LOW=RX)
- Firmware auto-detects meter type and baud rate on startup

**For detailed firmware integration, see:** `docs/pico/Power_Metering.md`

---

# 11. Safety & Protection

## 11.1 Mains Input Protection

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                        MAINS INPUT PROTECTION                                   │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    COMPLETE MAINS INPUT CIRCUIT:                                              │
│    ────────────────────────────                                                │
│                                                                                 │
│    L (Live)  ─────┬─────────────────────────────────────────────────────────  │
│                   │                                                            │
│              ┌────┴────┐                                                       │
│              │  F1     │  ← Fuse: 10A slow-blow, 5x20mm glass                 │
│              │  10A    │    (relay-switched loads only - pump, valves)        │
│              │ 250V    │                                                       │
│              └────┬────┘                                                       │
│                   │                                                            │
│              ┌────┴────┐     ┌─────────┐                                      │
│              │  RV1    │     │   C1    │                                      │
│              │  MOV    │     │   X2    │  ← EMI suppression capacitor         │
│              │  275V   ├─────┤  100nF  │    X2 safety rated, 275V AC          │
│              │  14mm   │     │  275V   │                                      │
│              └────┬────┘     └────┬────┘                                      │
│                   │               │                                            │
│    N (Neutral) ───┴───────────────┴─────────────────────────────────────────  │
│                                                                                │
│    Optional EMI Filter (for CE compliance):                                   │
│    ────────────────────────────────────────                                   │
│                                                                                │
│         L ──┬──[CMC]──┬─── L (filtered)                                       │
│             │    ║    │                                                        │
│         N ──┴────╫────┴─── N (filtered)                                       │
│                  ║                                                             │
│              Common-mode choke                                                 │
│              (e.g., Würth 744272102, 10mH)                                    │
│                                                                                │
│    PE (Earth) ────────────────────────────────────────────────────────────    │
│                   │                                                            │
│                   └──► To chassis ground point (if metal enclosure)           │
│                        PE connects to signal GND at single point (see 12.5)   │
│                                                                                │
│    Component Specifications:                                                   │
│    ─────────────────────────                                                   │
│    F1:  Fuse, 10A/250V, 5x20mm, slow-blow (time-lag)                         │
│         Fuse: Littelfuse 0218010.MXP or equivalent                            │
│         Holder: Littelfuse 01000056Z (PCB mount clips with cover)            │
│                 Alt: Schurter 0031.8201 (enclosed PCB mount)                  │
│                                                                                │
│    ⚠️  FUSE HOLDER LAYOUT NOTES:                                              │
│    • Holder is ~27mm long - verify clearance to HLK-5M05 and AC terminals    │
│    • Orient PARALLEL to board edge for easy fuse access when installed       │
│    • Minimum 4mm clearance from high-voltage components                       │
│    • Verify physical fit in CAD before committing layout                      │
│                                                                                │
│    ✅ FUSE SIZING SIMPLIFIED (PZEM-004T Design):                              │
│    ─────────────────────────────────────────────                              │
│    With external PZEM-004T, heater current does NOT flow through the PCB.   │
│    The on-board fuse only protects relay-switched loads:                     │
│    • K2 (Pump): ~5A peak (Ulka EP5)                                          │
│    • K1 (LED): ≤100mA                                                        │
│    • K3 (Solenoid): ~0.5A                                                    │
│    Total relay-switched: ~6A maximum                                          │
│                                                                                │
│    10A fuse provides adequate margin for relay-switched loads.               │
│    Standard 5×20mm fuse holders (rated 10A) are now acceptable.              │
│                                                                                │
│    RV1: MOV/Varistor, 275V AC, 14mm disc, 4500A surge                        │
│         Part: Littelfuse V275LA20AP or Epcos S14K275                          │
│         Provides surge protection for lightning/transients                    │
│                                                                                │
│    C1:  X2 capacitor, 100nF, 275V AC, safety rated                           │
│         Part: Vishay MKP X2 or TDK B32922C series                            │
│         Provides EMI filtering for common-mode noise                          │
│                                                                                │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 11.2 Creepage and Clearance Requirements

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                    CREEPAGE AND CLEARANCE REQUIREMENTS                          │
│                        (Per IEC 60950-1 / IEC 62368-1)                         │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    DEFINITIONS:                                                                │
│    ─────────────                                                               │
│    Creepage: Shortest path along a surface between conductors                 │
│    Clearance: Shortest path through air between conductors                    │
│                                                                                 │
│    For 240V RMS mains (peak 340V), pollution degree 2:                        │
│                                                                                 │
│    │ Insulation Type     │ Creepage  │ Clearance │ Application              │ │
│    │─────────────────────│───────────│───────────│──────────────────────────│ │
│    │ Basic Insulation    │ 2.5mm     │ 2.0mm     │ Within mains section     │ │
│    │ Reinforced (double) │ 6.0mm     │ 4.0mm     │ Mains to low-voltage     │ │
│    │                     │           │           │ (required for safety)    │ │
│                                                                                 │
│    DESIGN REQUIREMENTS:                                                        │
│    ────────────────────                                                        │
│                                                                                 │
│    1. AC Input to 5V DC Rail: ≥6mm creepage, ≥4mm clearance                  │
│       (achieved by HLK-PM05 internal isolation + PCB layout)                  │
│                                                                                 │
│    2. Relay Contacts to Coil: ≥6mm creepage, ≥4mm clearance                  │
│       (check relay datasheet - most meet this internally)                     │
│       PCB traces must also maintain this separation                           │
│                                                                                 │
│    3. Power Meter Mains to Pico: ≥6mm creepage, ≥4mm clearance              │
│       Use PCB slots if necessary                                              │
│                                                                                 │
│    4. Between Live and Neutral: ≥2.5mm creepage/clearance                    │
│                                                                                 │
│    PCB SLOT USAGE:                                                            │
│    ────────────────                                                           │
│                                                                                │
│            MAINS SIDE          │          LOW VOLTAGE SIDE                    │
│    ┌──────────────────────┐    │    ┌──────────────────────┐                 │
│    │                      │    │    │                      │                 │
│    │   Relay Contacts     │    │    │      Pico            │                 │
│    │   AC/DC Module       │    │    │      Sensors         │                 │
│    │   Power Meter        │ ═══╪═══ │      Logic           │                 │
│    │                      │    │    │                      │                 │
│    └──────────────────────┘   SLOT  └──────────────────────┘                 │
│                                │                                              │
│                           ≥2mm wide routed slot in PCB                        │
│                           Extends creepage path                               │
│                                                                                │
│    Slot dimensions: 2-3mm wide, bridged only by approved components          │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 11.3 ESD and Transient Protection

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                      ESD AND TRANSIENT PROTECTION                               │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    External-facing signals require ESD protection:                            │
│                                                                                 │
│    SENSOR INPUTS (GPIO2-5, ADC0-2):                                           │
│    ─────────────────────────────────                                          │
│                                                                                 │
│    External         ESD Clamp                  To Pico GPIO                   │
│    Signal ────┬────[PESD5V0S1BL]────┬─────────────────►                       │
│               │                     │                                          │
│               │                     │                                          │
│              GND                   GND                                         │
│                                                                                 │
│    ESD Clamp Options:                                                         │
│    • PESD5V0S1BL (SOD-323): Single-line, 5V, <1pF                            │
│    • TPD1E10B06 (SOD-882): 10kV HBM rating                                   │
│    • PRTR5V0U2X (SOT-143): Dual-line                                         │
│                                                                                 │
│    Place ESD clamps as close to connector as possible.                        │
│                                                                                 │
│    UART LINES (GPIO0-1, GPIO8-9):                                             │
│    ───────────────────────────────                                            │
│    33Ω series resistor provides some protection.                              │
│    Add ESD clamp if UART connectors are externally accessible.               │
│                                                                                 │
│    5V RAIL TRANSIENT SUPPRESSION:                                             │
│    ───────────────────────────────                                            │
│                                                                                 │
│    From HLK-PM05 ────┬────[Ferrite Bead]────┬──► 5V to circuits              │
│                      │                       │                                 │
│                 ┌────┴────┐            ┌────┴────┐                            │
│                 │  TVS    │            │  100µF  │                            │
│                 │  5.6V   │            │  16V    │                            │
│                 │ (SMBJ5.0A)           │         │                            │
│                 └────┬────┘            └────┬────┘                            │
│                      │                      │                                  │
│                     GND                    GND                                 │
│                                                                                 │
│    TVS diode absorbs relay coil flyback spikes that pass the 1N4007.         │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 11.4 Thermal Management

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                          THERMAL MANAGEMENT                                     │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    HEAT SOURCES ON PCB:                                                        │
│    ────────────────────                                                        │
│                                                                                 │
│    │ Component          │ Power Dissipation │ Management                    │ │
│    │────────────────────│───────────────────│───────────────────────────────│ │
│    │ HLK-PM05 (AC/DC)   │ 0.5-1.0W @ 1A     │ Place away from heat-sensitive│ │
│    │ LDO Regulator      │ 0.3W @ 200mA      │ Use AP2112K for lower drop    │ │
│    │ Relay Coils (×4)   │ 0.4W each @ 80mA  │ Normal, space apart           │ │
│    │ Shunt Resistor     │ 0.26W @ 16A cont. │ 5W rating provides margin     │ │
│    │ Transistor drivers │ <0.1W each        │ Negligible                    │ │
│                                                                                 │
│    GUIDELINES:                                                                 │
│    ───────────                                                                 │
│    1. HLK-PM05: Leave 5mm clearance on all sides for airflow                 │
│    2. Place electrolytic capacitors away from heat sources (reduces aging)   │
│    3. LDO: Use thermal vias under SOT-223 tab (5× 0.3mm vias to GND plane)  │
│    4. Relays: Space 10mm apart if possible                                    │
│    5. Copper pours: Use exposed copper for heat spreading where possible     │
│                                                                                 │
│    VENTILATION:                                                               │
│    ────────────                                                               │
│    If enclosed, ensure enclosure has ventilation holes near HLK-PM05         │
│    and relay section. Natural convection is usually sufficient.              │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

---

# 12. PCB Design Requirements

## 12.1 Board Specifications

| Parameter               | Specification                                               |
| ----------------------- | ----------------------------------------------------------- |
| **Dimensions**          | **130mm × 80mm**                                            |
| Enclosure Mounting Area | 150mm × 100mm (leaves room for terminals + enclosure walls) |
| **Layers**              | **2-layer**                                                 |
| Copper Weight           | 2oz (70µm) both layers for high-current traces              |
| Board Thickness         | 1.6mm                                                       |
| Material                | FR-4, Tg 130°C minimum, UL 94V-0 flammability rating        |

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                         ENCLOSURE MOUNTING LAYOUT                               │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    Enclosure Mounting Area: 150mm × 100mm                                      │
│    ┌─────────────────────────────────────────────────────────────────────────┐ │
│    │                                                                         │ │
│    │    ┌─────────────────────────────────────────────────────────────────┐ │ │
│    │    │                                                                 │ │ │
│    │    │                      PCB: 130mm × 80mm                         │ │ │
│    │    │                                                                 │ │ │
│    │    │   ┌───┐                                              ┌───┐     │ │ │
│    │    │   │M3 │  ← Mounting holes (4×)                       │M3 │     │ │ │
│    │    │   └───┘                                              └───┘     │ │ │
│    │    │                                                                 │ │ │
│    │    │                                                                 │ │ │
│    │    │   ┌───┐                                              ┌───┐     │ │ │
│    │    │   │M3 │                                              │M3 │     │ │ │
│    │    │   └───┘                                              └───┘     │ │ │
│    │    │                                                                 │ │ │
│    │    └───────────────────────────┬─────────────────────────────────────┘ │ │
│    │                                │                                       │ │
│    │    ←─── 10mm ───→              ▼                     ←─── 10mm ───→    │ │
│    │                        Terminal clearance                              │ │
│    │                           (~10mm)                                      │ │
│    └─────────────────────────────────────────────────────────────────────────┘ │
│                                                                                 │
│    Clearances:                                                                 │
│    • 10mm on each side for enclosure walls                                    │
│    • 10mm below PCB for terminal/wire clearance                               │
│    • Mounting holes: 3.2mm diameter for M3 screws                             │
│    • Hole placement: 5mm from board edges                                     │
│                                                                                 │
│    ⚠️  MECHANICAL FIT CONSIDERATIONS (ECM Synchronika Specifics):             │
│    ─────────────────────────────────────────────────────────────              │
│    • ECM Synchronika: GICAR box is low in chassis, near bottom plate         │
│    • Space is tight but relatively cool (away from boilers)                   │
│    • CRITICAL: Ensure Raspberry Pi Pico USB port is ACCESSIBLE               │
│                                                                                │
│    Pico Orientation Recommendations:                                          │
│    ─────────────────────────────────                                          │
│    • Orient Pico so USB edge faces "Service" side of machine                 │
│    • If board mounted flat, vertical USB port may be blocked by cables       │
│    • Consider right-angle USB adapter or cable for space-constrained installs│
│    • Add access cutout in enclosure aligned with Pico USB port               │
│    • Alternative: Use USB breakout cable to enclosure-mounted USB jack       │
│                                                                                │
│    Thermal Considerations:                                                    │
│    ──────────────────────                                                     │
│    • Keep board away from boilers and steam paths                            │
│    • Max operating temp: 50°C ambient (derate HLK-PM05 above this)           │
│    • Add ventilation holes in enclosure if mounting near warm components     │
│                                                                                │
└────────────────────────────────────────────────────────────────────────────────┘
```

| Surface Finish | ENIG preferred (HASL acceptable) |
| Solder Mask | Both sides, green or black |
| Silkscreen | Both sides, white |
| Minimum Trace/Space | 0.2mm / 0.2mm (8 mil) |
| Minimum Via | 0.3mm drill, 0.6mm pad |

## 12.2 Trace Width Requirements

| Signal/Power        | Current | Width (2oz Cu)     | Notes                     |
| ------------------- | ------- | ------------------ | ------------------------- |
| Mains Live/Neutral  | 6A peak | 1.5mm (60 mil)     | Relay-switched loads only |
| Relay K2 (Pump)     | 5A peak | 1.5mm (60 mil)     | Ulka pump                 |
| Relay K1 (LED)      | 100mA   | 0.5mm (20 mil)     | Water status LED          |
| Relay K3 (Solenoid) | 0.5A    | 1.0mm (40 mil)     | 3-way solenoid valve      |
| 5V power rail       | 1A      | 1.0mm (40 mil)     | Main distribution         |
| 5V to Pico VSYS     | 500mA   | 0.5mm (20 mil)     |                           |
| 3.3V power rail     | 500mA   | 0.5mm (20 mil)     |                           |
| Relay coil drive    | 80mA    | 0.3mm (12 mil)     |                           |
| Signal traces       | <10mA   | 0.25mm (10 mil)    | GPIO, UART, SPI           |
| Ground returns      | -       | Match signal width | Use ground plane          |

### ✅ SIMPLIFIED PCB DESIGN (PZEM-004T External Metering)

**With PZEM-004T external power metering, heater current (12A+) does NOT flow through the control PCB.**

The PCB only handles relay-switched loads:

- **K2 (Pump):** 5A peak (Ulka EP5)
- **K1 (LED):** ≤100mA
- **K3 (Solenoid):** ~0.5A
- **Total maximum:** ~6A

**Benefits of PZEM-004T Design:**

- ✅ No 16A shunt resistor required
- ✅ No solder mask openings on high-current traces needed
- ✅ Standard thermal relief is acceptable
- ✅ Standard terminal blocks are sufficient
- ✅ 10A fuse with standard holder
- ✅ Simpler PCB layout
- ✅ No fire risk from high-current paths

### Standard PCB Practices Apply

For the relay-switched loads (max ~6A):

1. Use 1.5mm trace width for pump relay (K2) traces
2. Standard solder mask is fine
3. Thermal relief can be used on all pads
4. Standard 6.3mm spade terminals rated for 10A are acceptable

## 12.3 Layer Stackup (2-Layer)

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                   2-LAYER PCB STACKUP (130mm × 80mm)                            │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    ┌────────────────────────────────────────────────────────────────────┐     │
│    │  TOP COPPER (2oz / 70µm) - Component Side                          │     │
│    │  ─────────────────────────────────────────────                     │     │
│    │  • SMD components (Pico, ICs, passives)                            │     │
│    │  • High-current mains traces (3mm+ width)                          │     │
│    │  • Relay and connector pads                                        │     │
│    │  • Signal routing (LV section)                                     │     │
│    │  • Isolation slot along HV/LV boundary                             │     │
│    ├────────────────────────────────────────────────────────────────────┤     │
│    │  FR-4 CORE (1.4mm)                                                 │     │
│    │  - Tg ≥ 130°C                                                      │     │
│    │  - UL 94V-0 rated                                                  │     │
│    ├────────────────────────────────────────────────────────────────────┤     │
│    │  BOTTOM COPPER (2oz / 70µm) - Solder Side                          │     │
│    │  ─────────────────────────────────────────────                     │     │
│    │  • Ground plane (maximize coverage in LV section)                  │     │
│    │  • Signal routing for crossovers (keep short)                      │     │
│    │  • Through-hole component leads                                    │     │
│    │  • Star ground connection point near Pico                          │     │
│    │  • NO high-voltage traces on bottom layer                          │     │
│    └────────────────────────────────────────────────────────────────────┘     │
│                                                                                 │
│    Total Thickness: 1.6mm (standard)                                          │
│                                                                                 │
│    2-Layer Routing Strategy:                                                   │
│    ─────────────────────────                                                   │
│    • Top: Components, all HV traces, horizontal LV signal routing             │
│    • Bottom: Ground plane (priority), vertical LV routing, power polygons     │
│    • Keep ground plane continuous under Pico and analog section               │
│    • Use thermal relief on high-current ground connections                    │
│    • Route all mains traces ONLY on top layer                                 │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 12.4 Critical Layout Notes

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                         ⚠️ CRITICAL LAYOUT NOTES                               │
│                    (Review these BEFORE starting layout)                       │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    1. PZEM UART TRACES (GPIO6/7) - PIO Driven                                  │
│    ──────────────────────────────────────────────                              │
│    GPIO6/7 (PZEM-004T UART) will be driven by RP2040 PIO (Programmable I/O),  │
│    not hardware UART. This means:                                              │
│                                                                                 │
│    ✅ NO differential pair routing required                                    │
│    ✅ NO matched trace lengths required                                        │
│    ✅ Just keep traces SHORT (< 5cm recommended)                               │
│    ✅ Include 33Ω series resistors (R44, R45) for protection                   │
│                                                                                 │
│    PIO handles timing in software, so standard routing rules for              │
│    high-speed differential pairs do NOT apply here.                            │
│                                                                                 │
│    ─────────────────────────────────────────────────────────────────────────   │
│                                                                                 │
│    2. STAR GROUND AT MH1 ONLY                                                  │
│    ──────────────────────────────                                              │
│    ⚠️ CRITICAL: Only mounting hole MH1 connects to GND plane!                 │
│                                                                                 │
│    │ Hole │ Type │ Connection │ Purpose                    │                  │
│    │──────│──────│────────────│────────────────────────────│                  │
│    │ MH1  │ PTH  │ GND plane  │ Star ground / PE bond      │                  │
│    │ MH2  │ NPTH │ ISOLATED   │ Mechanical only            │                  │
│    │ MH3  │ NPTH │ ISOLATED   │ Mechanical only            │                  │
│    │ MH4  │ NPTH │ ISOLATED   │ Mechanical only            │                  │
│                                                                                 │
│    WHY: If all mounting holes connect to GND, current will flow through       │
│    the metal chassis via multiple paths, creating GROUND LOOPS.               │
│    This causes EMI, noise, and unreliable ground reference.                   │
│                                                                                 │
│    Mark MH1 with "⏚" symbol on silkscreen.                                    │
│                                                                                 │
│    ─────────────────────────────────────────────────────────────────────────   │
│                                                                                 │
│    3. LEVEL PROBE GUARD RING (High-Impedance Trace Protection)                │
│    ────────────────────────────────────────────────────────────               │
│    The trace from Level Probe (J26 Pin 5) to OPA342 input is:                 │
│    • High-impedance (MΩ range)                                                │
│    • Very sensitive to noise pickup                                           │
│    • Susceptible to 50/60Hz mains hum                                         │
│                                                                                 │
│    REQUIRED: Surround this trace with a GUARD RING                            │
│                                                                                 │
│    Layout:                                                                     │
│    ┌─────────────────────────────────────────────────────────────────────┐    │
│    │  GND ─────────────────────────────────────────────────────────── GND│    │
│    │   │                                                               │ │    │
│    │   │   J26-5 (Probe) ─────────────────────────────► OPA342 IN+    │ │    │
│    │   │                                                               │ │    │
│    │  GND ─────────────────────────────────────────────────────────── GND│    │
│    └─────────────────────────────────────────────────────────────────────┘    │
│                                                                                 │
│    Implementation:                                                             │
│    • Route GND traces on BOTH sides of the probe signal trace                 │
│    • Connect guard traces to GND plane with vias every 5mm                    │
│    • Keep probe trace as SHORT as possible (< 2cm ideal)                      │
│    • Place OPA342 physically CLOSE to J26 screw terminal                      │
│    • Avoid routing probe trace near relay coils or mains traces               │
│                                                                                 │
│    This prevents 50Hz mains hum from coupling into the AC level sensing.      │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 12.5 PCB Layout Zones

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                    PCB LAYOUT ZONES (130mm × 80mm)                              │
│                      (Board orientation: landscape)                            │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    ┌──────────────────────────────────────────────────────────────────────┐   │
│    │                                                                      │   │
│    │  ┌──────────────────────────────┐ ║ ┌──────────────────────────────┐│   │
│    │  │        HIGH VOLTAGE          │ ║ │       LOW VOLTAGE            ││   │
│    │  │                              │ ║ │                              ││   │
│    │  │  ┌────────────┐  ┌────────┐  │ ║ │  ┌─────────────────────────┐ ││   │
│    │  │  │ AC Input   │  │HLK-PM05│  │ ║ │  │    RASPBERRY PI PICO   │ ││   │
│    │  │  │ L N PE     │  │ AC/DC  │  │ ║ │  │    (Module or socket)  │ ││   │
│    │  │  │ Fuse, MOV  │  │        │  │ ║ │  │                        │ ││   │
│    │  │  └────────────┘  └────────┘  │ ║ │  └─────────────────────────┘ ││   │
│    │  │                              │ S │                              ││   │
│    │  │  ┌──────┐┌──────┐┌──────┐   │ L │  ┌─────────┐  ┌──────────┐   ││   │
│    │  │  │ K1   ││ K2   ││ K3   │   │ O │  │ LDO 3.3V│  │ MAX31855 │   ││   │
│    │  │  │Relay ││Relay ││Relay │   │ T │  │         │  │Thermo-   │   ││   │
│    │  │  └──────┘└──────┘└──────┘   │   │  └─────────┘  │couple    │   ││   │
│    │  │                              │ ║ │              └──────────┘   ││   │
│    │  │  ┌────────────────────────┐  │ ║ │  ┌─────────────────────────┐ ││   │
│    │  │  │ Level Probe Circuit    │  │ ║ │  │   Sensor Input Section │ ││   │
│    │  │  │ (OPA342 + TLV3201)     │  │ ║ │  │   NTC, Pressure, etc.  │ ││   │
│    │  │  └────────────────────────┘  │ ║ │  └─────────────────────────┘ ││   │
│    │  │                              │ ║ │                              ││   │
│    │  └──────────────────────────────┘ ║ └──────────────────────────────┘│   │
│    │                                   ║                                  │   │
│    │  ─────────────────────────────────╫──────────────────────────────────│   │
│    │   HV CONNECTORS (6.3mm spades)    ║   LV CONNECTORS (screw terms)   │   │
│    │   K1 K2 K3 SSR1 SSR2              ║   NTC PRES TC SW1 SW2 SW3 SW4  │   │
│    │                                   ║   ESP32(JST) SERVICE(HDR)        │   │
│    │                                   ║                                  │   │
│    └──────────────────────────────────────────────────────────────────────┘   │
│                                                                                 │
│    KEY:                                                                        │
│    ════  Silkscreen boundary line between HV and LV zones                     │
│    SLOT  PCB routed slot for isolation (2-3mm wide)                           │
│                                                                                 │
│    Mounting Holes: 4× M3 (3.2mm) near corners, ≥5mm from board edge           │
│                                                                                │
│    ⚠️  MOUNTING HOLE GROUNDING (Critical for ground loop prevention):         │
│    ─────────────────────────────────────────────────────────────────          │
│    MH1 (bottom-left): PTH, connected to GND plane → PE STAR POINT             │
│    MH2 (bottom-right): NPTH or isolated pad → NOT connected to GND            │
│    MH3 (top-left): NPTH or isolated pad → NOT connected to GND                │
│    MH4 (top-right): NPTH or isolated pad → NOT connected to GND               │
│                                                                                │
│    Mark MH1 with "⏚" silkscreen symbol for PE bonding screw location.        │
│    This prevents multiple ground loops through metal chassis/screws.          │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 12.6 Grounding Strategy

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                          GROUNDING STRATEGY                                     │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    ⚠️  IMPORTANT: PE and Signal GND are CONNECTED at a single point.          │
│    This matches the existing ECM machine grounding and is required for the     │
│    steam boiler level probe sensing circuit to work properly.                  │
│                                                                                 │
│    GROUND HIERARCHY:                                                           │
│    ──────────────────                                                          │
│                                                                                 │
│    PE (Protective Earth) ───► J1-PE terminal                                   │
│           │                                                                    │
│           │    ┌────────────────────────────────────────────────────┐         │
│           │    │  SINGLE POINT PE-GND CONNECTION                    │         │
│           └────┤  (at mounting hole pad or designated star point)   │         │
│                │  This provides reference for level probe sensing.  │         │
│                └────────────────────┬───────────────────────────────┘         │
│                                     │                                          │
│    ┌────────────────────────────────┴───────────────────────────────────┐     │
│    │                        SIGNAL GROUND (0V)                           │     │
│    │                    (From isolated AC/DC module secondary)           │     │
│    └──────┬───────────────────────────────────────────────────────────────┘     │
│           │                                                                    │
│           ├───────────────────────────────────────────────────────────────┐   │
│           │                                                               │   │
│    ┌──────┴──────┐                                              ┌─────────┴──┐│
│    │ DIGITAL GND │                                              │ ANALOG GND ││
│    │             │                                              │            ││
│    │ - Pico GND  │                                              │ - ADC GND  ││
│    │ - Relays    │    Star ground connection point              │ - NTC GND  ││
│    │ - LEDs      │◄──────────────────────────────────────────── │ - MAX31855 ││
│    │ - Buzzer    │    (single point near Pico ADC_GND pin)      │            ││
│    │ - UART      │                                              │            ││
│    └─────────────┘                                              └────────────┘│
│                                                                                │
│    WHY PE-GND CONNECTION IS REQUIRED:                                         │
│    ──────────────────────────────────                                         │
│    • Steam boiler level probe uses boiler body (PE) as ground reference      │
│    • Without PE-GND connection, probe circuit has no return path             │
│    • Original ECM machine has shared PE/GND - this maintains compatibility   │
│    • AC/DC module still provides isolation from mains L/N (safety)           │
│                                                                                │
│    ⚠️  USB GROUND LOOP WARNING:                                               │
│    ─────────────────────────────                                              │
│    With PE bonded to signal GND, connecting USB to a grounded PC while       │
│    mains is active creates a ground loop. If there is any voltage potential  │
│    difference between machine PE and PC ground, current can flow through     │
│    the USB shield, causing:                                                   │
│    • Erratic behavior or data corruption                                      │
│    • Potential damage to USB ports                                            │
│    • Safety hazard if isolation fails                                         │
│                                                                                │
│    MANDATORY: Add silkscreen near USB/service header:                         │
│    "⚠️ DISCONNECT MAINS BEFORE USB/UART DEBUGGING"                           │
│                                                                                │
│    For development with mains active, use a USB isolator (e.g., Adafruit     │
│    USB Isolator) to break the ground loop.                                    │
│                                                                                │
│    IMPLEMENTATION:                                                            │
│    ────────────────                                                           │
│    1. Use ground plane on bottom layer (as continuous as possible)           │
│    2. Connect PE to signal GND at MH1 ONLY (see Section 12.4 zone layout)    │
│       - MH1: PTH with GND connection (PE star point)                         │
│       - MH2, MH3, MH4: NPTH or isolated pads (NO GND connection)             │
│    3. Route analog ground traces directly to Pico ADC_GND pin               │
│    4. Add ferrite bead between digital and analog sections if noise issue    │
│    5. Use star grounding: all critical grounds meet at one point             │
│                                                                                │
│    ⚠️  PCB MANUFACTURING NOTE:                                               │
│    By default, manufacturers plate all holes and connect to ground pour.      │
│    EXPLICITLY specify MH2-MH4 as NPTH (Non-Plated Through Hole) or           │
│    request isolated pads with no connection to ground plane.                  │
│                                                                                │
│    TEST POINTS:                                                               │
│    ────────────                                                               │
│    Add test pads for: GND, 5V, 3.3V, 3.3V_ANALOG, each ADC input             │
│                                                                                │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 12.7 Silkscreen Requirements

| Marking                              | Location                        | Purpose                                |
| ------------------------------------ | ------------------------------- | -------------------------------------- |
| ⚠️ HIGH VOLTAGE                      | Near AC input and relays        | Safety warning                         |
| ⚠️ DISCONNECT MAINS BEFORE USB DEBUG | Near USB/service header         | Ground loop warning                    |
| Dashed boundary line                 | Between HV and LV zones         | Visual separation                      |
| L, N, ⏚                              | Mains input terminal            | Wire identification                    |
| ⏚ (PE symbol)                        | At MH1 mounting hole            | PE star point / bonding screw location |
| K1, K2, K3                           | Relay terminals                 | Function identification                |
| SSR1+, SSR1-, etc.                   | SSR connectors                  | Polarity marking                       |
| Pin numbers                          | All connectors                  | Wiring reference                       |
| R1, C1, U1, etc.                     | All components                  | Assembly reference                     |
| Version, date                        | Board corner                    | Revision tracking                      |
| Polarity marks                       | Electrolytic caps, diodes, LEDs | Assembly guidance                      |
| Pin 1 indicator                      | ICs, Pico socket                | Orientation                            |

---

# 13. Connector Specifications

## 13.1 6.3mm Spade Terminals (High-Power Machine Connections)

High-current connections to original machine wiring use 6.3mm (0.25") spade terminals for plug & play compatibility.

### Power Metering Wiring

Relay-switched loads (pump, valves) are fused and distributed via internal bus. Power metering is handled by external PZEM-004T with CT clamp (no high current through PCB).

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                    POWER METERING - MAINS DISTRIBUTION                           │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                  │
│   FROM MAINS          ON PCB                             TO MACHINE LOADS       │
│   ──────────        ────────────                        ─────────────────       │
│                                                                                  │
│                    ┌─────────┐   ┌─────────┐                                    │
│   L_IN ───────────►│  FUSE   ├──────────────────► L_FUSED (internal bus)       │
│   (J1-L)           │  (F1)   │   │ (1mΩ)   │         │                          │
│                    └─────────┘   └─────────┘         ├──► Relay K1 COM          │
│                                                      ├──► Relay K2 COM          │
│                                                      ├──► Relay K3 COM          │
│                                                      └──► J24-L (PZEM 220V)     │
│                                                                                  │
│   N_IN ──────────────────────────────────────────────┬──► N (to all loads)      │
│                                                      └──► J24-N (PZEM 220V)     │
│                                                                                  │
│   ⚠️ SSR heater power: Mains → SSR → Heater (via existing machine wiring)       │
│      NOT through this PCB! PCB provides 5V control signals via J26 Pin 19-22.   │
│                                                                                  │
│   N_IN ──────────────────────────────────────────────────► N (to all loads)     │
│   (J1-N)                                                                         │
│                                                                                  │
│   PE ────────────────────────────────────────────────────► Chassis ground       │
│   (J1-PE)                                                                        │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

### High-Power Terminal Assignments (6.3mm Spade)

| Designator                                                   | Function            | Terminal Type | Wire Gauge | Notes                           |
| ------------------------------------------------------------ | ------------------- | ------------- | ---------- | ------------------------------- |
| **Mains Input**                                              |
| J1-L                                                         | Mains Live Input    | 6.3mm male    | 14 AWG     | Fused, to relay COMs            |
| J1-N                                                         | Mains Neutral Input | 6.3mm male    | 14 AWG     | Common neutral bus              |
| J1-PE                                                        | Protective Earth    | 6.3mm male    | 14 AWG     | To chassis                      |
| **220V AC Relay Outputs (All COMs internal to L_FUSED bus)** |
| J2-NO                                                        | Relay K1 N.O.       | 6.3mm male    | 16 AWG     | Water LED output (≤100mA, 220V) |
| J3-NO                                                        | Relay K2 N.O.       | 6.3mm male    | 14 AWG     | Pump output (5A peak, 220V)     |
| J4-NO                                                        | Relay K3 N.O.       | 6.3mm male    | 16 AWG     | Solenoid output (~0.5A, 220V)   |

**Note:** Relay K1, K2, K3 COMs are connected internally to the fused live bus - no external COM terminals needed.

**Spade Terminal Part Numbers:**

- PCB Mount: Keystone 1285 or TE 63951-1 (6.3mm blade)
- Use vertical or right-angle spade terminals depending on enclosure

## 13.1a Unified Low-Voltage Screw Terminal Block (J26 - 22 Position)

**ALL low-voltage connections are consolidated into a single 22-position screw terminal block.**

**⚠️ J26 is for LOW VOLTAGE ONLY! 220V AC relay outputs (K1, K2, K3) use 6.3mm spade terminals.**

**⚠️ CT Clamp connections REMOVED - now handled directly by external power meter module.**

```
┌──────────────────────────────────────────────────────────────────────────────────────────────────────┐
│                    UNIFIED LOW-VOLTAGE SCREW TERMINAL BLOCK (J26 - 22 Position)                      │
│                              Phoenix MKDS 1/22-5.08 (5.08mm pitch)                                   │
│                                    ⚠️ LOW VOLTAGE ONLY ⚠️                                            │
├──────────────────────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                                      │
│  SECTION A: SWITCHES     SECTION B: ANALOG SENSORS              SECTION C: SSR OUTPUTS       SPARE  │
│  ────────────────────    ─────────────────────────────────────  ───────────────────────────  ─────  │
│                                                                                                      │
│  ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐          │
│  │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │10 │11 │12 │13 │14 │15 │16 │17 │18 │19 │20 │21 │22 │          │
│  │S1 │S1G│S2 │S2G│S3 │S4 │S4G│T1 │T1G│T2 │T2G│TC+│TC-│P5V│PGD│PSG│SR+│SR-│SR+│SR-│GND│GND│          │
│  └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘          │
│   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │   │                  │
│   └─S1─┘   └─S2─┘  S3  └─S4─┘   └─T1──┘   └─T2──┘ └─TC──┘ └──Pressure──┘ └SSR1─┘ └SSR2─┘ Spare       │
│   Water    Tank   Lvl  Brew     Brew     Steam   Thermo    Transducer   Brew    Steam                │
│   Res.     Level  Prb  Handle   NTC      NTC     couple    (YD4060)     Heater  Heater               │
│                                                                                                      │
├──────────────────────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                                      │
│  COMPLETE PIN ASSIGNMENT TABLE:                                                                      │
│  ══════════════════════════════                                                                      │
│                                                                                                      │
│  │ Pin │ Label │ Function                    │ Wire   │ Signal      │ Notes                       │ │
│  │─────│───────│─────────────────────────────│────────│─────────────│─────────────────────────────│ │
│  │  1  │ S1    │ Water Reservoir Switch      │ 22 AWG │ GPIO2       │ Digital input, active low   │ │
│  │  2  │ S1-G  │ Water Reservoir GND         │ 22 AWG │ GND         │ Switch return               │ │
│  │  3  │ S2    │ Tank Level Sensor           │ 22 AWG │ GPIO3       │ Digital input, active low   │ │
│  │  4  │ S2-G  │ Tank Level GND              │ 22 AWG │ GND         │ Sensor return               │ │
│  │  5  │ S3    │ Steam Boiler Level Probe    │ 22 AWG │ PROBE       │ Via OPA342/TLV3201→GPIO4    │ │
│  │  6  │ S4    │ Brew Handle Switch          │ 22 AWG │ GPIO5       │ Digital input, active low   │ │
│  │  7  │ S4-G  │ Brew Handle GND             │ 22 AWG │ GND         │ Switch return               │ │
│  │  8  │ T1    │ Brew NTC Signal             │ 22 AWG │ NTC1_SIG    │ To ADC via divider          │ │
│  │  9  │ T1-G  │ Brew NTC GND                │ 22 AWG │ GND         │ Sensor return               │ │
│  │ 10  │ T2    │ Steam NTC Signal            │ 22 AWG │ NTC2_SIG    │ To ADC via divider          │ │
│  │ 11  │ T2-G  │ Steam NTC GND               │ 22 AWG │ GND         │ Sensor return               │ │
│  │ 12  │ TC+   │ Thermocouple +              │ 22 AWG │ TC_POS      │ K-type to MAX31855          │ │
│  │ 13  │ TC-   │ Thermocouple -              │ 22 AWG │ TC_NEG      │ K-type to MAX31855          │ │
│  │ 14  │ P-5V  │ Pressure Transducer +5V     │ 22 AWG │ +5V         │ Power for YD4060            │ │
│  │ 15  │ P-GND │ Pressure Transducer GND     │ 22 AWG │ GND         │ Sensor return               │ │
│  │ 16  │ P-SIG │ Pressure Transducer Signal  │ 22 AWG │ PRESS_SIG   │ 0.5-4.5V to ADC divider     │ │
│  │ 17  │ SSR1+ │ SSR1 Control +5V            │ 22 AWG │ +5V         │ Brew heater SSR power       │ │
│  │ 18  │ SSR1- │ SSR1 Control -              │ 22 AWG │ SSR1_NEG    │ Brew heater SSR trigger     │ │
│  │ 19  │ SSR2+ │ SSR2 Control +5V            │ 22 AWG │ +5V         │ Steam heater SSR power      │ │
│  │ 20  │ SSR2- │ SSR2 Control -              │ 22 AWG │ SSR2_NEG    │ Steam heater SSR trigger    │ │
│  │ 21  │ GND   │ Spare Ground                │ 22 AWG │ GND         │ Extra GND terminal          │ │
│  │ 22  │ GND   │ Spare Ground                │ 22 AWG │ GND         │ Extra GND terminal          │ │
│                                                                                                      │
│  WIRING NOTES:                                                                                       │
│  ─────────────                                                                                       │
│  • SWITCHES (Pin 1-7): N.O. switches connect between signal and adjacent GND pin                    │
│  • S3 (Pin 5): Level probe single wire, ground return via boiler body (PE connection)               │
│  • NTCs (Pin 8-11): 2-wire thermistors, polarity doesn't matter                                     │
│  • TC (Pin 12-13): OBSERVE POLARITY - red wire = TC-, yellow wire = TC+                             │
│  • PRESSURE (Pin 14-16): 3-wire transducer: +5V (red), GND (black), Signal (yellow/white)           │
│  • SSRs (Pin 17-20): Connect to SSR DC input terminals (+5V to SSR+, SSR- to SSR DC-)               │
│  • SPARE GND (Pin 21-22): Extra ground terminals for convenience                                     │
│                                                                                                      │
│  ⚠️ CT CLAMP: Connect directly to external power meter module (not through this PCB)                │
│                                                                                                      │
└──────────────────────────────────────────────────────────────────────────────────────────────────────┘
```

**Screw Terminal Part Number:**

- Phoenix Contact MKDS 1/22-5.08 (22-position, 5.08mm pitch)
- Alternative: 2× Phoenix MKDS 1/11-5.08 ganged together, or custom arrangement

## 13.2 Pin Headers and JST Connectors

| Designator | Function            | Type               | Pitch  | Notes                                     |
| ---------- | ------------------- | ------------------ | ------ | ----------------------------------------- |
| J15        | ESP32 Display       | JST-XH 8-pin       | 2.54mm | Keyed, locking (incl. WEIGHT_STOP, SPARE) |
| J16        | Service/Debug       | Pin header 4-pin   | 2.54mm | 3V3, GND, TX, RX (shared GPIO0/1)         |
| J17        | Power Meter (Univ.) | JST-XH 6-pin       | 2.54mm | 3V3, 5V, GND, RX, TX, DE/RE (TTL/RS485)   |
| J20        | Pico Socket         | 2×20 female header | 2.54mm | Or solder direct                          |
| J23        | I2C Accessory       | Pin header 4-pin   | 2.54mm | 3V3, GND, SDA, SCL (GPIO8/9)              |

**Note:** J24 (PZEM HV+CT) has been **REMOVED** - external power meters handle their own HV connections.

### Complete SSR Wiring (Each SSR has 2 connections to the board)

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│                    EXTERNAL SSR WIRING DIAGRAM                                   │
│                  (SSR1 for Brew Heater shown - SSR2 identical)                  │
├─────────────────────────────────────────────────────────────────────────────────┤
│                                                                                  │
│   YOUR CONTROL PCB                          EXTERNAL SSR + MACHINE WIRING        │
│   ─────────────────                         ─────────────────────────────        │
│                                                                                  │
│   ┌─────────────────┐                      ┌─────────────────┐                  │
│   │                 │    5V CONTROL        │   SSR-40DA      │                  │
│   │  ┌───────────┐  │    (LOW VOLTAGE)     │   (or similar)  │                  │
│   │  │ J26:19-20 │  │    ───────────       │                 │                  │
│   │  │  SSR1+  ──┼──┼───────────────────►──┼── DC+ (3-32V)   │                  │
│   │  │  SSR1-  ──┼──┼───────────────────►──┼── DC- (control) │                  │
│   │  └───────────┘  │                      │                 │    ┌─────────┐  │
│   │                 │                      │                 │    │  BREW   │  │
│   │  ┌───────────┐  │                      │   AC Load 1   ──┼───►│ HEATER  │  │
│   │  │ J26:21-22 │  │                      │                 │    │ 1400W   │  │
│   │  │  SSR2+  ──┼──┼───► (to SSR2)        │   AC Load 2   ◄─┼────┤         │  │
│   │  │  SSR2-  ──┼──┼───► (to SSR2)        │                 │    └─────────┘  │
│   │  └───────────┘  │                      └────────┬────────┘         ▲       │
│   │                 │                               │                  │       │
│   └─────────────────┘                               │                  │       │
│                                                     │                  │       │
│   ⚠️  MAINS POWER TO SSRs (NOT through PCB!)       │                  │       │
│   ─────────────────────────────────────────         │                  │       │
│                                                     │                  │       │
│   Machine's existing wiring:                        ▼                  │       │
│   ┌───────────────────────────────────────────────────────────────────┐│       │
│   │                                                                   ││       │
│   │   MAINS L ────────────────────────────────────► SSR AC IN (L1)   ││       │
│   │   (from wall)                                                     ││       │
│   │                                                                   ││       │
│   │   MAINS N ◄────────────────────────────────────────────────────────┘       │
│   │   (from wall)                                                              │
│   │                                                                            │
│   └────────────────────────────────────────────────────────────────────────────┘
│                                                                                  │
│   ✅ KEY DESIGN PRINCIPLE:                                                       │
│   ─────────────────────────                                                      │
│   • PCB provides ONLY low-voltage control signals (5V DC) to SSRs               │
│   • Mains power to SSRs uses EXISTING machine wiring (not through PCB)          │
│   • NO high current (heater loads) flows through the control PCB                │
│   • 10A fuse protects relay loads only (pump, valves)                           │
│                                                                                  │
│   CONNECTIONS PER SSR:                                                           │
│   ─────────────────────                                                          │
│   │ Connection      │ Source           │ Type       │ Voltage │                 │
│   │─────────────────│──────────────────│────────────│─────────│                 │
│   │ Control DC+     │ J26 Pin 19/21    │ Screw term │ 5V DC   │                 │
│   │ Control DC-     │ J26 Pin 20/22    │ Screw term │ GND     │                 │
│   │ Mains Live In   │ Machine wiring   │ Existing   │ 220V AC │ ← NOT from PCB │
│   │ Load Output     │ SSR AC terminal  │ Existing   │ 220V AC │                 │
│   │ Neutral         │ Machine wiring   │ Existing   │ 220V AC │                 │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

**JST Part Numbers:**

- JST B8B-XH-A (PCB header, 8-pin)
- JST XHP-6 (housing) with SXH-001T-P0.6 contacts (cable side)

---

# 14. Bill of Materials (BOM)

## 14.1 Integrated Circuits

| Qty | Ref | Description           | Part Number     | Package  | Notes                                      |
| --- | --- | --------------------- | --------------- | -------- | ------------------------------------------ |
| 1   | U1  | Raspberry Pi Pico     | SC0915          | Module   | Or Pico W                                  |
| 1   | U2  | AC/DC Converter 5V 3A | HLK-5M05        | Module   | Isolated, 3A, compact (alt: IRM-20-5)      |
| 1   | U3  | 3.3V LDO Regulator    | AP2112K-3.3TRG1 | SOT-23-5 | 600mA                                      |
| 1   | U4  | Thermocouple Amp      | MAX31855KASA+   | SOIC-8   | K-type                                     |
| 1   | U6  | Rail-to-Rail Op-Amp   | OPA342UA        | SOIC-8   | Level probe oscillator (alt: OPA207)       |
| 1   | U7  | Precision Comparator  | TLV3201AIDBVR   | SOT-23-5 | Level probe detector                       |
| 1   | U8  | RS485 Transceiver     | MAX3485ESA+     | SOIC-8   | For industrial meters (alt: SP3485EN-L/TR) |

## 14.2 Transistors and Diodes

| Qty | Ref     | Description      | Part Number | Package | Notes              |
| --- | ------- | ---------------- | ----------- | ------- | ------------------ |
| 6   | Q1-Q6   | NPN Transistor   | MMBT2222A   | SOT-23  | Relay/SSR drivers  |
| 2   | Q7-Q8   | N-Channel MOSFET | 2N7002      | SOT-23  | ESP32 control      |
| 4   | D1-D4   | Flyback Diode    | 1N4007      | DO-41   | Or MINIMELF        |
| 6   | D10-D15 | ESD Protection   | PESD5V0S1BL | SOD-323 | Sensor inputs      |
| 1   | D20     | TVS Diode        | SMBJ5.0A    | SMB     | 5V rail protection |

## 14.3 Passive Components - Resistors

| Qty | Ref     | Value | Tolerance | Package | Notes                                                            |
| --- | ------- | ----- | --------- | ------- | ---------------------------------------------------------------- |
| 1   | R1      | 3.3kΩ | 1%        | 0805    | Brew NTC pull-up (50kΩ NTC, optimized for 93°C)                  |
| 1   | R2      | 1.2kΩ | 1%        | 0805    | Steam NTC pull-up (50kΩ NTC, optimized for 135°C)                |
| 2   | R5-R6   | 1kΩ   | 1%        | 0805    | NTC ADC series protection                                        |
| 1   | R3      | 10kΩ  | 1%        | 0805    | Pressure divider                                                 |
| 1   | R4      | 4.7kΩ | 1%        | 0805    | Pressure divider (optimized for 90% ADC range)                   |
| 10  | R10-R19 | 10kΩ  | 5%        | 0805    | Pull-ups/pull-downs                                              |
| 8   | R20-R27 | 1kΩ   | 5%        | 0805    | Transistor base                                                  |
| 4   | R30-R33 | 470Ω  | 5%        | 0805    | Relay Indicator LEDs (4× relays) - brighter                      |
| 2   | R34-R35 | 330Ω  | 5%        | 0805    | SSR Indicator LEDs (logic-side)                                  |
| 4   | R40-R43 | 33Ω   | 5%        | 0805    | UART series (ESP32/Service)                                      |
| 2   | R44-R45 | 33Ω   | 5%        | 0805    | UART series (PZEM)                                               |
| 2   | R46-R47 | 4.7kΩ | 5%        | 0805    | I2C pull-ups (SDA, SCL)                                          |
| 1   | R48     | 330Ω  | 5%        | 0805    | Status LED                                                       |
| 1   | R49     | 100Ω  | 5%        | 0805    | Buzzer                                                           |
| 2   | R71-R72 | 10kΩ  | 5%        | 0805    | Pico RUN/BOOTSEL pull-ups (J15 Pin 5/6)                          |
| 1   | R73     | 10kΩ  | 5%        | 0805    | WEIGHT_STOP pull-down (J15 Pin 7)                                |
| 1   | R91     | 10kΩ  | 1%        | 0805    | Level probe oscillator feedback                                  |
| 2   | R92-R93 | 10kΩ  | 1%        | 0805    | Level probe Wien bridge                                          |
| 1   | R94     | 100Ω  | 5%        | 0805    | Level probe current limit                                        |
| 1   | R95     | 10kΩ  | 5%        | 0805    | Level probe AC bias                                              |
| 2   | R96-R97 | 100kΩ | 1%        | 0805    | Level probe threshold divider                                    |
| 1   | R98     | 1MΩ   | 5%        | 0805    | Level probe hysteresis                                           |
| 2   | R80-R81 | 100Ω  | 2W        | 1210    | **MANDATORY** - Snubber for K2 (Pump), K3 (Solenoid)             |
| 1   | R82     | 100Ω  | 2W        | 1210    | DNP - Snubber for K1 (footprint required, populate if inductive) |
| 1   | R99     | 120Ω  | 1%        | 0805    | RS485 termination (via JP1 solder jumper)                        |

## 14.4 Passive Components - Capacitors

| Qty | Ref     | Value    | Voltage | Package      | Notes                                                            |
| --- | ------- | -------- | ------- | ------------ | ---------------------------------------------------------------- |
| 1   | C1      | 100nF X2 | 275V AC | Radial       | Mains EMI filter                                                 |
| 2   | C2-C3   | 100µF    | 16V     | Radial 6.3mm | 5V bulk                                                          |
| 1   | C4      | 47µF     | 10V     | 1206 Ceramic | 3.3V output                                                      |
| 1   | C5      | 22µF     | 10V     | 1206 Ceramic | 3.3V analog                                                      |
| 12  | C10-C21 | 100nF    | 25V     | 0805         | Decoupling (general)                                             |
| 1   | C60     | 100nF    | 25V     | 0805         | OPA342 VCC decoupling                                            |
| 2   | C61-C62 | 100nF    | 25V     | 0805         | Level probe Wien bridge timing                                   |
| 1   | C63     | 100nF    | 25V     | 0805         | TLV3201 VCC decoupling                                           |
| 1   | C64     | 1µF      | 25V     | 0805         | Level probe AC coupling                                          |
| 1   | C65     | 100nF    | 25V     | 0805         | Level probe sense filter                                         |
| 4   | C30-C33 | 100pF    | 50V     | 0603         | UART/ADC filter                                                  |
| 1   | C40     | 10nF     | 50V     | 0805         | Thermocouple filter                                              |
| 2   | C50-C51 | 100nF X2 | 275V AC | Radial       | **MANDATORY** - Snubber for K2 (Pump), K3 (Solenoid)             |
| 1   | C70     | 100nF    | 25V     | 0805         | RS485 transceiver (U8) decoupling                                |
| 1   | C52     | 100nF X2 | 275V AC | Radial       | DNP - Snubber for K1 (footprint required, populate if inductive) |

## 14.5 Electromechanical

| Qty | Ref     | Description          | Part Number             | Notes                             |
| --- | ------- | -------------------- | ----------------------- | --------------------------------- |
| 2   | K1,K3   | Relay 5V 10A SPST-NO | HF46F-G/005-HS1         | SPST-NO                           |
| 1   | K2      | Relay 5V 16A SPST-NO | G5LE-1A4 DC5            | SPST-NO, Pump                     |
| 1   | F1      | Fuse 10A + Holder    | 0218010.MXP + 01000056Z | 5×20mm slow, PCB mount with cover |
| 1   | RV1     | Varistor 275V        | S14K275                 | 14mm disc                         |
| 2   | SW1-SW2 | Tactile Switch       | EVQP7A01P               | SMD 6×6mm                         |
| 1   | BZ1     | Passive Buzzer       | CEM-1203(42)            | 12mm                              |

## 14.6 LEDs

| Qty | Ref       | Description             | Color  | Package |
| --- | --------- | ----------------------- | ------ | ------- |
| 3   | LED1-LED3 | Relay Indicator (K1-K3) | Green  | 0805    |
| 2   | LED5-LED6 | SSR Indicator           | Orange | 0805    |
| 1   | LED7      | Status                  | Green  | 0805    |

## 14.7 Connectors

| Qty | Ref     | Description               | Part Number                | Notes                                          |
| --- | ------- | ------------------------- | -------------------------- | ---------------------------------------------- |
| 6   | J1-J4   | 6.3mm Spade Terminal      | Keystone 1285              | Mains (L,N,PE) + 220V relay outputs (K1,K2,K3) |
| 1   | **J26** | **Screw Terminal 22-pos** | **Phoenix MKDS 1/22-5.08** | **ALL LV connections - see 13.1a**             |
| 1   | J15     | JST-XH 8-pin Header       | B8B-XH-A                   | ESP32 display + brew-by-weight                 |
| 1   | J16     | Pin Header 1×4 2.54mm     | -                          | Service/debug (shared with J15)                |
| 1   | J17     | JST-XH 6-pin Header       | B6B-XH-A                   | Power Meter: 3V3, 5V, GND, RX, TX, DE/RE       |
| 1   | J20     | Female Header 2×20        | -                          | Pico socket                                    |
| 1   | J23     | Pin Header 1×4 2.54mm     | -                          | I2C accessory port                             |

**Note:** J24 (PZEM HV+CT header) has been **REMOVED** - external modules handle their own HV.

## 14.8 User-Supplied Components (NOT included with PCB)

The following components are **NOT** included with the PCB and must be sourced by the user:

| Component                   | Notes                                              |
| --------------------------- | -------------------------------------------------- |
| Raspberry Pi Pico           | SC0915 or Pico W (SC0918) for onboard WiFi         |
| JST-XH 8-pin Cable 50cm     | For ESP32 display connection (J15)                 |
| JST-XH 6-pin Cable 30-50cm  | For power meter connection (J17)                   |
| ESP32 Display Module        | User purchases separately                          |
| SSR Relays                  | Already exist on machine                           |
| External Power Meter Module | PZEM-004T, JSY-MK-163T, Eastron SDM, or compatible |

**EXTERNAL POWER METER INSTALLATION:**

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                   EXTERNAL POWER METER SETUP                                 │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   SUPPORTED MODULES (Off-Board, user mounts separately):                    │
│   ────────────────────────────────────────────────────                       │
│   • PZEM-004T-100A (TTL UART, 9600 baud, with CT clamp)                     │
│   • JSY-MK-163T (TTL UART, 4800 baud, bi-directional)                       │
│   • JSY-MK-194T (TTL UART, 4800 baud, dual-channel)                         │
│   • Eastron SDM120/230 (RS485, DIN rail mount)                              │
│   • Other Modbus-RTU compatible energy meters                               │
│                                                                              │
│   CONNECTION TO CONTROL PCB:                                                 │
│   ──────────────────────────                                                 │
│   ┌─────────────────┐          ┌─────────────────────────┐                  │
│   │  Control PCB    │   JST    │  External Power Meter   │                  │
│   │                 │  Cable   │  (PZEM, JSY, etc.)      │                  │
│   │  ┌───────────┐  │ (4-wire) │                         │                  │
│   │  │    J17    │──┼─────────►│  LV UART/RS485 input    │                  │
│   │  │  JST-XH   │  │          │  5V, GND, TX, RX        │                  │
│   │  │   6-pin   │  │          │                         │                  │
│   │  └───────────┘  │          │  HV terminals (L, N, CT)│◄── User wires   │
│   └─────────────────┘          │  directly to mains      │    mains here   │
│                                └─────────────────────────┘                  │
│                                                                              │
│   ⚠️ NO HIGH VOLTAGE flows through control PCB for power metering           │
│   ⚠️ User wires machine mains L/N directly to external meter                │
│   ⚠️ CT clamp clips around machine's Live wire, connects to meter           │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

**Cable Notes:**

- JST-XH 8-pin (J15): 5V, GND, TX, RX, RUN, BOOT, WEIGHT_STOP, SPARE - 50cm for ESP32
- JST-XH 6-pin (J17): 3V3, 5V, GND, RX, TX, DE/RE - 30-50cm for power meter

---

# 15. Testing & Validation

**For detailed test procedures, see:** `docs/hardware/Test_Procedures.md`

## 15.1 Test Points Required on PCB

| TP# | Signal   | Location        | Purpose          |
| --- | -------- | --------------- | ---------------- |
| TP1 | GND      | Near Pico       | Ground reference |
| TP2 | 5V       | Near LDO input  | Power rail check |
| TP3 | 3.3V     | Near Pico       | Power rail check |
| TP4 | 3.3V_A   | Near ADC        | Analog reference |
| TP5 | ADC0     | Near Pico       | Brew NTC signal  |
| TP6 | ADC1     | Near Pico       | Steam NTC signal |
| TP7 | ADC2     | Near Pico       | Pressure signal  |
| TP8 | UART0_TX | Near ESP32 conn | Debug probe      |
| TP9 | UART0_RX | Near ESP32 conn | Debug probe      |

---

# 16. Deliverables

## 16.1 Design Documentation

| Document         | Format       | Description                                     |
| ---------------- | ------------ | ----------------------------------------------- |
| Schematic        | PDF + Native | Complete circuit schematic with all connections |
| BOM              | Excel/CSV    | All components with Manufacturer Part Numbers   |
| PCB Layout       | Gerber X2    | Manufacturing files for board fabrication       |
| Drill File       | Excellon     | NC drill data                                   |
| Pick & Place     | CSV          | Component positions for SMT assembly            |
| Assembly Drawing | PDF          | Component placement, polarity, notes            |
| 3D Model         | STEP         | Board with components for enclosure design      |

## 16.2 Gerber File List

| Layer              | Filename         | Description            |
| ------------------ | ---------------- | ---------------------- |
| Top Copper         | \*-F_Cu.gtl      | Front copper layer     |
| Bottom Copper      | \*-B_Cu.gbl      | Back copper layer      |
| Top Solder Mask    | \*-F_Mask.gts    | Front solder mask      |
| Bottom Solder Mask | \*-B_Mask.gbs    | Back solder mask       |
| Top Silkscreen     | \*-F_SilkS.gto   | Front legend           |
| Bottom Silkscreen  | \*-B_SilkS.gbo   | Back legend            |
| Top Paste          | \*-F_Paste.gtp   | SMD stencil (optional) |
| Board Outline      | \*-Edge_Cuts.gm1 | PCB boundary           |
| Drill File         | \*.drl           | Plated through-holes   |
| Drill Map          | \*-drl_map.pdf   | Drill visualization    |

## 16.3 PCB Fabrication Specifications

```
┌────────────────────────────────────────────────────────────────────────────────┐
│                       PCB FABRICATION NOTES                                     │
├────────────────────────────────────────────────────────────────────────────────┤
│                                                                                 │
│    Material:           FR-4 TG130 (or higher)                                  │
│    Layers:             2                                                        │
│    Dimensions:         130mm × 80mm                                            │
│    Thickness:          1.6mm ±10%                                              │
│    Copper Weight:      2oz (70µm) both layers                                  │
│    Min Trace/Space:    0.2mm / 0.2mm (8mil/8mil)                              │
│    Min Drill:          0.3mm                                                   │
│    Mounting Holes:     4× M3 (3.2mm), 5mm from edges                          │
│    Surface Finish:     ENIG (preferred) or HASL Lead-Free                     │
│    Solder Mask:        Green (both sides)                                      │
│    Silkscreen:         White (both sides)                                      │
│    UL Marking:         Required (UL 94V-0)                                     │
│    IPC Class:          Class 2 minimum                                         │
│    Routed Slots:       Yes (isolation slots as marked)                         │
│                                                                                 │
└────────────────────────────────────────────────────────────────────────────────┘
```

## 16.4 Assembly Notes

1. **SMT components first**, then through-hole
2. **Pico module**: Solder directly or use socket (socketed preferred for prototype)
3. **HLK-PM05**: Solder last, check orientation (L, N marking)
4. **Relays**: Check coil polarity if polarized
5. **Electrolytic capacitors**: Observe polarity markings
6. **ESD handling**: Use proper ESD precautions for Pico and ICs
7. **Conformal coating**: Apply to HV section after testing (optional)

## 16.5 Firmware Interface Summary

**For firmware implementation details, see:** `docs/pico/` directory

Hardware interfaces the firmware must support:

- UART0 (GPIO0/1): ESP32 communication @ 115200 baud
- UART1 (GPIO6/7): Power meter @ configurable baud (9600/4800/2400)
- SPI0 (GPIO16-18): MAX31855 thermocouple
- ADC (GPIO26-28): NTC thermistors, pressure transducer
- GPIO20: RS485 DE/RE direction control

---

# 17. Design Summary

## 17.1 Key Design Decisions

| Item                 | Decision                                                       |
| -------------------- | -------------------------------------------------------------- |
| Power Metering       | Universal external modules (PZEM, JSY, Eastron via J17 JST-XH) |
| Pressure Transducers | J26 Pin 14-16 (0.5-4.5V amplified output)                      |
| Pico Mounting        | Socket (2×20 female header) for easy replacement               |
| Level Probe          | OPA342 + TLV3201 on-board AC sensing                           |

## 17.2 Critical Design Points

1. **Only relay-switched loads flow through control board** (pump, valves ~6A max)
2. **Heater power does NOT flow through PCB** (external SSRs connect directly to mains)
3. **Power metering HV does NOT flow through PCB** (external modules handle their own mains)
4. **Fused live bus** (10A) feeds relay COMs only
5. **6mm creepage/clearance** required between HV and LV sections
6. **MH1 = PE star point** (PTH), MH2-4 = NPTH (isolated)

## 17.3 EDA Tool Requirements

**Preferred:** KiCad 7.0+ (open source)

Deliver: Native project files, PDF schematic, Gerber files (Section 16.2), STEP 3D model

---

# Appendix A: Reference Documents

**Safety Standards:** IEC 60950-1, IEC 62368-1, UL 60950-1

**Key Datasheets:** Raspberry Pi Pico, RP2040, MAX31855, OPA342, TLV3201, MAX3485

**Machine Reference:** ECM Synchronika Service Manual, Parts Diagram

---

**Revision History:** See `CHANGELOG.md`

---

_End of Document_
