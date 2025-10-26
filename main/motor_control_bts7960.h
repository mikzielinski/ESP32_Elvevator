#ifndef MOTOR_CONTROL_BTS7960_H
#define MOTOR_CONTROL_BTS7960_H

#include "esp_err.h"
#include "driver/ledc.h"

// === BTS7960 Pin Mapping (ESP32-S3 Safe Pins) ===
#define BTS7960_LPWM_PIN GPIO_NUM_8    // PWM Reverse (L_S)
#define BTS7960_RPWM_PIN GPIO_NUM_9    // PWM Forward (R_S)
#define BTS7960_LEN_PIN  GPIO_NUM_5    // Enable Left
#define BTS7960_REN_PIN  GPIO_NUM_4    // Enable Right

// === PWM Configuration ===
#define BTS7960_PWM_FREQ        20000                       // 20 kHz — silent
#define BTS7960_PWM_RESOLUTION  LEDC_TIMER_8_BIT            // 0–255
#define BTS7960_PWM_CHANNEL_FORWARD LEDC_CHANNEL_0
#define BTS7960_PWM_CHANNEL_REVERSE LEDC_CHANNEL_1

// === Motor States ===
typedef enum {
    MOTOR_STATE_STOPPED = 0,
    MOTOR_STATE_FORWARD,
    MOTOR_STATE_BACKWARD
} motor_state_t;

// === Function Prototypes ===
esp_err_t motor_control_init(void);
void motor_start_forward(void);
void motor_start_backward(void);
void motor_stop(void);
void motor_set_speed(uint8_t speed_percent);
motor_state_t motor_get_state(void);

// === Predefined Speed Presets ===
void motor_forward_fast(void);
void motor_backward_fast(void);
void motor_forward_medium(void);
void motor_backward_medium(void);

// === Power Check ===
void motor_check_power(void);

#endif // MOTOR_CONTROL_BTS7960_H