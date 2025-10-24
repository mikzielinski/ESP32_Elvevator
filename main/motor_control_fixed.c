/*
 * ESP32-Elevator Motor Control - Fixed Version
 * 
 * This file contains the corrected motor control implementation
 * with proper Modbus RTU CRC-16 calculation for DFRobot DRI0050 driver.
 * 
 * Problem: DRI0050 was not responding to UART commands
 * Solution: Added proper CRC-16 calculation for Modbus RTU protocol
 */

#include "motor_control.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <string.h>

// Modbus RTU CRC-16 calculation
uint16_t modbus_crc16(const uint8_t *buffer, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (int i = 0; i < length; i++) {
        crc ^= buffer[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Send Modbus RTU command to DRI0050
void send_modbus_cmd(uint8_t dev, uint8_t func, uint16_t reg, uint16_t val) {
    uint8_t frame[8];
    
    // Build Modbus RTU frame
    frame[0] = dev;           // Device address
    frame[1] = func;          // Function code
    frame[2] = reg >> 8;      // Register high byte
    frame[3] = reg & 0xFF;    // Register low byte
    frame[4] = val >> 8;      // Value high byte
    frame[5] = val & 0xFF;    // Value low byte
    
    // Calculate CRC-16 for first 6 bytes
    uint16_t crc = modbus_crc16(frame, 6);
    frame[6] = crc & 0xFF;    // CRC Low byte
    frame[7] = crc >> 8;      // CRC High byte
    
    // Send frame via UART
    uart_write_bytes(MOTOR_UART_NUM, (const char*)frame, 8);
    
    // Debug output
    printf("Sent Modbus frame: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", frame[i]);
    }
    printf("\n");
}

// Initialize motor control system
void motor_init() {
    printf("Initializing motor control...\n");
    
    // Configure UART
    uart_config_t uart_config = {
        .baud_rate = MOTOR_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };
    
    // Configure UART parameters
    uart_param_config(MOTOR_UART_NUM, &uart_config);
    
    // Set UART pins
    uart_set_pin(MOTOR_UART_NUM, MOTOR_TX_PIN, MOTOR_RX_PIN, 
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // Install UART driver
    uart_driver_install(MOTOR_UART_NUM, 256, 0, 0, NULL, 0);
    
    printf("Motor control initialized\n");
    printf("UART%d: TX=%d, RX=%d, Baud=%d\n", 
           MOTOR_UART_NUM, MOTOR_TX_PIN, MOTOR_RX_PIN, MOTOR_BAUD_RATE);
}

// Stop motor
void motor_stop() {
    printf("Stopping motor...\n");
    send_modbus_cmd(MODBUS_DEV_ADDR, MODBUS_FUNC_WRITE, MODBUS_REG_DUTY, 0);
}

// Set motor speed (0-1000 duty cycle)
void motor_set_speed(uint16_t duty) {
    if (duty > 1000) duty = 1000;  // Limit to maximum
    
    printf("Setting motor speed to %d (%.1f%%)\n", duty, (float)duty/10.0);
    send_modbus_cmd(MODBUS_DEV_ADDR, MODBUS_FUNC_WRITE, MODBUS_REG_DUTY, duty);
}

// Set motor frequency (1-1000 Hz)
void motor_set_frequency(uint16_t freq) {
    if (freq < 1) freq = 1;
    if (freq > 1000) freq = 1000;
    
    printf("Setting motor frequency to %d Hz\n", freq);
    send_modbus_cmd(MODBUS_DEV_ADDR, MODBUS_FUNC_WRITE, MODBUS_REG_FREQ, freq);
}

// Emergency stop - immediate motor shutdown
void motor_emergency_stop() {
    printf("EMERGENCY STOP!\n");
    motor_stop();
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Wait for command to be sent
}

// Test motor with different speeds
void motor_test_sequence() {
    printf("Starting motor test sequence...\n");
    
    // Test different speeds
    uint16_t test_speeds[] = {100, 200, 300, 500, 800, 1000};
    int num_tests = sizeof(test_speeds) / sizeof(test_speeds[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("Test %d/%d: Speed %d\n", i+1, num_tests, test_speeds[i]);
        motor_set_speed(test_speeds[i]);
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Run for 2 seconds
    }
    
    // Stop motor
    motor_stop();
    printf("Motor test sequence completed\n");
}

// Read motor status (if supported by DRI0050)
void motor_read_status() {
    printf("Reading motor status...\n");
    // Note: This would require implementing Modbus read function
    // For now, just send a status request
    send_modbus_cmd(MODBUS_DEV_ADDR, 0x03, 0x0000, 0x0001);  // Read holding register
}

// Initialize motor with default settings
void motor_setup_default() {
    printf("Setting up motor with default settings...\n");
    
    // Set default frequency (100 Hz)
    motor_set_frequency(100);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    // Set default duty (0% - stopped)
    motor_set_speed(0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    printf("Motor setup completed\n");
}
