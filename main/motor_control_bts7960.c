#include "motor_control_bts7960.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define TAG "MOTOR_BTS7960"

static motor_state_t current_state = MOTOR_STATE_STOPPED;
static uint8_t current_speed = 128; // default half speed

esp_err_t motor_control_init(void)
{
    // --- Configure Enable Pins ---
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTS7960_LEN_PIN) | (1ULL << BTS7960_REN_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Default state — disabled
    gpio_set_level(BTS7960_LEN_PIN, 0);
    gpio_set_level(BTS7960_REN_PIN, 0);

    // --- Configure PWM Timers ---
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = BTS7960_PWM_RESOLUTION,
        .freq_hz = BTS7960_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // --- Configure PWM Channels ---
    ledc_channel_config_t forward_channel = {
        .gpio_num = BTS7960_RPWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BTS7960_PWM_CHANNEL_FORWARD,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&forward_channel));

    ledc_channel_config_t reverse_channel = {
        .gpio_num = BTS7960_LPWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BTS7960_PWM_CHANNEL_REVERSE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&reverse_channel));

    ESP_LOGI(TAG, "BTS7960 motor control initialized (LPWM=GPIO %d, RPWM=GPIO %d)", 
             BTS7960_LPWM_PIN, BTS7960_RPWM_PIN);

    current_state = MOTOR_STATE_STOPPED;
    return ESP_OK;
}

void motor_start_forward(void)
{
    ESP_LOGI(TAG, "Motor FORWARD at duty %d", current_speed);
    gpio_set_level(BTS7960_LEN_PIN, 1);
    gpio_set_level(BTS7960_REN_PIN, 1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD, current_speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE);
    current_state = MOTOR_STATE_FORWARD;
}

void motor_start_backward(void)
{
    ESP_LOGI(TAG, "Motor BACKWARD at duty %d", current_speed);
    gpio_set_level(BTS7960_LEN_PIN, 1);
    gpio_set_level(BTS7960_REN_PIN, 1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE, current_speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD);
    current_state = MOTOR_STATE_BACKWARD;
}

void motor_stop(void)
{
    ESP_LOGI(TAG, "Motor STOPPED");
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD, 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE);
    gpio_set_level(BTS7960_LEN_PIN, 0);
    gpio_set_level(BTS7960_REN_PIN, 0);
    current_state = MOTOR_STATE_STOPPED;
}

void motor_set_speed(uint8_t speed_percent)
{
    if (speed_percent > 100) speed_percent = 100;
    current_speed = (255 * speed_percent) / 100;
    ESP_LOGI(TAG, "Motor speed set to %d%% (duty %d)", speed_percent, current_speed);
}

motor_state_t motor_get_state(void)
{
    return current_state;
}

void motor_forward_fast(void)    { motor_set_speed(100); motor_start_forward(); }
void motor_backward_fast(void)   { motor_set_speed(100); motor_start_backward(); }
void motor_forward_medium(void)  { motor_set_speed(60);  motor_start_forward(); }
void motor_backward_medium(void) { motor_set_speed(60);  motor_start_backward(); }

void motor_check_power(void)
{
    // Placeholder for future current/voltage sense if HX711 is repurposed
    ESP_LOGD(TAG, "motor_check_power(): not implemented");
}