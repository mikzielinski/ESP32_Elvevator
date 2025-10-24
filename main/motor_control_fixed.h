/*
 * ESP32-Elevator Motor Control - Fixed Header
 * 
 * This header file contains the corrected definitions for motor control
 * with proper Modbus RTU protocol support for DFRobot DRI0050 driver.
 */

#ifndef MOTOR_CONTROL_FIXED_H
#define MOTOR_CONTROL_FIXED_H

#include <stdint.h>
#include "driver/uart.h"
#include "driver/gpio.h"

// UART Configuration
#define MOTOR_UART_NUM UART_NUM_1
#define MOTOR_TX_PIN GPIO_NUM_12
#define MOTOR_RX_PIN GPIO_NUM_11
#define MOTOR_BAUD_RATE 9600

// Modbus RTU Configuration for DRI0050
#define MODBUS_DEV_ADDR 0x32        // Device address (50 decimal)
#define MODBUS_FUNC_WRITE 0x06       // Write single register
#define MODBUS_FUNC_READ 0x03        // Read holding registers

// DRI0050 Register Addresses
#define MODBUS_REG_DUTY 0x0006       // Duty cycle register (0-1000)
#define MODBUS_REG_FREQ 0x0007       // Frequency register (1-1000 Hz)
#define MODBUS_REG_DIR 0x0008        // Direction register (0=forward, 1=reverse)

// Motor Control Limits
#define MOTOR_MAX_DUTY 1000          // Maximum duty cycle (100%)
#define MOTOR_MIN_DUTY 0             // Minimum duty cycle (0%)
#define MOTOR_MAX_FREQ 1000          // Maximum frequency (1000 Hz)
#define MOTOR_MIN_FREQ 1            // Minimum frequency (1 Hz)

// Safety Limits
#define MOTOR_EMERGENCY_STOP_DUTY 0  // Emergency stop duty
#define MOTOR_SAFE_START_DUTY 100    // Safe starting duty (10%)

// Function Declarations

/**
 * @brief Calculate Modbus RTU CRC-16 checksum
 * @param buffer Data buffer
 * @param length Buffer length
 * @return CRC-16 checksum
 */
uint16_t modbus_crc16(const uint8_t *buffer, uint16_t length);

/**
 * @brief Send Modbus RTU command to motor driver
 * @param dev Device address
 * @param func Function code
 * @param reg Register address
 * @param val Value to write
 */
void send_modbus_cmd(uint8_t dev, uint8_t func, uint16_t reg, uint16_t val);

/**
 * @brief Initialize motor control system
 * Configures UART and GPIO pins for motor communication
 */
void motor_init(void);

/**
 * @brief Stop motor immediately
 * Sets duty cycle to 0%
 */
void motor_stop(void);

/**
 * @brief Set motor speed
 * @param duty Duty cycle (0-1000, where 1000 = 100%)
 */
void motor_set_speed(uint16_t duty);

/**
 * @brief Set motor frequency
 * @param freq Frequency in Hz (1-1000)
 */
void motor_set_frequency(uint16_t freq);

/**
 * @brief Emergency stop motor
 * Immediately stops motor and waits for command completion
 */
void motor_emergency_stop(void);

/**
 * @brief Run motor test sequence
 * Tests motor at different speeds for diagnostics
 */
void motor_test_sequence(void);

/**
 * @brief Read motor status
 * Reads current motor parameters from driver
 */
void motor_read_status(void);

/**
 * @brief Setup motor with default settings
 * Initializes motor with safe default parameters
 */
void motor_setup_default(void);

// Utility Functions

/**
 * @brief Convert percentage to duty cycle
 * @param percent Percentage (0-100)
 * @return Duty cycle (0-1000)
 */
static inline uint16_t percent_to_duty(uint8_t percent) {
    if (percent > 100) percent = 100;
    return (uint16_t)(percent * 10);
}

/**
 * @brief Convert duty cycle to percentage
 * @param duty Duty cycle (0-1000)
 * @return Percentage (0-100)
 */
static inline uint8_t duty_to_percent(uint16_t duty) {
    if (duty > 1000) duty = 1000;
    return (uint8_t)(duty / 10);
}

/**
 * @brief Check if duty cycle is within safe limits
 * @param duty Duty cycle to check
 * @return true if safe, false if unsafe
 */
static inline bool is_duty_safe(uint16_t duty) {
    return (duty >= MOTOR_MIN_DUTY && duty <= MOTOR_MAX_DUTY);
}

/**
 * @brief Check if frequency is within limits
 * @param freq Frequency to check
 * @return true if valid, false if invalid
 */
static inline bool is_freq_valid(uint16_t freq) {
    return (freq >= MOTOR_MIN_FREQ && freq <= MOTOR_MAX_FREQ);
}

// Error Codes
#define MOTOR_ERROR_NONE 0
#define MOTOR_ERROR_INVALID_DUTY 1
#define MOTOR_ERROR_INVALID_FREQ 2
#define MOTOR_ERROR_UART_FAIL 3
#define MOTOR_ERROR_CRC_FAIL 4

// Debug Macros
#ifdef CONFIG_MOTOR_DEBUG
#define MOTOR_DEBUG_PRINT(fmt, ...) printf("[MOTOR] " fmt, ##__VA_ARGS__)
#else
#define MOTOR_DEBUG_PRINT(fmt, ...)
#endif

#endif // MOTOR_CONTROL_FIXED_H
