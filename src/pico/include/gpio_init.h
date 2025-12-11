/**
 * Pico Firmware - GPIO Initialization
 * 
 * Centralized GPIO initialization based on PCB configuration.
 * All GPIO setup happens here using the active PCB configuration.
 */

#ifndef GPIO_INIT_H
#define GPIO_INIT_H

#include <stdbool.h>
#include "pcb_config.h"

// =============================================================================
// GPIO Initialization
// =============================================================================

/**
 * Initialize all GPIO pins based on PCB configuration
 * Must be called early in main() before using any GPIO
 * 
 * Returns true on success, false if PCB config is invalid
 */
bool gpio_init_all(void);

/**
 * Initialize UART pins for ESP32 communication
 * Called separately to allow early UART setup
 */
void gpio_init_uart_esp32(void);

/**
 * Initialize ADC pins for sensor reading
 */
void gpio_init_adc(void);

/**
 * Initialize SPI pins for thermocouple
 */
void gpio_init_spi(void);

/**
 * Initialize I2C pins
 */
void gpio_init_i2c(void);

/**
 * Initialize digital input pins
 */
void gpio_init_inputs(void);

/**
 * Initialize output pins (relays, SSRs, LEDs)
 */
void gpio_init_outputs(void);

/**
 * Initialize PWM pins for SSR control
 */
void gpio_init_pwm(void);

#endif // GPIO_INIT_H

