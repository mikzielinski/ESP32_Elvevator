#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "driver/uart.h"
#include "driver/gpio.h"

// Motor control configuration
#define MOTOR_UART_NUM UART_NUM_1
#define MOTOR_TX_PIN GPIO_NUM_12
#define MOTOR_RX_PIN GPIO_NUM_11
#define MOTOR_BAUD_RATE 9600

// Motor commands - ModBus RTU protokół dla DRI0050
// Format: [DEV_ADDR][FUNC][REG_HI][REG_LO][VAL_HI][VAL_LO][CRC_HI][CRC]

// ModBus RTU constants
#define MODBUS_DEV_ADDR 0x32        // Device address
#define MODBUS_FUNC_WRITE 0x06      // Write Single Register
#define MODBUS_REG_DUTY 0x0006      // PWM Duty register
#define MODBUS_REG_FREQ 0x0007      // PWM Frequency register  
#define MODBUS_REG_ENABLE 0x0008    // PWM Enable register

// Motor commands - ModBus RTU frames with CRC
#define MOTOR_STOP_CMD_MODBUS {0x32, 0x06, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00}        // Stop motor (0% duty)
#define MOTOR_FORWARD_CMD_MODBUS {0x32, 0x06, 0x00, 0x06, 0x00, 0xC8, 0x00, 0x00}     // Forward 100% speed (200 duty)
#define MOTOR_BACKWARD_CMD_MODBUS {0x32, 0x06, 0x00, 0x06, 0x00, 0xC8, 0x00, 0x00}    // Backward 100% speed (200 duty)

// CRC calculation function
uint16_t calculate_crc(const uint8_t *data, uint8_t len);

// Długości komend ModBus RTU
#define MOTOR_CMD_LEN 8

// Motor states
typedef enum {
    MOTOR_STATE_STOPPED = 0,
    MOTOR_STATE_FORWARD,
    MOTOR_STATE_BACKWARD
} motor_state_t;

// Initialize motor control
void motor_control_init(void);

// Motor control functions
void motor_start_forward(void);
void motor_start_backward(void);
void motor_stop(void);

// Get current motor state
motor_state_t motor_get_state(void);

// Set motor state
void motor_set_state(motor_state_t state);

// Test function with different commands
void motor_test_commands(void);

// Check motor power status (READ ENABLE and DUTY registers)
void motor_check_power(void);

// Speed control functions
void motor_set_speed(uint8_t speed_percent);  // 0-100%
void motor_set_frequency(uint16_t freq_hz);   // 1-1000 Hz

// Predefined speed functions
void motor_forward_fast(void);     // 100% speed forward
void motor_backward_fast(void);    // 100% speed backward
void motor_forward_medium(void);   // 50% speed forward
void motor_backward_medium(void);  // 50% speed backward

#endif // MOTOR_CONTROL_H