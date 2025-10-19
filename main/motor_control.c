#include "motor_control.h"
#include "elevator_config.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "MOTOR";
static motor_direction_t current_direction = MOTOR_STOP;

void motor_init(void)
{
    // Configure motor direction pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << MOTOR_PIN_1) | (1ULL << MOTOR_PIN_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    // Configure PWM for motor speed control
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = MOTOR_PWM_RESOLUTION,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = MOTOR_PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = {
        .gpio_num = MOTOR_PWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
    
    motor_stop();
    ESP_LOGI(TAG, "Motor initialized");
}

void motor_set_direction(motor_direction_t direction)
{
    current_direction = direction;
    
    switch (direction) {
        case MOTOR_UP:
            gpio_set_level(MOTOR_PIN_1, 1);
            gpio_set_level(MOTOR_PIN_2, 0);
            break;
            
        case MOTOR_DOWN:
            gpio_set_level(MOTOR_PIN_1, 0);
            gpio_set_level(MOTOR_PIN_2, 1);
            break;
            
        case MOTOR_STOP:
        default:
            gpio_set_level(MOTOR_PIN_1, 0);
            gpio_set_level(MOTOR_PIN_2, 0);
            break;
    }
}

void motor_set_speed(int speed)
{
    if (speed < 0) speed = 0;
    if (speed > 1023) speed = 1023;
    
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void motor_stop(void)
{
    motor_set_direction(MOTOR_STOP);
    motor_set_speed(0);
}

void motor_move_up(int speed)
{
    motor_set_direction(MOTOR_UP);
    motor_set_speed(speed);
    ESP_LOGI(TAG, "Motor moving up at speed %d", speed);
}

void motor_move_down(int speed)
{
    motor_set_direction(MOTOR_DOWN);
    motor_set_speed(speed);
    ESP_LOGI(TAG, "Motor moving down at speed %d", speed);
}

motor_direction_t motor_get_direction(void)
{
    return current_direction;
}

