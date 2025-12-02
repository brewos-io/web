# ECM Synchronika Control Board - Schematic Reference

## Detailed Circuit Schematics for PCB Design

**Board Size:** 130mm × 80mm (2-layer, 2oz copper)  
**Enclosure:** 150mm × 100mm mounting area  
**Revision:** Matches ECM_Control_Board_Specification_v2.md (v2.20)

---

## ⚠️ KEY DESIGN CHANGES IN v2.20

1. **Unified J26 Screw Terminal (24-pos)** - ALL low-voltage connections consolidated into single Phoenix MKDS 1/24-5.08 terminal block
2. **J26 includes:** Switches (S1-S4), NTCs (T1-T2), Thermocouple, Pressure transducer, CT clamp, SSR control outputs
3. **Eliminates:** J10, J11, J12, J13, J18, J19, J25 (all merged into J26)
4. **6.3mm spades retained ONLY for 220V AC**: Mains input (L, N, PE), K1 LED (J2), K2 Pump (J3), K3 Solenoid (J4)
5. **OPA342 + TLV3201** for steam boiler level probe (AC sensing with common components)
6. **PZEM-004T v3.0** external power meter (NO high current through PCB)
7. **HLK-5M05** power supply (3A, compact 16mm height)
8. **10A fuse** with PCB mount holder (Littelfuse 01000056Z)
9. **Optimized NTC pull-ups**: R1=3.3kΩ (brew 93°C), R2=1.2kΩ (steam 135°C)
10. **Pressure divider R4=4.7kΩ** (91% ADC range utilization)
11. **Snubbers MANDATORY** for K2 (Pump) and K3 (Solenoid)
12. **Mounting holes**: MH1=PE star point (PTH), MH2-4=NPTH (isolated)

---

# Sheet 1: Power Supply

## 1.1 Mains Input & Protection

```
                                    MAINS INPUT & PROTECTION
    ════════════════════════════════════════════════════════════════════════════

    J1 (Mains Input)
    ┌─────────────┐
    │ L  (Live)   ├────┬──────────────────────────────────────────────────────────►
    │             │    │
    │ N  (Neutral)├──┐ │    F1                RV1              C1
    │             │  │ │  ┌─────┐          ┌──────┐        ┌──────┐
    │ PE (Earth)  ├─┐│ └──┤ 10A ├────┬─────┤ MOV  ├────┬───┤ 100nF├───┬─────► L_FUSED
    └─────────────┘ ││    │250V │    │     │ 275V │    │   │ X2   │   │
                    ││    │T/Lag│    │     │ 14mm │    │   │275VAC│   │
        To Chassis  ││    └─────┘    │     └──┬───┘    │   └──┬───┘   │
        Ground      ││               │        │        │      │       │
        (if metal   │└───────────────┼────────┴────────┴──────┴───────┴─────► N_FUSED
        enclosure)  │                │
                    │                │
                   ─┴─              ─┴─
                   GND              GND
                   (PE)            (Mains)

    Component Values:
    ─────────────────
    F1:  Littelfuse 0218010.MXP, 10A 250V, 5x20mm, Slow-blow (relay loads only ~6A)
    RV1: Epcos S14K275 or Littelfuse V275LA20AP, 275VAC, 14mm disc
    C1:  TDK B32922C3104M, 100nF, 275VAC, X2 Safety Rated
```

## 1.2 AC/DC Isolated Power Supply

```
                                AC/DC ISOLATED CONVERTER
    ════════════════════════════════════════════════════════════════════════════

                                    U2: Hi-Link HLK-5M05
                                    (3A, compact 16mm height)
                              ┌──────────────────────────┐
                              │                          │
    L_FUSED ──────────────────┤ L               +Vout    ├───────┬──────────► +5V
                              │     ╔═══════╗            │       │
                              │     ║ 3kV   ║            │   C2  │
                              │     ║ISOLATE║            │ ┌─────┴─────┐
    N_FUSED ──────────────────┤ N   ╚═══════╝    -Vout   │ │   100µF   │
                              │                          ├─┤   16V     │
                              │                          │ │   Alum.   │
                              └──────────────────────────┘ └─────┬─────┘
                                                                 │
                                                                ─┴─
                                                                GND
                                                              (Secondary)

    NOTES:
    ══════
    • Primary side (L, N) is MAINS VOLTAGE - maintain 6mm clearance to secondary
    • Secondary side (-Vout) becomes system GND
    • GND is ISOLATED from mains PE (earth)
    • HLK-5M05 is 50×27×16mm - verify clearance to fuse holder

    Component Values:
    ─────────────────
    U2:  Hi-Link HLK-5M05 (5V 3A) - adequate for ~1.1A peak load
         Alt: Mean Well IRM-20-5 (5V 4A) if more headroom needed
    C2:  100µF 16V Electrolytic, Low ESR, 6.3mm diameter
```

## 1.3 5V to 3.3V LDO Regulator

```
                                3.3V LDO REGULATOR
    ════════════════════════════════════════════════════════════════════════════

                            U3: AP2112K-3.3TRG1
                           ┌──────────────────┐
                           │                  │
    +5V ─────┬─────────────┤ VIN        VOUT  ├───────┬───────────────────► +3.3V
             │             │                  │       │
         C3  │             │       GND        │   C4  │   C5
       ┌─────┴─────┐       │        │         │ ┌─────┴─────┐ ┌─────┐
       │   100µF   │       └────────┼─────────┘ │   47µF    │ │100nF│
       │   16V     │                │           │   10V     │ │ 25V │
       │   Alum.   │                │           │  Ceramic  │ │     │
       └─────┬─────┘                │           │   1206    │ └──┬──┘
             │                      │           └─────┬─────┘    │
            ─┴─                    ─┴─               ─┴─        ─┴─
            GND                    GND               GND        GND


                        Ferrite Bead for Analog Section
                        ───────────────────────────────

    +3.3V ──────────┬───[FB1: 600Ω @ 100MHz]───┬──────────────────► +3.3V_ANALOG
                    │                          │
                C6  │                      C7  │
              ┌─────┴─────┐              ┌─────┴─────┐
              │   100nF   │              │   22µF    │
              │    25V    │              │   10V     │
              │  Ceramic  │              │  Ceramic  │
              └─────┬─────┘              └─────┬─────┘
                   ─┴─                        ─┴─
                   GND                        GND

    Component Values:
    ─────────────────
    U3:  Diodes Inc. AP2112K-3.3TRG1, 600mA, SOT-23-5
    C3:  100µF 16V Electrolytic
    C4:  47µF 10V Ceramic, 1206 (X5R or X7R)
    C5:  100nF 25V Ceramic, 0805
    FB1: Murata BLM18PG601SN1D, 600Ω @ 100MHz, 0603
    C6:  100nF 25V Ceramic, 0805
    C7:  22µF 10V Ceramic, 1206
```

---

# Sheet 2: Microcontroller (Raspberry Pi Pico)

## 2.1 Pico Module & Decoupling

```
                            RASPBERRY PI PICO MODULE
    ════════════════════════════════════════════════════════════════════════════

                                U1: Raspberry Pi Pico

         +3.3V ◄────────────────┐           ┌────────────────► (Internal 3.3V)
                                │           │
         +5V ───────────────────┤ VSYS  3V3 ├───────────────── (3.3V out, max 300mA)
                   ┌────┐       │           │       ┌────┐
         GND ──┬───┤100n├───────┤ GND   GND ├───────┤100n├───┬── GND
               │   └────┘       │           │       └────┘   │
               │                │           │                │
               │     ┌──────────┤ GP0   GP28├────────────────┼─► ADC2 (Pressure)
               │     │          │           │                │
    ESP32_TX◄──┼─────┤          │ GP1   GP27├────────────────┼─► ADC1 (Steam NTC)
               │     │          │           │                │
    ESP32_RX ──┼─────┤          │ GP2   GP26├────────────────┼─► ADC0 (Brew NTC)
               │     │          │           │                │
               │     │          │ GP3   RUN ├────────────────┼── SW1 (Reset)
               │     │          │           │                │
               │     │          │ GP4  GP22 ├────────────────┼── (Spare)
               │     │          │           │                │
               │     │          │ GP5  GP21 ├────────────────┼── (Spare)
               │     │          │           │                │
               │     │          │ GP6  GP20 ├────────────────┼─► (spare)
               │     │          │           │                │
               │     │          │ GP7  GP19 ├────────────────┼─► BUZZER
               │     │          │           │                │
               │     │          │ GP8  GP18 ├────────────────┼─► SPI_SCK
               │     │          │           │                │
               │     │          │ GP9  GP17 ├────────────────┼─► SPI_CS
               │     │          │           │                │
               │     │          │ GP10 GP16 ├────────────────┼─► SPI_MISO
               │     │          │           │                │
               │     │          │ GP11 GP15 ├────────────────┼─► STATUS_LED
               │     │          │           │                │
               │     │          │ GP12 GP14 ├────────────────┼─► SSR2_STEAM
               │     │          │           │                │
               │     │          │ GP13 BOOT ├────────────────┼── SW2 (Bootsel)
               │                │           │                │
               │                │ GND   GND │                │
               │                │     │     │                │
               └────────────────┴─────┼─────┴────────────────┘
                                      │
                                     ─┴─
                                     GND

    Decoupling Capacitors (place adjacent to Pico):
    ───────────────────────────────────────────────
    • 100nF ceramic on each VSYS pin
    • 100nF ceramic on each 3V3 pin
    • 10µF ceramic near VSYS input (optional)
```

## 2.2 Reset and Boot Buttons

```
                            RESET AND BOOT BUTTONS
    ════════════════════════════════════════════════════════════════════════════

    RESET BUTTON (SW1)                      BOOT BUTTON (SW2)
    ──────────────────                      ─────────────────

         +3.3V                                    BOOTSEL
           │                                    (Pico TP6)
      ┌────┴────┐                                   │
      │  10kΩ   │  ◄── Internal on Pico            │
      │  R70    │      (may not need external)     │
      └────┬────┘                              ┌───┴───┐
           │                                   │  SW2  │
           ├──────────────► RUN Pin            │ BOOT  │  6x6mm SMD
           │                (Pico)             │       │  Tactile
       ┌───┴───┐                               └───┬───┘
       │  SW1  │  6x6mm SMD                        │
       │ RESET │  Tactile                         ─┴─
       │       │  (EVQP7A01P)                     GND
       └───┬───┘
           │
          ─┴─
          GND

    Operation:
    ──────────
    • Press SW1 → Reset Pico
    • Hold SW2 + Press SW1 + Release SW1 + Release SW2 → Enter USB Bootloader
```

---

# Sheet 3: Relay Drivers

## 3.1 Relay Driver Circuit (K1, K2, K3)

```
                            RELAY DRIVER CIRCUIT
    ════════════════════════════════════════════════════════════════════════════

    (Identical circuit for K1, K2, K3. K2 uses 16A relay for pump.)

                                        +5V
                                         │
               ┌─────────────────────────┴─────────────────────────┐
               │                                                   │
          ┌────┴────┐                                         ┌────┴────┐
          │  Relay  │                                         │  470Ω   │
          │  Coil   │                                         │  R30+n  │
          │   5V    │                                         │  (LED)  │
          │  K1/2/3 │                                         └────┬────┘
          └────┬────┘                                              │
               │                                              ┌────┴────┐
          ┌────┴────┐                                         │   LED   │
          │  D1/2/3 │ ◄── Flyback diode                       │  Green  │
          │ 1N4007  │                                         │ LED1-3  │
          └────┬────┘                                         └────┬────┘
               │                                                   │
               └───────────────────────┬───────────────────────────┤
                                       │                           │
                                  ┌────┴────┐                      │
                                  │    C    │                      │
                                  │  Q1/2/3 │  MMBT2222A           │
                                  │   NPN   │  SOT-23              │
    GPIO (10/11/12) ────[1kΩ]────►│    B    │                      │
                        R20+n     │    E    ├──────────────────────┘
                           │      └────┬────┘
                      ┌────┴────┐      │
                      │  10kΩ   │     ─┴─
                      │  R10+n  │     GND
                      │ Pull-dn │
                      └────┬────┘
                          ─┴─
                          GND

    OPERATION:
    ──────────
    GPIO LOW  → Transistor OFF → Relay OFF, LED OFF
    GPIO HIGH → Transistor ON  → Relay ON, LED ON

    Relay coil current: ~70mA (through transistor collector)
    LED current: (5V - 2.0V) / 470Ω = 6.4mA (bright indicator)

    Relay Contact Connections (All 220V AC - 6.3mm Spade Terminals):
    ─────────────────────────────────────────────────────────────────

    K1 (Water LED):            K2 (Pump):                K3 (Solenoid):
    J2-NO ── K1-NO             J3-NO ── K2-NO            J4-NO ── K3-NO
    (6.3mm spade, 220V)        (6.3mm spade, 220V 5A)    (6.3mm spade, 220V)
    COM internal to L_FUSED    COM internal to L_FUSED   COM internal to L_FUSED

    Component Values:
    ─────────────────
    K1, K3: HF46F-G/005-HS1 (5V coil, 10A contacts, SPST-NO)
    K2:     Omron G5LE-1A4 DC5 (5V coil, 16A contacts, SPST-NO) - FOR PUMP
    D1-D3:  1N4007 (1A, 1000V) or LL4007 MINIMELF
    Q1-Q3:  MMBT2222A (SOT-23)
    LED1-3: Green 0805, Vf~2.0V
    R20-22: 1kΩ 5% 0805 (transistor base)
    R30-32: 470Ω 5% 0805 (LED current limit - gives 6.4mA)
    R10-12: 10kΩ 5% 0805 (pull-down, ensures relay off at boot)

    ═══════════════════════════════════════════════════════════════════════════
    RC SNUBBER CIRCUITS (Inductive Load Protection)
    ═══════════════════════════════════════════════════════════════════════════

    ⚠️ MANDATORY for K2 (Pump) and K3 (Solenoid) - prevents EMI crashes!

    Across relay contacts (HV side):
    ─────────────────────────────────

         Relay NO ──────┬───────────────────────────► To Load
                        │
                   ┌────┴────┐      ┌────────┐
                   │  100nF  │──────┤  100Ω  │
                   │   X2    │      │   2W   │
                   │  275VAC │      │        │
                   └────┬────┘      └────┬───┘
                        │               │
                        └───────┬───────┘
                                │
         Relay COM ─────────────┴───────────────────── From L_FUSED

    Snubber Components:
    ────────────────────
    K2 (Pump):     C50 (100nF X2) + R80 (100Ω 2W) - MANDATORY
    K3 (Solenoid): C51 (100nF X2) + R81 (100Ω 2W) - MANDATORY
    K1 (LED):      C52 (100nF X2) + R82 (100Ω 2W) - DNP (footprint only)

    Snubber Component Values:
    ─────────────────────────
    C50-C53: 100nF X2, 275V AC (Vishay MKP1840 or TDK B32922)
    R80-R83: 100Ω 2W, Metal Film (1210 package or through-hole)
```

---

# Sheet 4: SSR Drivers

## 4.1 SSR Trigger Circuit

```
                            SSR TRIGGER CIRCUIT
    ════════════════════════════════════════════════════════════════════════════

    (Identical circuit for SSR1 and SSR2)

    IMPORTANT: SSR has internal current limiting - NO series resistor needed!
    SSR input spec: 4-32V DC (KS15 D-24Z25-LQ)

                                        +5V
                                         │
               ┌─────────────────────────┴─────────────────────────┐
               │                                                   │
               │                                              ┌────┴────┐
               │                                              │   1kΩ   │
               │                                              │ R34/35  │
               │                                              └────┬────┘
               ▼                                                   │
    ┌──────────────────┐                                      ┌────┴────┐
    │  To SSR Input    │                                      │   LED   │
    │  (+) Positive    │                                      │ Orange  │
    │ J26-19/21 (SSR+) │                                      │ LED5/6  │
    └────────┬─────────┘                                      └────┬────┘
             │                                                     │
             │    ┌───────────────────────────────┐                │
             │    │   EXTERNAL SSR                │                │
             │    │   (has internal LED+resistor) │                │
             │    │   KS15 D-24Z25-LQ             │                │
             │    └───────────────────────────────┘                │
             │                                                     │
    ┌────────┴─────────┐                                           │
    │  To SSR Input    │                                           │
    │  (-) Negative    ├───────────────────────────────────────────┤
    │ J26-20/22 (SSR-) │                                           │
    └────────┬─────────┘                                      ┌────┴────┐
             │                                                │    C    │
             │                                                │  Q5/Q6  │  MMBT2222A
             │                                                │   NPN   │
             └───────────────────────────────────────────────►│    E    │
                                                              └────┬────┘
                                                                   │
    GPIO (13/14) ───────[1kΩ R24/25]──────────────────────────────►B
                                  │                                │
                             ┌────┴────┐                          ─┴─
                             │  10kΩ   │                          GND
                             │ R14/15  │
                             └────┬────┘
                                 ─┴─
                                 GND

    OPERATION:
    ──────────
    GPIO LOW  → Transistor OFF → SSR- floating → SSR OFF (no current path)
    GPIO HIGH → Transistor ON  → SSR- to GND   → SSR ON  (~4.8V across SSR)

    Voltage at SSR = 5V - Vce(sat) = 5V - 0.2V = 4.8V  ✓ (exceeds 4V minimum)

    External SSR Connections:
    ─────────────────────────
    SSR1 (Brew Heater):
        J26-19 (SSR1+) ── SSR Input (+) ── from 5V rail
        J26-20 (SSR1-) ── SSR Input (-) ── to transistor collector
        SSR AC Load side: Connected via EXISTING MACHINE WIRING (not through PCB)

    SSR2 (Steam Heater):
        J26-21 (SSR2+) ── SSR Input (+) ── from 5V rail
        J26-22 (SSR2-) ── SSR Input (-) ── to transistor collector
        SSR AC Load side: Connected via EXISTING MACHINE WIRING (not through PCB)

    ⚠️ IMPORTANT: NO HIGH CURRENT THROUGH PCB
    ────────────────────────────────────────
    The PCB provides ONLY 5V control signals to the SSRs.
    Mains power to SSRs uses the machine's existing wiring:
    • Machine mains Live → SSR AC input
    • SSR AC output → Heater element
    • Heater return → Machine mains Neutral

    Component Values:
    ─────────────────
    External SSRs: KS15 D-24Z25-LQ (25A, 4-32V DC input, 24-280V AC output)
    R24-25: 1kΩ 5% 0805 (transistor base drive, ~3mA)
    R34-35: 1kΩ 5% 0805 (LED current limit, ~3mA)
    R14-15: 10kΩ 5% 0805 (pull-down, keeps SSR OFF at boot)
    Q5-Q6:  MMBT2222A (SOT-23), Vce(sat) < 0.3V @ 100mA
    LED5-6: Orange 0805, Vf~2.0V
```

---

# Sheet 5: Sensor Inputs

## 5.1 NTC Thermistor Inputs

```
                        NTC THERMISTOR INPUT CIRCUITS
    ════════════════════════════════════════════════════════════════════════════

    ⚠️ EACH SENSOR HAS DIFFERENT PULL-UP - OPTIMIZED FOR TARGET TEMPERATURE
    ⚠️ CONFIGURED FOR 50kΩ NTC SENSORS (ECM Synchronika standard)

    BREW NTC (GPIO26/ADC0)                  STEAM NTC (GPIO27/ADC1)
    Target: 90-100°C                        Target: 125-150°C

         +3.3V_ANALOG                            +3.3V_ANALOG
              │                                       │
         ┌────┴────┐                             ┌────┴────┐
         │  3.3kΩ  │  R1 (Brew)                  │  1.2kΩ  │  R2 (Steam)
         │  ±1%    │  Optimized for 93°C         │  ±1%    │  Optimized for 135°C
         └────┬────┘                             └────┬────┘
              │                                       │
   J26-8 ─────┼────┬───────► GPIO26          J26-10 ──┼────┬───────► GPIO27
              │    │         (ADC0)                   │    │         (ADC1)
         ┌────┴────┐  ┌────┴────┐             ┌────┴────┐  ┌────┴────┐
         │   NTC   │  │  1kΩ    │             │   NTC   │  │  1kΩ    │
         │  50kΩ   │  │  R5     │             │  50kΩ   │  │  R6     │
         │  @25°C  │  └────┬────┘             │  @25°C  │  └────┬────┘
         └────┬────┘       │                  └────┬────┘       │
              │       ┌────┴────┐                  │       ┌────┴────┐
              │       │  100nF  │                  │       │  100nF  │
              │       │   C8    │                  │       │   C9    │
              │       └────┬────┘                  │       └────┬────┘
              │            │                       │            │
              └────────────┴─── GND                └────────────┴─── GND

    ESD Protection (both channels):
    ────────────────────────────────
    J26-8/10 (NTC) ┬──────────────────────────────────────────► To ADC
                   │
              ┌────┴────┐
              │ D10/D11 │  PESD5V0S1BL (ESD clamp)
              └────┬────┘
                  ─┴─
                  GND

    Component Values:
    ─────────────────
    R1:      3.3kΩ ±1%, 0805 (Brew NTC, optimized for 93°C)
    R2:      1.2kΩ ±1%, 0805 (Steam NTC, optimized for 135°C)
    R5-R6:   1kΩ 1%, 0805 (series protection)
    C8-C9:   100nF 25V, 0805, Ceramic
    D10-D11: PESD5V0S1BL, SOD-323 (bidirectional ESD clamp)

    NTC Sensors: 50kΩ @ 25°C, B25/85 ≈ 3950K (ECM Synchronika standard)

    Resolution at Target Temps:
    ───────────────────────────
    • Brew (93°C):  ~31 ADC counts/°C → 0.032°C resolution
    • Steam (135°C): ~25 ADC counts/°C → 0.04°C resolution
```

## 5.2 K-Type Thermocouple Input

```
                        K-TYPE THERMOCOUPLE INPUT
    ════════════════════════════════════════════════════════════════════════════

                                    +3.3V
                                      │
                                 ┌────┴────┐
                                 │  100nF  │  Decoupling
                                 │   C10   │
                                 └────┬────┘
                                      │
                      U4: MAX31855KASA+
                     ┌─────────────────────────┐
                     │                         │
    J26-12 (TC+)─────┤ T+   (1)        VCC (8) ├───────────────────── +3.3V
    (TC+)            │                         │
                     │                         │
    J26-13 (TC-)─┬───┤ T-   (2)        SCK (7) ├───────────────────── GPIO18 (SPI_SCK)
    (TC-)        │   │                         │
                 │   │                         │
            ┌────┴───┤ NC   (3)         CS (6) ├───────────────────── GPIO17 (SPI_CS)
            │ 10nF   │                         │
            │ C40    │                         │
            └───┬────┤ GND  (4)         DO (5) ├───────────────────── GPIO16 (SPI_MISO)
                │    │                         │
                │    └─────────────────────────┘
                │
               ─┴─
               GND

    Thermocouple Connector:
    ───────────────────────
    J26-12/13: Thermocouple connections (part of unified 24-pos terminal)
    Pin 1: TC+ (Yellow for K-type, or check thermocouple wire color code)
    Pin 2: TC- (Red for K-type)

    NOTE: Use proper K-type thermocouple connectors if available

    Component Values:
    ─────────────────
    U4:  MAX31855KASA+, SOIC-8, K-type specific
    C40: 10nF 50V Ceramic, 0805 (differential noise filter)
    C10: 100nF 25V Ceramic, 0805 (VCC decoupling)

    PCB Layout Notes:
    ─────────────────
    • Keep T+ and T- traces short and symmetric
    • Route away from power traces and relay coils
    • Add ground guard ring around thermocouple traces
```

## 5.3 Pressure Transducer Input (J26 Pin 14-16 - Amplified 0.5-4.5V)

```
                    PRESSURE TRANSDUCER INPUT (AMPLIFIED TYPE)
    ════════════════════════════════════════════════════════════════════════════

    J26-14/15/16: Pressure transducer (part of unified 24-pos terminal)

                            +5V
                             │
    J26-14 (P-5V)────────────┴────────────────────────────────── Transducer VCC
    (5V)                                                         (Red wire)

    J26-15 (P-GND)─────────────────────────────────────────────── Transducer GND
    (GND)           │                                            (Black wire)
                   ─┴─
                   GND

    J26-16 (P-SIG)──────────────────────┬─────────────────────── Transducer Signal
    (Signal)                            │                        (Yellow/White wire)
    0.5V - 4.5V                         │
                                   ┌────┴────┐
                                   │  4.7kΩ  │  R4 (Series) ±1%
                                   │         │
                                   └────┬────┘
                                        │
                                        ├──────────────┬───────────────────► GPIO28
                                        │              │                     (ADC2)
                                   ┌────┴────┐    ┌────┴────┐
                                   │  10kΩ   │    │  100nF  │  LP Filter
                                   │   R3    │    │   C11   │
                                   │   ±1%   │    └────┬────┘
                                   └────┬────┘         │
                                        │              │
                                       ─┴─            ─┴─
                                       GND            GND

    Voltage Divider Calculation (OPTIMIZED for 91% ADC range):
    ───────────────────────────────────────────────────────────
    Ratio = R3 / (R3 + R4) = 10k / (10k + 4.7k) = 0.68

    Input 0.5V  → Output 0.34V → ADC ~422
    Input 2.5V  → Output 1.70V → ADC ~2109
    Input 4.5V  → Output 3.06V → ADC ~3795

    Resolution: 0.0047 bar/count (15% better than old 15kΩ design)

    Component Values:
    ─────────────────
    R3: 10kΩ ±1%, 0805 (to GND)
    R4: 4.7kΩ ±1%, 0805 (series, from signal)
    C11: 100nF 25V Ceramic, 0805

    Selected Transducer: YD4060 Series
    ───────────────────────────────────
    - Pressure Range: 0-1.6 MPa (0-16 bar)
    - Output: 0.5-4.5V ratiometric
    - Supply: 5VDC, ≤3mA
    - Thread: 1/8" NPT (use adapter for espresso machine)
    - Wiring: Red=5V, Black=GND, Yellow=Signal
```

## 5.4 Digital Switch Inputs (J26 - Low Voltage Only)

```
                        DIGITAL SWITCH INPUTS
    ════════════════════════════════════════════════════════════════════════════

    All LOW VOLTAGE digital switch inputs are consolidated in J26 (8-position screw terminal).
    (Water Reservoir, Tank Level, Level Probe, Brew Handle - ALL 3.3V LOGIC)

    ⚠️ J26 is for LOW VOLTAGE (3.3V) inputs ONLY!
    ⚠️ 220V relay outputs (K1, K2, K3) use 6.3mm spade terminals (J2, J3, J4)

    J26 LOW-VOLTAGE SWITCH INPUT TERMINAL BLOCK (Phoenix MKDS 1/8-5.08):
    ─────────────────────────────────────────────────────────────────────
    ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
    │  1  │  2  │  3  │  4  │  5  │  6  │  7  │  8  │
    │ S1  │S1-G │ S2  │S2-G │ S3  │ S4  │S4-G │ GND │
    └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
     WtrRes GND  Tank  GND  Probe Brew  GND  Spare

    SWITCH INPUT CIRCUIT (S1, S2, S4):
    ───────────────────────────────────

                            +3.3V
                              │
                         ┌────┴────┐
                         │  10kΩ   │  Pull-up
                         │  R16-18 │  (or use Pico internal)
                         └────┬────┘
                              │
    J26 Pin 1/3/6  ───────────┼───────────────────────┬──────────────► GPIO 2/3/5
    (Switch Signal)           │                       │
                              │                  ┌────┴────┐
                         ┌────┴────┐             │  D12-14 │  ESD
                         │ Switch  │             │  PESD   │  Clamp
                         │  N.O.   │             └────┬────┘
                         └────┬────┘                  │
                              │                       │
    J26 Pin 2/4/7  ───────────┴───────────────────────┴──────────────── GND
    (Switch GND)

    J26 SIGNAL MAPPING (8-pin):
    ───────────────────────────
    Pin 1 (S1)  → GPIO2 (Water Reservoir Switch Signal)
    Pin 2 (S1-G)→ GND   (Water Reservoir Switch Return)
    Pin 3 (S2)  → GPIO3 (Tank Level Sensor Signal)
    Pin 4 (S2-G)→ GND   (Tank Level Sensor Return)
    Pin 5 (S3)  → GPIO4 (Steam Boiler Level Probe - via comparator)
    Pin 6 (S4)  → GPIO5 (Brew Handle Switch Signal)
    Pin 7 (S4-G)→ GND   (Brew Handle Switch Return)
    Pin 8 (GND) → GND   (Common Ground - spare terminal)


    Steam Boiler Level Probe (OPA342 + TLV3201 AC Sensing):
    ─────────────────────────────────────────────────────────

    ⚠️  AC sensing prevents electrolysis and probe corrosion!

    STAGE 1: WIEN BRIDGE OSCILLATOR (~160Hz)
    ────────────────────────────────────────
                          +3.3V
                            │
                       ┌────┴────┐
                       │  100nF  │  C60 (OPA342 decoupling)
                       └────┬────┘
                            │
                     VCC────┤
                            │ U6: OPA342
    ┌────[R91 10kΩ]─────────┤-  (Op-Amp)  ├──┬───────────► AC_OUT
    │                  ┌────┤+             │  │              (to probe)
    │                  │    │     GND      │  │
    │                  │    └──────┬───────┘  │
    │                  │           │          │
    │                  │          ─┴─         │
    │                  │          GND         │
    │                  │                      │
    │             ┌────┴────┐           ┌────┴────┐
    │             │  10kΩ   │           │  10kΩ   │
    │             │  R92    │           │  R93    │
    │             └────┬────┘           └────┬────┘
    │                  │                     │
    │                  ├─────────────────────┤
    │                  │                     │
    │             ┌────┴────┐           ┌────┴────┐
    │             │  100nF  │           │  100nF  │
    │             │  C61    │           │  C62    │
    │             └────┬────┘           └────┬────┘
    │                  │                     │
    │                 ─┴─                   ─┴─
    │                 GND                   GND
    │
    └─────────────────────────────────────────────────────────────────────────┐
                                                                              │
    STAGE 2: PROBE & SIGNAL CONDITIONING                                     │
    ────────────────────────────────────                                      │
                                                                              │
    AC_OUT ───[100Ω R94]───┬────────────────────► J26 Pin 5 (Level Probe S3) │
           (current limit) │                      Screw terminal (LV)         │
                           │                           │                      │
                      ┌────┴────┐                 ┌────┴────┐                 │
                      │   1µF   │                 │  Probe  │                 │
                      │   C64   │                 │   ~~~   │                 │
                      │(coupling)│                 │  Water  │                 │
                      └────┬────┘                 │  Level  │                 │
                           │                      └────┬────┘                 │
                           ▼                           │                      │
                       AC_SENSE                   Boiler Body                 │
                           │                      (Ground via PE)             │
                           │                          ─┴─                     │
                           │                          GND                     │

    STAGE 3: RECTIFIER & COMPARATOR
    ────────────────────────────────
                                 +3.3V
                                   │
                              ┌────┴────┐
                              │  100nF  │  C63 (TLV3201 decoupling)
                              └────┬────┘
                                   │
                              VCC──┤
                                   │ U7: TLV3201
    AC_SENSE ────[10kΩ R95]───┬────┤+  (Comparator) ├───────────► GPIO4
                              │    │                 │
                         ┌────┴────┐    VREF ────────┤-
                         │  100nF  │    (1.65V)      │
                         │   C65   │                 │
                         │(filter) │    └────┬───────┘
                         └────┬────┘         │
                              │             ─┴─
                             ─┴─            GND
                             GND

    VREF Divider:   +3.3V ──[100kΩ R96]──┬──[100kΩ R97]── GND
                                          │
                                          └──► VREF (~1.65V)

    Hysteresis:     GPIO4 ────[1MΩ R98]────► + input (TLV3201)

    Logic States:
    ─────────────
    Water BELOW probe → GPIO4 reads HIGH → Low Water / UNSAFE for heater
    Water AT/ABOVE probe → GPIO4 reads LOW → Level OK

    Component Values:
    ─────────────────
    U6:  OPA342UA (SOIC-8) or OPA2342UA (use one section)
         Alt: OPA207 for lower noise
    U7:  TLV3201AIDBVR (SOT-23-5)
    R91: 10kΩ 1%, 0805 (oscillator feedback)
    R92: 10kΩ 1%, 0805 (Wien bridge)
    R93: 10kΩ 1%, 0805 (Wien bridge)
    R94: 100Ω 5%, 0805 (probe current limit)
    R95: 10kΩ 5%, 0805 (AC bias)
    R96: 100kΩ 1%, 0805 (reference divider)
    R97: 100kΩ 1%, 0805 (reference divider)
    R98: 1MΩ 5%, 0805 (hysteresis)
    C60: 100nF 25V, 0805 (OPA342 decoupling)
    C61: 100nF 25V, 0805 (Wien bridge timing)
    C62: 100nF 25V, 0805 (Wien bridge timing)
    C63: 100nF 25V, 0805 (TLV3201 decoupling)
    C64: 1µF 25V, 0805 (AC coupling to probe)
    C65: 100nF 25V, 0805 (sense filter)

    ⚠️ PCB LAYOUT: GUARD RING REQUIRED
    ───────────────────────────────────
    The trace from J26 Pin 5 (Level Probe) to OPA342 input is HIGH-IMPEDANCE
    and will pick up 50/60Hz mains hum if not properly shielded.

    REQUIRED: Surround probe trace with GND guard ring on both sides.
    Place OPA342 as CLOSE as possible to J26 terminal (< 2cm trace).
    Avoid routing near relay coils or mains traces.

    Benefits of OPA342 + TLV3201:
    ─────────────────────────────
    • Uses commonly available, modern components
    • AC excitation prevents electrolysis and probe corrosion
    • Adjustable threshold via R96/R97 ratio
    • Clean hysteresis via R98
    • Low power consumption (<1mA)

    Other Switches (unchanged):
    ───────────────────────────
    R16-18: 10kΩ 5%, 0805 (or use Pico internal pull-ups)
    D12-14: PESD5V0S1BL, SOD-323
```

---

# Sheet 6: Communication Interfaces

## 6.1 ESP32 Display Module Interface

```
                    ESP32 DISPLAY MODULE INTERFACE
    ════════════════════════════════════════════════════════════════════════════

    J15: JST-XH 8-pin connector (B8B-XH-A) - Includes brew-by-weight support

                                            +5V
                                             │
                                        ┌────┴────┐
                                        │  100µF  │  Bulk cap
                                        │   C13   │  for ESP32
                                        └────┬────┘
                                             │
    J15 Pin 1 (5V) ──────────────────────────┴──────────────────────────────────

    J15 Pin 2 (GND) ────────────────────────────────────────────────────── GND


    UART TX (GPIO0 → ESP32 RX):
    ───────────────────────────
                                    ┌────┐
    GPIO0 ──────────────────────────┤ 33Ω├─────────────────── J15 Pin 3 (TX)
    (UART0_TX)                      │R40 │
                                    └────┘

    UART RX (ESP32 TX → GPIO1):
    ───────────────────────────
                                    ┌────┐
    GPIO1 ◄─────────────────────────┤ 33Ω├─────────────────── J15 Pin 4 (RX)
    (UART0_RX)                      │R41 │
                                    └────┘


    ════════════════════════════════════════════════════════════════════════
    ESP32 → PICO CONTROL (OTA Firmware Updates)
    ════════════════════════════════════════════════════════════════════════

    ESP32 updates ITSELF via WiFi OTA (standard ESP-IDF).
    ESP32 updates PICO via UART + RUN/BOOTSEL control below.
    Pico has no WiFi - relies on ESP32 as firmware update gateway.


    PICO RUN Control (J15 Pin 5 → Pico RUN):
    ─────────────────────────────────────────

                                 +3.3V
                                    │
                               ┌────┴────┐
                               │  10kΩ   │  R71
                               │ Pull-up │  (Pico has internal, add external)
                               └────┬────┘
                                    │
    Pico RUN pin ◄──────────────────┴────────────────── J15 Pin 5 (RUN)
                                                              │
                                                              │
                                                         ESP32 GPIO
                                                      (Open-Drain Output)

    • ESP32 pulls LOW → Pico resets
    • ESP32 releases (high-Z) → Pico runs normally


    PICO BOOTSEL Control (J15 Pin 6 → Pico BOOTSEL):
    ─────────────────────────────────────────────────

                                 +3.3V
                                    │
                               ┌────┴────┐
                               │  10kΩ   │  R72
                               │ Pull-up │  (BOOTSEL normally HIGH)
                               └────┬────┘
                                    │
    Pico BOOTSEL ◄──────────────────┴────────────────── J15 Pin 6 (BOOT)
                                                              │
                                                              │
                                                         ESP32 GPIO
                                                      (Open-Drain Output)

    • ESP32 holds LOW during reset → Pico enters USB bootloader
    • ESP32 releases → Pico boots normally


    WEIGHT_STOP Signal (J15 Pin 7 → GPIO21) - BREW BY WEIGHT:
    ──────────────────────────────────────────────────────────

                                 +3.3V
                                    │
                               ┌────┴────┐
                               │  10kΩ   │  R73
                               │Pull-down│  (Normally LOW - no brew stop)
                               └────┬────┘
                                    │
    GPIO21 ◄────────────────────────┴────────────────── J15 Pin 7 (WEIGHT_STOP)
    (Input)                                                   │
                                                              │
                                                         ESP32 GPIO
                                                       (Push-Pull Output)

    • ESP32 connects to Bluetooth scale (Acaia, Decent, etc.)
    • ESP32 monitors weight during brew
    • When target weight reached → ESP32 sets Pin 7 HIGH
    • Pico detects HIGH on GPIO21 → Stops pump (K2) immediately
    • ESP32 sets Pin 7 LOW → Ready for next brew


    SPARE Signal (J15 Pin 8 → GPIO22):
    ───────────────────────────────────

    GPIO22 ◄────────────────────────────────────────── J15 Pin 8 (SPARE)
    (I/O)                                                    │
                                                             │
                                                        ESP32 GPIO

    • Reserved for future expansion
    • No pull-up/down installed (configure in firmware as needed)
    • Suggested uses: Flow sensor, additional sensor, bidirectional signal


    OTA Update Sequence:
    ─────────────────────
    1. ESP32 downloads Pico firmware via WiFi
    2. ESP32 sends "ENTER_BOOTLOADER" command via UART
    3. Pico custom bootloader acknowledges
    4. ESP32 streams firmware via UART (115200 baud)
    5. Pico writes to flash
    6. ESP32 pulses RUN LOW to reset Pico
    7. Pico boots with new firmware

    Recovery (if Pico firmware corrupted):
    ───────────────────────────────────────
    1. ESP32 holds BOOTSEL LOW
    2. ESP32 pulses RUN LOW then releases
    3. Pico enters hardware bootloader
    4. ESP32 releases BOOTSEL
    5. ESP32 can reflash via custom UART protocol


    J15 Pinout Summary (8-pin):
    ───────────────────────────
    Pin 1: 5V      → ESP32 VIN (power)
    Pin 2: GND     → Common ground
    Pin 3: TX      → ESP32 RX (from Pico GPIO0)
    Pin 4: RX      ← ESP32 TX (to Pico GPIO1)
    Pin 5: RUN     ← ESP32 GPIO (to Pico RUN pin)
    Pin 6: BOOT    ← ESP32 GPIO (to Pico BOOTSEL)
    Pin 7: WGHT    ← ESP32 GPIO (to Pico GPIO21) - Brew-by-weight stop signal
    Pin 8: SPARE   ↔ ESP32 GPIO (to Pico GPIO22) - Reserved for future use

    Component Values:
    ─────────────────
    R40-41: 33Ω 5%, 0805 (UART series protection)
    R71-72: 10kΩ 5%, 0805 (RUN/BOOTSEL pull-ups)
    R73:    10kΩ 5%, 0805 (WEIGHT_STOP pull-down)
    C13:    100µF 10V, Electrolytic (ESP32 power decoupling)
```

## 6.2 Service/Debug Port

```
                        SERVICE/DEBUG PORT
    ════════════════════════════════════════════════════════════════════════════

    J16: 4-pin 2.54mm header

                            +3.3V
                              │
    J16 Pin 1 (3V3) ──────────┴───────────────────────────────────────────────

    J16 Pin 2 (GND) ──────────────────────────────────────────────────── GND

    SERVICE PORT (J16) - Shared with ESP32 on GPIO0/1:
    ─────────────────────────────────────────────────
    ⚠️ DISCONNECT ESP32 (J15) BEFORE USING SERVICE PORT

                                    ┌────┐
    GPIO0 ──────────────────────────┤ 33Ω├──┬──────────────── J15 Pin 3 (ESP32 RX)
    (UART0_TX)                      │R40 │  │
                                    └────┘  └──────────────── J16 Pin 3 (Service TX)

                                    ┌────┐
    GPIO1 ◄─────────────────────────┤ 33Ω├──┬──────────────── J15 Pin 4 (ESP32 TX)
    (UART0_RX)                      │R41 │  │
                                    └────┘  └──────────────── J16 Pin 4 (Service RX)

    I2C ACCESSORY PORT (J23) - GPIO8/9:
    ────────────────────────────────────
                  3.3V
                   │
            ┌──────┴──────┐
         [4.7kΩ]       [4.7kΩ]
          R50           R51
            │             │
    GPIO8 ──┴─────────────┼─────────────── J23 Pin 3 (SDA)
    (I2C0_SDA)            │
    GPIO9 ────────────────┴─────────────── J23 Pin 4 (SCL)
    (I2C0_SCL)

    Configuration:
    ──────────────
    Default baud rate: 115200, 8N1

    Silkscreen: "3V3 GND TX RX" or "DEBUG"
```

---

# Sheet 7: User Interface

## 7.1 Status LED

```
                            STATUS LED
    ════════════════════════════════════════════════════════════════════════════

                                    ┌────────┐
    GPIO15 ─────────────────────────┤  330Ω  ├──────────────┬────────────────
                                    │  R50   │              │
                                    └────────┘         ┌────┴────┐
                                                       │   LED   │
                                                       │  Green  │
                                                       │  LED7   │
                                                       │  0805   │
                                                       └────┬────┘
                                                            │
                                                           ─┴─
                                                           GND

    Current: (3.3V - 2.0V) / 330Ω ≈ 4mA (clearly visible)

    NOTE: Green LED chosen over blue because:
    • Blue LEDs have Vf=3.0-3.4V, leaving only 0-0.3V margin with 3.3V supply
    • Green LEDs have Vf=1.8-2.2V, providing reliable ~4mA current
    • Green matches relay indicator LEDs for consistency

    Component Values:
    ─────────────────
    R50:  330Ω 5%, 0805
    LED7: Green 0805, Vf~2.0V
```

## 7.2 Buzzer

```
                            BUZZER (Passive)
    ════════════════════════════════════════════════════════════════════════════

    Using passive buzzer with PWM for variable tones:

                                    ┌────────────┐
    GPIO19 ─────────────────────────┤   100Ω     ├──────────────┐
    (PWM)                           │    R51     │              │
                                    └────────────┘              │
                                                           ┌────┴────┐
                                                           │ Passive │
                                                           │ Buzzer  │
                                                           │   BZ1   │
                                                           └────┬────┘
                                                                │
                                                               ─┴─
                                                               GND

    Component Values:
    ─────────────────
    R51: 100Ω 5%, 0805
    BZ1: CUI CEM-1203(42), 12mm, Passive Piezo

    PWM Frequencies:
    ────────────────
    2000-2500 Hz: Ready/confirmation beeps
    1000-1500 Hz: Warnings
    500 Hz:       Error
```

---

# Sheet 8: Power Metering (PZEM-004T External)

## 8.1 PZEM-004T v3.0 External Power Meter

```
                    PZEM-004T v3.0 EXTERNAL POWER METERING
    ════════════════════════════════════════════════════════════════════════════

    ✅ NO HIGH CURRENT THROUGH CONTROL PCB!
    ✅ Power metering via external CT clamp (non-invasive)
    ✅ UART interface (9600 baud Modbus-RTU)

    ═══════════════════════════════════════════════════════════════════════════
    PZEM-004T-100A-D-P DIRECT MOUNT (Plugs into PCB via two female headers)
    ═══════════════════════════════════════════════════════════════════════════

    PZEM Module (user-supplied, plugs directly into PCB):
    ─────────────────────────────────────────────────────

               J24 (HV+CT)                      J17 (LV/UART)
               Left Side                        Right Side
                  │                                │
       ┌──────────┴────────────────────────────────┴──────────┐
       │               PZEM-004T-100A-D-P                     │
       │   ┌────┐                                 ┌─────┐     │
       │   │▼▼▼▼│  ← Pin headers                 │▼▼▼▼▼│     │
       └───┴────┴─────────────────────────────────┴─────┴─────┘
            ║║║║                                    ║║║║║
   ═════════╩╩╩╩════════════════════════════════════╩╩╩╩╩═════  ← Control PCB
           J24                                       J17
        (4-pin)                                    (5-pin)

    CT CLAMP CONNECTION:
    ────────────────────
    Machine Live wire ──────────────────────────────► To all loads
                          │
                     ┌────┴────┐
                     │   CT    │  ← Clamp around Live wire (non-invasive)
                     │  Clamp  │    User-supplied with PZEM module
                     └────┬────┘
                          │ (2-wire cable)
                          └─────────────────────► J26-17/18 (CT+/CT-)
                                                       │
                                                       └──► Routed to J24 CT+/CT-

    ═══════════════════════════════════════════════════════════════════════════

    PZEM-004T Specifications:
    ─────────────────────────
    • Voltage Range: 80-260V AC
    • Current Range: 0-100A (via CT clamp)
    • Accuracy: ±0.5% (pre-calibrated)
    • Interface: UART 9600 baud, Modbus-RTU
    • Measurements: V, I, P, E (energy), PF, Frequency

    Control PCB Power Distribution (Simplified):
    ─────────────────────────────────────────────
    J1-L (Live) ──►[FUSE F1: 10A]──► L_FUSED bus
                                        │
                                        ├──► K1 COM (Water LED → J2 spade, 220V)
                                        ├──► K2 COM (Pump ~5A → J3 spade, 220V)
                                        └──► K3 COM (3-Way Valve → J4 spade, 220V)

    J26 LOW-VOLTAGE INPUTS: 8-pos screw terminal for digital switch
                            inputs (S1-S4) ONLY. 3.3V logic level.

    Note: Heaters connect to external SSRs via machine's existing wiring.
          SSR control signals only come from PCB (J26 Pin 19-22).
          NO heater current flows through control PCB.

    J17 Pinout (UART - Right side of PZEM):
    ───────────────────────────────────────
    Pin 1: 5V (to PZEM VCC)
    Pin 2: RX (GPIO7 ← PZEM TX) - 33Ω series resistor
    Pin 3: TX (GPIO6 → PZEM RX) - 33Ω series resistor
    Pin 4: CF (not connected - pulse output)
    Pin 5: GND (to PZEM GND)

    J24 Pinout (HV+CT - Left side of PZEM):
    ───────────────────────────────────────
    Pin 1: CT+ (routed to J26-17)
    Pin 2: CT- (routed to J26-18)
    Pin 3: N (from N bus)
    Pin 4: L (from L_FUSED bus) ⚠️ 220V!

    J26-17/18 (CT Clamp connections - part of unified terminal):
    ─────────────────────────────────────
    Pin 1: CT+ (from J24 Pin 1)
    Pin 2: CT- (from J24 Pin 2)

    Component Values:
    ─────────────────
    J17:     Female Header 1×5 2.54mm
    J24:     Female Header 1×4 2.54mm (⚠️ HV!)
    J26:     Unified Screw Terminal 24-pos 5.08mm (Phoenix MKDS 1/24-5.08)
    R44-45:  33Ω 5%, 0805 (UART series protection)

    ⚠️ CRITICAL: J24 MILLING SLOT REQUIREMENT
    ─────────────────────────────────────────
    J24 carries Mains L and N on adjacent pins (2.54mm pitch).
    Mill a 1mm wide slot between Pin 3 (N) and Pin 4 (L) for safety.

    J24 Top View:
    ┌─────────────────────────────┐
    │ Pin1  Pin2  Pin3 ║║ Pin4   │
    │  CT+   CT-    N  ║║   L    │  ← 1mm slot between N and L
    └─────────────────────────────┘
```

---

# Sheet 9: Protection & Filtering

## 9.1 5V Rail Protection

```
                        5V RAIL TRANSIENT PROTECTION
    ════════════════════════════════════════════════════════════════════════════

    From AC/DC                                              To 5V Distribution
    Module                                                  (Pico, Relays, etc.)
        │                                                           │
        │      ┌────────────┐                                       │
        └──────┤  Ferrite   ├───────────┬───────────────────────────┘
               │   Bead     │           │
               │   FB2      │           │
               └────────────┘           │
                                   ┌────┴────┐    ┌────┴────┐
                                   │  D20    │    │  C2     │
                                   │  TVS    │    │  100µF  │
                                   │ SMBJ5.0A│    │  16V    │
                                   └────┬────┘    └────┬────┘
                                        │              │
                                       ─┴─            ─┴─
                                       GND            GND

    Component Values:
    ─────────────────
    FB2: Ferrite Bead, 600Ω @ 100MHz, 0805 (optional, for noise)
    D20: SMBJ5.0A, SMB package, 5V TVS diode (absorbs transients)
    C2:  100µF 16V Electrolytic (bulk decoupling)
```

---

# Net List Summary

```
                            NET LIST SUMMARY
    ════════════════════════════════════════════════════════════════════════════

    POWER NETS:
    ───────────
    +5V          → Pico VSYS, Relay coils, SSR drivers, ESP32 (J15-1), LED anodes
    +3.3V        → Pico 3V3, Digital I/O, pull-ups
    +3.3V_ANALOG → ADC reference, NTC dividers, MAX31855
    GND          → System ground (isolated from mains PE)

    HIGH VOLTAGE NETS (⚠️ MAINS):
    ─────────────────────────────
    L_IN         → J1-L (Mains Live Input)
    L_FUSED      → After F1 fuse (10A) - to relay COMs only
    N            → J1-N (Mains Neutral)
    PE           → J1-PE (Protective Earth, to chassis)

    NOTE: With PZEM-004T external power metering, NO heater current
    flows through the control PCB. Heaters connect directly to
    external SSRs via machine's existing wiring.

    RELAY OUTPUT NETS:
    ──────────────────
    K1_COM       → Relay K1 Common (internal to L_FUSED)
    K2_COM       → Relay K2 Common (internal to L_FUSED)
    K3_COM       → Relay K3 Common (internal to L_FUSED)

    GPIO NETS:
    ──────────
    GPIO0  → ESP32_TX (J15-3)
    GPIO1  → ESP32_RX (J15-4)
    GPIO2  → WATER_SW (J26-1, S1)
    GPIO3  → TANK_LVL (J26-3, S2)
    GPIO4  → STEAM_LVL (J26-5, S3 via comparator)
    GPIO5  → BREW_SW (J26-6, S4)
    GPIO6  → PZEM_TX (UART to PZEM RX, J17-3)
    GPIO7  → PZEM_RX (UART from PZEM TX, J17-4)
    GPIO8  → I2C0_SDA (J23-3, with 4.7kΩ pull-up)
    GPIO9  → I2C0_SCL (J23-4, with 4.7kΩ pull-up)
    GPIO10 → K1_DRIVE (relay coil, output to J2 spade - 220V)
    GPIO11 → K2_DRIVE (relay coil, output to J3 spade - 220V)
    GPIO12 → K3_DRIVE (relay coil, output to J4 spade - 220V)
    GPIO13 → SSR1_DRIVE
    GPIO14 → SSR2_DRIVE
    GPIO15 → STATUS_LED
    GPIO16 → SPI_MISO (MAX31855 DO)
    GPIO17 → SPI_CS (MAX31855 CS)
    GPIO18 → SPI_SCK (MAX31855 CLK)
    GPIO19 → BUZZER
    GPIO20 → TP1 (spare - test point for future expansion)
    GPIO21 → WEIGHT_STOP (J15-7, ESP32 brew-by-weight signal)
    GPIO22 → SPARE (J15-8, reserved for future)
    GPIO26 → ADC0_BREW_NTC (J26-8/9)
    GPIO27 → ADC1_STEAM_NTC (J26-10/11)
    GPIO28 → ADC2_PRESSURE (from J26-16 voltage divider)
    RUN    → SW1_RESET
    BOOTSEL→ SW2_BOOT

    220V AC RELAY OUTPUTS (6.3mm Spade Terminals):
    ──────────────────────────────────────────────
    J2-NO  → Relay K1 N.O. output (Water LED, 220V ≤100mA)
    J3-NO  → Relay K2 N.O. output (Pump, 220V 5A)
    J4-NO  → Relay K3 N.O. output (Solenoid, 220V ~0.5A)

    J26 LOW-VOLTAGE SWITCH INPUT TERMINAL BLOCK (8-pos, 3.3V):
    ──────────────────────────────────────────────────────────
    J26-1  (S1)    → Water Reservoir Switch Signal → GPIO2
    J26-2  (S1-G)  → Water Reservoir Switch GND
    J26-3  (S2)    → Tank Level Sensor Signal → GPIO3
    J26-4  (S2-G)  → Tank Level Sensor GND
    J26-5  (S3)    → Steam Boiler Level Probe → GPIO4 (via comparator)
    J26-6  (S4)    → Brew Handle Switch Signal → GPIO5
    J26-7  (S4-G)  → Brew Handle Switch GND
    J26-8  (GND)   → Common Ground (spare)

    ANALOG SENSOR CONNECTOR NETS:
    ─────────────────────────────
    J26-8/9 (Brew NTC)      → NTC_BREW+ to R1, NTC_BREW- to GND
    J26-10/11 (Steam NTC)   → NTC_STEAM+ to R2, NTC_STEAM- to GND
    J26-12/13 (Thermocouple)→ TC+ to U4 pin 1, TC- to U4 pin 2
    J26-14/15/16 (Pressure) → 5V, GND, SIGNAL to R3/R4 divider
    J26-17/18 (CT Clamp)    → CT+/CT- from PZEM J24
    J26-19/20 (SSR1)        → +5V, SSR1_NEG (Brew heater control)
    J26-21/22 (SSR2)        → +5V, SSR2_NEG (Steam heater control)

    SOLDER BRIDGE:
    ──────────────
```

---

_End of Schematic Reference Document_
