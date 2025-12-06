# Hardware Specification Changelog

## Revision History

| Rev  | Date     | Description                                             |
| ---- | -------- | ------------------------------------------------------- |
| 2.21 | Dec 2025 | **CURRENT** - Universal external power metering (J17)   |
| 2.20 | Dec 2025 | Unified 22-pos screw terminal (J26)                     |
| 2.19 | Dec 2025 | Removed spare relay K4                                  |
| 2.17 | Nov 2025 | Brew-by-weight support (J15 8-pin)                      |
| 2.16 | Nov 2025 | Production-ready specification                          |

---

## v2.21 (December 2025)

**Major Change: Universal External Power Metering**

- **REMOVED:** Embedded PZEM-004T daughterboard (J17 female header, J24 HV header)
- **NEW:** Universal J17 connector (JST-XH 6-pin) for external meter modules
- **Supports:** PZEM-004T, JSY-MK-163T/194T, Eastron SDM, and other Modbus meters
- **Added:** MAX3485 RS485 transceiver (U8) with jumper-selectable 120Ω termination
- **J26 reduced:** 24-pos → 22-pos (CT clamp pins removed, now on external module)
- **No HV on PCB for metering:** External modules handle their own mains connections

---

## v2.20 (December 2025)

**Unified Low-Voltage Terminal Block**

- **J26 unified screw terminal (24-pos):** ALL low-voltage connections consolidated
- **Includes:** Switches (S1-S4), NTCs (T1-T2), Thermocouple, Pressure, CT clamp, SSRs
- **Eliminates:** J10, J11, J12, J13, J18, J19, J25 (all merged into J26)
- **6.3mm spades retained ONLY for 220V AC:** Mains input and relay outputs

---

## v2.19 (December 2025)

**Simplified Relay Configuration**

- Removed spare relay K4 and associated components
- GPIO20 made available as test point TP1

---

## v2.17 (November 2025)

**Brew-by-Weight Support**

- J15 ESP32 connector expanded from 6-pin to 8-pin JST-XH
- GPIO21: WEIGHT_STOP signal (ESP32 → Pico)
- GPIO22: SPARE for future expansion
- Enables Bluetooth scale integration via ESP32

---

## v2.16 (November 2025)

**Production-Ready Specification**

- Power supply: HLK-5M05 (5V 3A, compact 16mm height)
- Level probe: OPA342 + TLV3201 AC sensing circuit
- Snubbers: MANDATORY for K2 (Pump) and K3 (Solenoid)
- NTC pull-ups: Optimized for 50kΩ NTCs (R1=3.3kΩ, R2=1.2kΩ)
- Mounting: MH1=PE star point (PTH), MH2-4=NPTH (isolated)
- SSR control: 5V trigger signals only, mains via existing machine wiring

