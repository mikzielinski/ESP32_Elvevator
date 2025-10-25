#include "motor_control_bts7960.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "MOTOR_BTS7960";

static motor_state_t motor_state = MOTOR_STATE_STOPPED;
static uint8_t current_speed = 128; // Default speed

esp_err_t motor_control_init(void)
{
    // --- GPIO setup for EN/DIR ---
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BTS7960_REN_PIN) | (1ULL << BTS7960_LEN_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(BTS7960_REN_PIN, 0);
    gpio_set_level(BTS7960_LEN_PIN, 0);

    // --- PWM timer setup ---
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = BTS7960_PWM_RESOLUTION,
        .freq_hz = BTS7960_PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // --- Forward channel (RPWM) ---
    ledc_channel_config_t forward_channel = {
        .gpio_num = BTS7960_RPWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BTS7960_PWM_CHANNEL_FORWARD,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&forward_channel));

    // --- Reverse channel (LPWM) ---
    ledc_channel_config_t reverse_channel = {
        .gpio_num = BTS7960_LPWM_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = BTS7960_PWM_CHANNEL_REVERSE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&reverse_channel));

    ESP_LOGI(TAG, "BTS7960 motor control initialized");
    motor_state = MOTOR_STATE_STOPPED;
    return ESP_OK;
}

static void motor_apply_pwm(uint8_t forward_duty, uint8_t reverse_duty)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD, forward_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_FORWARD);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE, reverse_duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BTS7960_PWM_CHANNEL_REVERSE);
}

void motor_stop(void)
{
    motor_apply_pwm(0, 0);
    gpio_set_level(BTS7960_REN_PIN, 0);
    gpio_set_level(BTS7960_LEN_PIN, 0);
    motor_state = MOTOR_STATE_STOPPED;
    ESP_LOGI(TAG, "Motor STOPPED");
}

void motor_start_forward(void)
{
    gpio_set_level(BTS7960_LEN_PIN, 1); // Direction
    gpio_set_level(BTS7960_REN_PIN, 1); // Enable
    motor_apply_pwm(current_speed, 0);
    motor_state = MOTOR_STATE_FORWARD;
    ESP_LOGI(TAG, "Motor FORWARD at duty %d", current_speed);
}

void motor_start_backward(void)
{
    gpio_set_level(BTS7960_LEN_PIN, 0); // Direction
    gpio_set_level(BTS7960_REN_PIN, 1); // Enable
    motor_apply_pwm(0, current_speed);
    motor_state = MOTOR_STATE_BACKWARD;
    ESP_LOGI(TAG, "Motor BACKWARD at duty %d", current_speed);
}

void motor_set_speed(uint8_t speed_percent)
{
    if (speed_percent > 100) speed_percent = 100;
    current_speed = (uint8_t)((speed_percent * 255) / 100); // 0-255 duty

    if (motor_state == MOTOR_STATE_FORWARD)
        motor_apply_pwm(current_speed, 0);
    else if (motor_state == MOTOR_STATE_BACKWARD)
        motor_apply_pwm(0, current_speed);

    ESP_LOGI(TAG, "Motor speed set to %d%% (%d duty)", speed_percent, current_speed);
}

motor_state_t motor_get_state(void)
{
    return motor_state;
}

// --- optional test helpers ---
void motor_forward_fast(void)   { motor_set_speed(100); motor_start_forward(); }
void motor_backward_fast(void)  { motor_set_speed(100); motor_start_backward(); }
void motor_forward_medium(void) { motor_set_speed(60);  motor_start_forward(); }
void motor_backward_medium(void){ motor_set_speed(60);  motor_start_backward(); }

void motor_check_power(void)
{
    // Placeholder for future current/voltage sense if HX711 is repurposed
    ESP_LOGD(TAG, "motor_check_power(): not implemented");
}