# Power Metering Firmware Integration

## Overview

The control board supports universal external power metering modules via a configurable UART/RS485 interface. This document covers firmware implementation details for integrating various power meters.

---

## Hardware Interface

| GPIO   | Function       | Notes                          |
| ------ | -------------- | ------------------------------ |
| GPIO6  | UART1 TX       | To meter RX (via 33Ω)          |
| GPIO7  | UART1 RX       | From meter TX (via 33Ω)        |
| GPIO20 | RS485 DE/RE    | HIGH=Transmit, LOW=Receive     |

**Connector:** J17 (JST-XH 6-pin) - See hardware specification for pinout.

---

## Supported Modules

| Module          | Default Baud | Protocol    | Interface | Slave Addr |
| --------------- | ------------ | ----------- | --------- | ---------- |
| PZEM-004T V3    | 9600         | Modbus RTU  | TTL UART  | 0xF8       |
| JSY-MK-163T     | 4800         | Modbus RTU  | TTL UART  | 0x01       |
| JSY-MK-194T     | 4800         | Modbus RTU  | TTL UART  | 0x01       |
| Eastron SDM120  | 2400         | Modbus RTU  | RS485     | 0x01       |
| Eastron SDM230  | 9600         | Modbus RTU  | RS485     | 0x01       |

---

## Software Architecture

### Unified Data Model

All meter drivers normalize readings to a common structure:

```c
typedef struct {
    float voltage;        // Volts (RMS)
    float current;        // Amps (RMS)
    float power;          // Watts (Active)
    float energy_import;  // kWh (from grid)
    float energy_export;  // kWh (to grid - for solar)
    float frequency;      // Hz
    float power_factor;   // 0.0 - 1.0
} MeterReading;
```

### Register Map Configuration

Meters are supported via data-driven register maps:

```c
typedef struct {
    uint8_t  slave_addr;
    uint16_t voltage_reg;     float voltage_scale;
    uint16_t current_reg;     float current_scale;
    uint16_t power_reg;       float power_scale;
    uint16_t energy_reg;      float energy_scale;
    uint16_t frequency_reg;   float frequency_scale;
    uint16_t pf_reg;          float pf_scale;
} ModbusRegisterMap;
```

### PZEM-004T Configuration

```c
const ModbusRegisterMap PZEM_MAP = {
    .slave_addr = 0xF8,
    .voltage_reg = 0x0000, .voltage_scale = 0.1,
    .current_reg = 0x0001, .current_scale = 0.001,
    .power_reg   = 0x0003, .power_scale   = 0.1,
    .energy_reg  = 0x0005, .energy_scale  = 1.0,
    .frequency_reg = 0x0007, .frequency_scale = 0.1,
    .pf_reg      = 0x0008, .pf_scale      = 0.01,
};
```

### JSY-MK-163T Configuration

```c
const ModbusRegisterMap JSY_MAP = {
    .slave_addr = 0x01,
    .voltage_reg = 0x0048, .voltage_scale = 0.0001,
    .current_reg = 0x0049, .current_scale = 0.0001,
    .power_reg   = 0x004A, .power_scale   = 0.0001,
    .energy_reg  = 0x004C, .energy_scale  = 0.0001,
    .frequency_reg = 0x004E, .frequency_scale = 0.01,
    .pf_reg      = 0x004F, .pf_scale      = 0.001,
};
```

---

## Auto-Detection Algorithm

On startup, firmware scans for connected meters:

```c
const int BAUD_RATES[] = {9600, 4800, 2400, 19200};
const ModbusRegisterMap* METER_MAPS[] = {&PZEM_MAP, &JSY_MAP, &EASTRON_MAP};

bool autoDetectMeter() {
    for (int baud : BAUD_RATES) {
        uart_set_baudrate(METER_UART, baud);
        delay_ms(50);
        
        for (auto map : METER_MAPS) {
            if (tryModbusRead(map)) {
                saveMeterConfig(baud, map);
                return true;
            }
        }
    }
    return false; // No meter found
}
```

---

## RS485 Direction Control

For RS485 meters (Eastron), GPIO20 controls the MAX3485 transceiver:

```c
void rs485_transmit_enable() {
    gpio_put(PIN_RS485_DE, 1);  // HIGH = Transmit
}

void rs485_receive_enable() {
    gpio_put(PIN_RS485_DE, 0);  // LOW = Receive
}

void modbus_send(uint8_t* data, size_t len) {
    rs485_transmit_enable();
    uart_write_blocking(METER_UART, data, len);
    uart_tx_wait_blocking(METER_UART);
    rs485_receive_enable();
}
```

---

## Polling Guidelines

- **Poll interval:** 1 second (minimum 500ms)
- **Timeout:** 200ms per request
- **Retry:** 3 attempts before marking meter offline
- **CRC:** Modbus CRC-16 verification required

---

## Example: Reading PZEM-004T

```c
// Read all registers command
uint8_t cmd[] = {0xF8, 0x04, 0x00, 0x00, 0x00, 0x0A, 0x64, 0x64};

// Response format (20 bytes payload):
// [addr][func][len][voltage_h][voltage_l][current_h][current_l]...
// Voltage = (response[3] << 8 | response[4]) * 0.1
// Current = (response[5] << 24 | ... ) * 0.001
```

---

## Adding New Meter Support

To add a new Modbus meter:

1. Create a new `ModbusRegisterMap` with register addresses and scaling factors
2. Add to `METER_MAPS` array for auto-detection
3. Test with actual hardware to verify register map

No code changes required beyond the configuration struct.

