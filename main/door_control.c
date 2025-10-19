#include "door_control.h"
#include "elevator_config.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DOOR";
static door_state_t current_state = DOOR_CLOSED;

// Convert angle to PWM duty cycle for servo
static uint32_t angle_to_duty(int angle)
{
    // Standard servo: 1ms = 0°, 1.5ms = 90°, 2ms = 180°
    // At 50Hz (20ms period), with 10-bit resolution (1024 steps)
    // 1ms = 5% duty = 51 (out of 1023)
    // 2ms = 10% duty = 102 (out of 1023)
    const uint32_t min_duty = 51;
    const uint32_t max_duty = 102;
    
    uint32_t duty = min_duty + ((max_duty - min_duty) * angle) / 180;
    return duty;
}

void door_init(void)
{
    // Configure PWM for servo control
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_1,
        .freq_hz = DOOR_SERVO_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = {
        .gpio_num = DOOR_SERVO_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_1,
        .duty = angle_to_duty(DOOR_CLOSE_ANGLE),
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
    
    current_state = DOOR_CLOSED;
    ESP_LOGI(TAG, "Door control initialized");
}

void door_open(void)
{
    uint32_t duty = angle_to_duty(DOOR_OPEN_ANGLE);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    
    current_state = DOOR_OPEN;
    ESP_LOGI(TAG, "Door opened");
    
    // Give servo time to move
    vTaskDelay(pdMS_TO_TICKS(500));
}

void door_close(void)
{
    uint32_t duty = angle_to_duty(DOOR_CLOSE_ANGLE);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    
    current_state = DOOR_CLOSED;
    ESP_LOGI(TAG, "Door closed");
    
    // Give servo time to move
    vTaskDelay(pdMS_TO_TICKS(500));
}

door_state_t door_get_state(void)
{
    return current_state;
}

bool door_is_open(void)
{
    return current_state == DOOR_OPEN;
}

bool door_is_closed(void)
{
    return current_state == DOOR_CLOSED;
}

bool door_check_obstruction(void)
{
    // This would check a sensor - for now return false
    // In real implementation, check GPIO pin or IR sensor
    return false;
}

