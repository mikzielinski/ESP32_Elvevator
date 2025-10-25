#ifndef MOTOR_CONTROL_BTS7960_H
#define MOTOR_CONTROL_BTS7960_H

#include "esp_err.h"
#include "driver/ledc.h"

// BTS7960 Pin assignments (user physical wiring)
#define BTS7960_RPWM_PIN GPIO_NUM_38   // PWM Forward (RPWM) - niebieski
#define BTS7960_LPWM_PIN GPIO_NUM_39   // PWM Reverse (LPWM) - zielony
#define BTS7960_REN_PIN  GPIO_NUM_4    // Inhibit / Enable (R_S) - safe GPIO
#define BTS7960_LEN_PIN  GPIO_NUM_5    // Direction (L_S - IN) - safe GPIO

// PWM Configuration (ChatGPT suggestion)
#define BTS7960_PWM_FREQ 20000          // 20 kHz frequency - cisza akustyczna
#define BTS7960_PWM_RESOLUTION LEDC_TIMER_8_BIT  // 0-255 range
#define BTS7960_PWM_CHANNEL_FORWARD LEDC_CHANNEL_0
#define BTS7960_PWM_CHANNEL_REVERSE LEDC_CHANNEL_1

// Motor states
typedef enum {
    MOTOR_STATE_STOPPED = 0,
    MOTOR_STATE_FORWARD,
    MOTOR_STATE_BACKWARD
} motor_state_t;

// Function prototypes
esp_err_t motor_control_init(void);
void motor_start_forward(void);
void motor_start_backward(void);
void motor_stop(void);
void motor_set_speed(uint8_t speed_percent);
motor_state_t motor_get_state(void);
void motor_check_power(void);

// Predefined speed functions
void motor_forward_fast(void);
void motor_backward_fast(void);
void motor_forward_medium(void);
void motor_backward_medium(void);

#endif // MOTOR_CONTROL_BTS7960_H
