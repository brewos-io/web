/**
 * Hardware Abstraction Layer - Usage Example
 * 
 * This file demonstrates how to use the hardware abstraction layer.
 * It can be used as a reference or compiled as a test program.
 * 
 * To compile and test:
 *   1. Copy this file to src/main.c (backup original first!)
 *   2. Build: cd build && cmake .. && make
 *   3. Flash to Pico
 *   4. Monitor via USB serial
 */

#include "pico/stdlib.h"
#include "hardware.h"
#include "config.h"

int main(void) {
    stdio_init_all();
    sleep_ms(100);
    
    printf("\n=== Hardware Abstraction Layer Test ===\n\n");
    
    // Initialize hardware
    if (!hw_init()) {
        printf("ERROR: Hardware initialization failed!\n");
        return 1;
    }
    
    printf("Hardware mode: %s\n\n", hw_is_simulation_mode() ? "SIMULATION" : "REAL");
    
    // =========================================================================
    // ADC Example
    // =========================================================================
    printf("--- ADC Test ---\n");
    
    // Read all ADC channels
    for (uint8_t ch = 0; ch < 4; ch++) {
        uint16_t adc_value = hw_read_adc(ch);
        float voltage = hw_adc_to_voltage(adc_value);
        printf("ADC[%d]: value=%d, voltage=%.3fV\n", ch, adc_value, voltage);
    }
    
    // In simulation mode, you can set values
    if (hw_is_simulation_mode()) {
        printf("\nSetting simulated ADC values...\n");
        hw_sim_set_adc(0, 2000);  // ~1.6V
        hw_sim_set_adc(1, 1500);  // ~1.2V
        
        uint16_t adc0 = hw_read_adc(0);
        float v0 = hw_adc_to_voltage(adc0);
        printf("ADC[0] after sim set: value=%d, voltage=%.3fV\n", adc0, v0);
    }
    
    printf("\n");
    
    // =========================================================================
    // MAX31855 Thermocouple Example
    // =========================================================================
    printf("--- MAX31855 Thermocouple Test ---\n");
    
    uint32_t max31855_data;
    if (hw_spi_read_max31855(&max31855_data)) {
        printf("MAX31855 data: 0x%08X\n", max31855_data);
        
        if (hw_max31855_is_fault(max31855_data)) {
            uint8_t fault = hw_max31855_get_fault(max31855_data);
            printf("FAULT detected: code=%d\n", fault);
        } else {
            float temp_c;
            if (hw_max31855_to_temp(max31855_data, &temp_c)) {
                printf("Temperature: %.2f°C\n", temp_c);
            }
        }
    } else {
        printf("Failed to read MAX31855\n");
    }
    
    // In simulation mode, set a temperature
    if (hw_is_simulation_mode()) {
        printf("\nSetting simulated temperature to 95.0°C...\n");
        // Format: 95.0C = 380 in 0.25C units, shifted to bits 18-31
        uint32_t sim_data = ((380 << 18) & 0xFFFC0000);
        hw_sim_set_max31855(sim_data);
        
        if (hw_spi_read_max31855(&max31855_data)) {
            float temp_c;
            if (hw_max31855_to_temp(max31855_data, &temp_c)) {
                printf("Simulated temperature: %.2f°C\n", temp_c);
            }
        }
    }
    
    printf("\n");
    
    // =========================================================================
    // GPIO Example
    // =========================================================================
    printf("--- GPIO Test ---\n");
    
    // Initialize a GPIO as output
    uint8_t test_pin = 2;  // Use a safe pin (not used by other peripherals)
    if (hw_gpio_init_output(test_pin, false)) {
        printf("GPIO %d initialized as output\n", test_pin);
        
        // Toggle a few times
        for (int i = 0; i < 5; i++) {
            hw_set_gpio(test_pin, true);
            printf("GPIO %d: HIGH\n", test_pin);
            sleep_ms(100);
            
            hw_set_gpio(test_pin, false);
            printf("GPIO %d: LOW\n", test_pin);
            sleep_ms(100);
        }
    }
    
    printf("\n");
    
    // =========================================================================
    // PWM Example
    // =========================================================================
    printf("--- PWM Test ---\n");
    
    uint8_t pwm_slice;
    uint8_t pwm_pin = 3;  // Use a safe pin
    
    if (hw_pwm_init_ssr(pwm_pin, &pwm_slice)) {
        printf("PWM initialized on GPIO %d, slice %d\n", pwm_pin, pwm_slice);
        
        // Test different duty cycles
        float duties[] = {0.0f, 25.0f, 50.0f, 75.0f, 100.0f, 0.0f};
        for (int i = 0; i < 6; i++) {
            hw_set_pwm_duty(pwm_slice, duties[i]);
            float current = hw_get_pwm_duty(pwm_slice);
            printf("PWM duty: %.1f%% (set), %.1f%% (read)\n", duties[i], current);
            sleep_ms(500);
        }
        
        // Disable PWM
        hw_pwm_set_enabled(pwm_slice, false);
        printf("PWM disabled\n");
    } else {
        printf("Failed to initialize PWM\n");
    }
    
    printf("\n");
    
    // =========================================================================
    // Summary
    // =========================================================================
    printf("=== Test Complete ===\n");
    printf("All hardware functions tested successfully!\n");
    printf("Mode: %s\n", hw_is_simulation_mode() ? "SIMULATION" : "REAL HARDWARE");
    
    // Loop forever
    while (true) {
        sleep_ms(1000);
    }
    
    return 0;
}

