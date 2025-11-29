# ESP32 Display UI Design

> **Status:** ✅ Implemented  
> **Last Updated:** 2025-11-29  
> **Display:** 480×480 Round IPS with Rotary Encoder

## Overview

The BrewOS UI runs on a 2.1" round display ([VIEWE UEDX48480021-MD80E](https://github.com/VIEWESMART/UEDX48480021-MD80ESP32_2.1inch-Knob)) with **rotary encoder + button only** (no touch).

### Design Priorities
- **Glanceability** - Key info visible at a glance
- **Simplicity** - Minimal steps for common tasks
- **Context-awareness** - UI adapts to machine state

### Encoder-First Principles

> ⚠️ **No Touch Screen** - This display has knob + click only. Design accordingly.

| Principle | Description |
|-----------|-------------|
| **One action per screen** | Click does the obvious thing - no button hunting |
| **Whole screen is clickable** | No visual buttons needed for primary action |
| **Rotation = adjustment** | Rotate to change values or cycle options |
| **Focus is implicit** | Current screen context determines what click does |
| **No multi-select** | Can't tap multiple areas - use sequential screens |

**Anti-patterns to avoid:**
- ❌ Multiple tappable buttons on one screen
- ❌ "Save" / "Cancel" button pairs
- ❌ Sliders or drag controls
- ❌ Tap-to-select from a grid

---

## Navigation Model

### Input Controls

| Input | Action |
|-------|--------|
| **Rotate CW** | Next item / Increase value |
| **Rotate CCW** | Previous item / Decrease value |
| **Short Press** | Select / Confirm |
| **Long Press (2s)** | Back / Cancel / Home |
| **Double Press** | Quick action (context-dependent) |

### Screen Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  [Setup] ──────────────────────────────────────────────┐    │
│     │                                                  │    │
│     ▼                                                  │    │
│  [Idle] ◄──────────────────────────────────────────────┤    │
│     │                                                  │    │
│     ├── Rotate ──► [Main/Home] ◄── Long Press ─────────┤    │
│     │                  │                               │    │
│     │                  ├── Rotate ──► [Settings]       │    │
│     │                  │                               │    │
│     │                  └── (Auto) ──► [Brewing]        │    │
│     │                                    │             │    │
│     │                                    └── (Auto) ───┘    │
│     │                                                       │
│     └── Press ──► Turn On Machine                           │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## Screen Definitions

### 1. Setup Screen (First Boot / No WiFi)

**Purpose:** Guide user through initial WiFi configuration.

**Trigger:** 
- First boot (no WiFi configured)
- WiFi connection failed
- Manual entry via Settings

**Layout:**
```
         ┌─────────────────┐
        ╱                   ╲
       │     📶 WiFi Setup    │
       │                      │
       │   Connect to:        │
       │   ┌──────────────┐   │
       │   │ BrewOS-XXXX  │   │
       │   └──────────────┘   │
       │                      │
       │   Password:          │
       │   ┌──────────────┐   │
       │   │  brewos123   │   │
       │   └──────────────┘   │
       │                      │
       │   Then open:         │
       │   192.168.4.1        │
       │                      │
        ╲   ○ ○ ○ ● ○ ○ ○   ╱
         └─────────────────┘
```

**Elements:**
- WiFi icon + "WiFi Setup" title
- AP SSID (device-specific)
- AP Password
- Configuration URL
- QR code option (rotate to show)

**Interactions:**
- Rotate: Show QR code for WiFi connection
- Press: Refresh / Check connection status
- Long Press: Skip (if WiFi already configured)

---

### 2. Idle Screen (Machine Off)

**Purpose:** Machine is powered but heating is off. User can turn on or adjust settings.

**Trigger:**
- Machine in IDLE state
- User selected "Turn Off" from menu

**Layout:**
```
         ┌─────────────────┐
        ╱                   ╲
       │                      │
       │        ☕             │
       │                      │
       │      BrewOS          │
       │                      │
       │                      │
       │   Click to Turn On   │
       │                      │
       │                      │
       │   ◄ Sequential ►     │  ← Rotate to change
       │                      │
        ╲                    ╱
         └─────────────────┘
```

**Elements:**
- BrewOS logo/icon (centered)
- Action hint text
- Heating strategy selector (rotate to change)
- Subtle arrows indicating rotation

**Interactions:**
- Click: Turn on machine (start heating)
- Rotate: Cycle heating strategies
- Long Press: Go to Settings

**Heating Strategies:**
| Strategy | Display Name | Description |
|----------|--------------|-------------|
| `HEAT_BREW_ONLY` | Brew Only | Steam heater disabled |
| `HEAT_SEQUENTIAL` | Sequential | Heat one boiler at a time |
| `HEAT_STEAM_PRIORITY` | Steam Priority | Steam first, then brew |
| `HEAT_PARALLEL` | Parallel | Both heaters (high power) |
| `HEAT_SMART_STAGGER` | Smart | Intelligent time-sharing |

---

### 3. Home Screen (Main)

**Purpose:** Primary screen showing machine status at a glance.

**Trigger:**
- Machine turned on
- Long press from any screen
- Brewing finished

**Layout:**
```
         ┌─────────────────┐
        ╱    ╭─────────╮    ╲
       │   ╱  93.5°C    ╲   │
       │  │    BREW      │  │    ← Brew temp arc
       │   ╲  ━━━━━━━━  ╱   │
       │    ╰─────────╯     │
       │                    │
       │      9.2 bar       │    ← Pressure
       │                    │
       │   ╭─────────╮      │
       │  │  145.2°C  │     │    ← Steam temp (smaller)
       │   ╰─────────╯      │
       │                    │
       │  ● Ready  📶 🔗    │    ← Status bar
        ╲                  ╱
         └─────────────────┘
```

**Elements:**
- **Brew Temperature Arc** (dominant)
  - Current temp (large)
  - Setpoint indicator on arc
  - Color: Blue when cold → Orange when heating → Green when ready
- **Pressure** (center)
- **Steam Temperature** (secondary)
  - Smaller display below
- **Status Bar** (bottom)
  - Machine state (Ready/Heating/etc.)
  - WiFi indicator
  - Pico connection indicator
  - Scale connection indicator (if paired)

**Interactions:**
- Rotate: Switch focus between Brew ↔ Steam temp display
- Click: Adjust focused temperature setpoint
- Long Press: Turn off machine (go to Idle)
- Double Click: Tare scale (if connected)

**State-Based Colors:**
| State | Arc Color | Status Text |
|-------|-----------|-------------|
| HEATING | Orange | "Heating..." |
| READY | Green | "Ready" |
| BREWING | Blue pulse | "Brewing" |
| STEAMING | Red | "Steaming" |
| FAULT | Red flash | "ALARM" |
| WATER LOW | Yellow | "Fill Water" |

---

### 4. Brewing Screen

**Purpose:** Active brewing display with timer and weight.

**Trigger:**
- Brewing detected (lever pulled)
- Auto-transitions from Home

**Layout (with scale):**
```
         ┌─────────────────┐
        ╱                   ╲
       │                      │
       │       00:24          │    ← Shot timer (large)
       │                      │
       │   ┌──────────────┐   │
       │   │              │   │
       │   │   18.5g      │   │    ← Current weight
       │   │   ━━━━━━━    │   │    ← Progress bar
       │   │   / 36.0g    │   │    ← Target weight
       │   │              │   │
       │   └──────────────┘   │
       │                      │
       │     2.1 ml/s         │    ← Flow rate
       │     9.1 bar          │    ← Pressure
       │                      │
        ╲   Brewing...       ╱
         └─────────────────┘
```

**Layout (no scale):**
```
         ┌─────────────────┐
        ╱                   ╲
       │                      │
       │                      │
       │       00:24          │    ← Shot timer (very large)
       │                      │
       │                      │
       │      9.1 bar         │    ← Pressure
       │                      │
       │      93.2°C          │    ← Brew temp
       │                      │
       │                      │
        ╲   Brewing...       ╱
         └─────────────────┘
```

**Elements:**
- **Shot Timer** (dominant) - MM:SS format
- **Weight Display** (if scale connected)
  - Current weight
  - Target weight
  - Progress arc/bar
- **Flow Rate** (if scale connected) - ml/s
- **Pressure** - Current bar
- **Brew Temperature** (without scale)

**Interactions:**
- Rotate: Adjust target weight (if scale)
- Press: Nothing (avoid accidental actions)
- Double Press: Tare scale
- Long Press: Emergency stop signal to Pico

**Auto-Stop:**
- When weight reaches target, signal Pico via `WEIGHT_STOP_PIN`
- Display "Target Reached!" briefly
- Note: Physical lever controls actual brew stop

---

### 5. Shot Complete Screen

**Purpose:** Show shot summary after brewing ends.

**Trigger:**
- Brewing stops (lever returned)
- Auto-dismisses after 10 seconds

**Layout:**
```
         ┌─────────────────┐
        ╱                   ╲
       │                      │
       │     Shot Complete    │
       │         ✓            │
       │                      │
       │       00:28          │    ← Total time
       │       35.2g          │    ← Final weight
       │       1.4 ml/s       │    ← Avg flow
       │                      │
       │   Click to Save      │
       │   (auto-dismiss 10s) │
       │                      │
        ╲                    ╱
         └─────────────────┘
```

**Elements:**
- Completion checkmark animation
- Total brew time (large)
- Final weight (if scale)
- Average flow rate
- Action hint

**Interactions:**
- Click: Save shot to history → Home
- Rotate: View ratio, peak pressure, etc.
- Long Press: Dismiss without saving
- Auto: Returns to Home after 10s if no input

---

### 6. Settings Menu

**Purpose:** Configuration and adjustments.

**Trigger:**
- Rotate from Home screen
- Long press from Idle

**Layout:**
```
         ┌─────────────────┐
        ╱                   ╲
       │      ⚙️ Settings     │
       │                      │
       │   ▸ Temperatures     │  ← Highlighted (rotate)
       │     Heating Mode     │
       │     Scale Setup      │
       │     WiFi             │
       │     Display          │
       │     Machine Info     │
       │                      │
       │                      │
       │   Click to Enter     │
        ╲                    ╱
         └─────────────────┘
```

**Menu Items:**

| Item | Description |
|------|-------------|
| **Temperatures** | Brew/Steam setpoints, max limits |
| **Heating Mode** | Select heating strategy |
| **Scale Setup** | BLE scale pairing |
| **Brew by Weight** | Target weight, auto-stop |
| **WiFi** | Network settings, reconnect |
| **MQTT** | Connection status |
| **Display** | Brightness, sleep timeout |
| **Machine Info** | Versions, stats |
| **Reset** | Factory reset options |

**Interactions:**
- Rotate: Scroll through menu
- Press: Enter selected item
- Long Press: Back to Home

---

### 7. Temperature Settings

**Purpose:** Adjust brew and steam temperature setpoints.

**Layout:**
```
         ┌─────────────────┐
        ╱                   ╲
       │    🌡️ Temperatures   │
       │                      │
       │   BREW SETPOINT      │
       │   ┌──────────────┐   │
       │   │ ◄  94.0°C  ► │   │  ← Rotate to adjust
       │   └──────────────┘   │
       │   (Max: 105°C)       │
       │                      │
       │   STEAM SETPOINT     │
       │   ┌──────────────┐   │
       │   │   145.0°C    │   │  ← Dimmed until selected
       │   └──────────────┘   │
       │   (Max: 155°C)       │
       │                      │
        ╲   Click to Save    ╱
         └─────────────────┘
```

**Elements:**
- Brew temperature (active, shows ◄ ► arrows)
- Steam temperature (dimmed until focused)
- Max limits displayed
- Hint text at bottom

**Interactions:**
- Rotate: Adjust current value (0.5°C steps)
- Click: Save & move to next field (or exit if last)
- Long Press: Cancel changes and go back

---

### 8. Scale Pairing Screen

**Purpose:** Connect to a BLE coffee scale.

**Trigger:**
- Selected from Settings
- First use of brew-by-weight

**Layout (Scanning):**
```
         ┌─────────────────┐
        ╱                   ╲
       │    ⚖️ Scale Setup    │
       │                      │
       │                      │
       │      Scanning...     │
       │         ◠            │
       │                      │
       │   Found:             │
       │   ┌──────────────┐   │
       │   │ Lunar ABC123 │   │
       │   └──────────────┘   │
       │   ┌──────────────┐   │
       │   │ Pearl XYZ789 │   │
       │   └──────────────┘   │
       │                      │
        ╲                    ╱
         └─────────────────┘
```

**Layout (Connected):**
```
         ┌─────────────────┐
        ╱                   ╲
       │    ⚖️ Scale Setup    │
       │                      │
       │   Connected ✓        │
       │   Lunar ABC123       │
       │                      │
       │      ┌───────┐       │
       │      │ 0.0g  │       │  ← Live weight
       │      └───────┘       │
       │                      │
       │   Click to Tare      │
       │                      │
       │   ◄ Disconnect ►     │  ← Rotate to select
       │                      │
        ╲                    ╱
         └─────────────────┘
```

**Interactions:**
- Click: Tare scale (when on Tare) / Disconnect (when selected)
- Rotate: Cycle between Tare ↔ Disconnect options
- Long Press: Back to Settings

---

### 9. Brew by Weight Settings

**Purpose:** Configure auto-stop weight and ratio.

**Layout:**
```
         ┌─────────────────┐
        ╱                   ╲
       │  ⚖️ Brew by Weight   │
       │                      │
       │   TARGET WEIGHT      │
       │   ┌──────────────┐   │
       │   │ ◄  36.0g   ► │   │  ← Rotate to adjust
       │   └──────────────┘   │
       │                      │
       │   DOSE (optional)    │
       │   ┌──────────────┐   │
       │   │    18.0g     │   │
       │   └──────────────┘   │
       │                      │
       │   Ratio: 1:2.0       │
       │                      │
       │   ○ Auto-stop: ON    │  ← Rotate cycles ON/OFF
       │                      │
        ╲   Click to Save    ╱
         └─────────────────┘
```

**Elements:**
- Target weight (rotate to adjust)
- Dose weight (optional, click to focus)
- Calculated ratio (auto-updates)
- Auto-stop toggle (rotate to toggle)

**Interactions:**
- Rotate: Adjust current value (0.5g steps) or toggle
- Click: Move to next field / Save when done
- Long Press: Cancel and go back

---

### 10. Alarm Screen

**Purpose:** Display critical alerts requiring attention.

**Trigger:**
- Pico sends alarm message
- Safety condition detected

**Layout:**
```
         ┌─────────────────┐
        ╱                   ╲
       │                      │
       │     ⚠️ ALARM ⚠️      │
       │                      │
       │   ┌──────────────┐   │
       │   │              │   │
       │   │  OVERTEMP    │   │
       │   │  Brew: 115°C │   │
       │   │              │   │
       │   └──────────────┘   │
       │                      │
       │   Heaters disabled   │
       │                      │
       │   Press to           │
       │   acknowledge        │
       │                      │
        ╲                    ╱
         └─────────────────┘
```

**Elements:**
- Alarm icon (flashing)
- Alarm type
- Details
- Acknowledgment prompt

**Alarm Types:**
| Alarm | Description |
|-------|-------------|
| `OVERTEMP` | Temperature exceeds max |
| `SENSOR_FAIL` | Sensor disconnected/failed |
| `WATER_EMPTY` | Water tank empty |
| `COMM_LOST` | Pico communication lost |
| `HEATER_FAIL` | Heater not responding |

**Interactions:**
- Press: Acknowledge alarm
- Long Press: Emergency shutdown

---

## Screen Transitions

### State Machine

```
                    ┌─────────┐
                    │  BOOT   │
                    └────┬────┘
                         │
            ┌────────────┴────────────┐
            │                         │
            ▼                         ▼
     ┌──────────┐              ┌──────────┐
     │  SETUP   │──WiFi OK───►│   IDLE   │
     └──────────┘              └────┬─────┘
            ▲                       │
            │                  Turn On
            │                       │
            │                       ▼
            │                ┌──────────┐
            │                │   HOME   │◄───────┐
            │                └────┬─────┘        │
            │                     │              │
            │         ┌───────────┼───────────┐  │
            │         │           │           │  │
            │         ▼           ▼           ▼  │
            │   ┌──────────┐ ┌────────┐ ┌────────┴─┐
            │   │ SETTINGS │ │BREWING │ │  ALARM   │
            │   └──────────┘ └───┬────┘ └──────────┘
            │                    │
            │                    ▼
            │              ┌──────────┐
            │              │ COMPLETE │
            │              └──────────┘
            │
            └──────────WiFi Lost─────────────────
```

### Transition Triggers

| From | To | Trigger |
|------|-----|---------|
| Boot | Setup | No WiFi configured |
| Boot | Idle | WiFi OK, machine off |
| Boot | Home | WiFi OK, machine on |
| Setup | Idle | WiFi connected |
| Idle | Home | User turns on |
| Home | Idle | User turns off (long press) |
| Home | Settings | Rotate + select |
| Home | Brewing | Brew detected |
| Brewing | Complete | Brew stopped |
| Complete | Home | Auto (10s) or dismiss |
| Any | Alarm | Alarm received |
| Alarm | Previous | Acknowledged |

---

## Visual Design

### Color Palette

| Element | Color | Hex |
|---------|-------|-----|
| Background | Dark brown | `#1A1512` |
| Card/Panel | Medium brown | `#2D2420` |
| Accent (warm) | Amber | `#D4A574` |
| Brew temp | Blue | `#3B82F6` |
| Steam temp | Red | `#EF4444` |
| Ready/OK | Green | `#22C55E` |
| Warning | Yellow | `#F59E0B` |
| Error/Alarm | Red | `#DC2626` |
| Text primary | White | `#FAFAFA` |
| Text muted | Gray | `#9CA3AF` |

### Typography

| Use | Font | Size |
|-----|------|------|
| Temperature (large) | Montserrat Bold | 48px |
| Timer | Montserrat Bold | 48px |
| Values | Montserrat Medium | 24px |
| Labels | Montserrat Regular | 16px |
| Status | Montserrat Regular | 14px |

### Animations

| Animation | Duration | Use |
|-----------|----------|-----|
| Screen transition | 200ms | Slide/fade between screens |
| Value change | 150ms | Number animations |
| Pulse | 1000ms | Heating indicator |
| Flash | 500ms | Alarm state |
| Arc fill | 300ms | Temperature progress |

---

## Implementation Priority

### Phase 1: Core Screens ✅
```
[x] Display driver and LVGL setup
[x] Home screen with temperature arcs
[x] Idle screen with turn on/off
[x] Basic brewing screen (timer + weight)
```

### Phase 2: Settings ✅
```
[x] Settings menu navigation
[x] Temperature adjustment screen (placeholder)
[x] Heating mode selection (in Idle screen)
[ ] Display settings (backlight, timeout)
```

### Phase 3: Scale Integration
```
[x] Scale pairing screen (placeholder)
[x] Weight display in brewing
[x] Target weight adjustment
[ ] BLE scale driver
[ ] Auto-stop functionality (WEIGHT_STOP_PIN)
```

### Phase 4: Polish
```
[x] Setup screen with WiFi info
[x] Shot complete summary
[x] Alarm handling
[x] Screen transitions (fade animations)
[ ] QR code for WiFi setup
[ ] Advanced animations
```

---

## Related Documentation

- [Implementation Plan](Implementation_Plan.md) - Overall ESP32 status
- [Machine Configurations](../pico/Machine_Configurations.md) - Heating strategies
- [MQTT Integration](integrations/MQTT.md) - Status publishing

