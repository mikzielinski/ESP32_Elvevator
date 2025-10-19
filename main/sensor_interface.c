#include "sensor_interface.h"
#include "elevator_config.h"
#include "hx711.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "SENSORS";

// Button debounce tracking
static uint32_t last_button_press[NUM_FLOORS] = {0};

// HX711 load cell
static hx711_t load_cell;

void sensors_init(void)
{
    // Configure floor sensor pins (inputs with pull-up)
    gpio_config_t floor_sensor_conf = {
        .pin_bit_mask = (1ULL << FLOOR_SENSOR_0) | (1ULL << FLOOR_SENSOR_1) |
                       (1ULL << FLOOR_SENSOR_2) | (1ULL << FLOOR_SENSOR_3),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&floor_sensor_conf);
    
    // Configure button pins (inputs with pull-up)
    gpio_config_t button_conf = {
        .pin_bit_mask = (1ULL << BUTTON_FLOOR_0) | (1ULL << BUTTON_FLOOR_1) |
                       (1ULL << BUTTON_FLOOR_2) | (1ULL << BUTTON_FLOOR_3),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&button_conf);
    
    // Configure LED pins (outputs)
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_FLOOR_0) | (1ULL << LED_FLOOR_1) |
                       (1ULL << LED_FLOOR_2) | (1ULL << LED_FLOOR_3),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_conf);
    
    // Turn off all LEDs initially
    gpio_set_level(LED_FLOOR_0, 0);
    gpio_set_level(LED_FLOOR_1, 0);
    gpio_set_level(LED_FLOOR_2, 0);
    gpio_set_level(LED_FLOOR_3, 0);
    
    // Configure emergency stop button (input with pull-up)
    gpio_config_t emergency_conf = {
        .pin_bit_mask = (1ULL << EMERGENCY_STOP_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&emergency_conf);
    
    // Configure door obstruction sensor (input with pull-up)
    gpio_config_t obstruction_conf = {
        .pin_bit_mask = (1ULL << DOOR_OBSTRUCTION_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&obstruction_conf);
    
    // Initialize HX711 load cell
    hx711_init(&load_cell, HX711_DT_PIN, HX711_SCK_PIN);
    hx711_set_scale(&load_cell, HX711_CALIBRATION_FACTOR);
    hx711_set_offset(&load_cell, HX711_OFFSET);
    
    ESP_LOGI(TAG, "Sensors initialized");
    ESP_LOGI(TAG, "HX711 load cell ready on DT=GPIO%d, SCK=GPIO%d", HX711_DT_PIN, HX711_SCK_PIN);
}

bool sensor_read_floor(int floor)
{
    gpio_num_t pin;
    
    switch (floor) {
        case 0: pin = FLOOR_SENSOR_0; break;
        case 1: pin = FLOOR_SENSOR_1; break;
        case 2: pin = FLOOR_SENSOR_2; break;
        case 3: pin = FLOOR_SENSOR_3; break;
        default: return false;
    }
    
    // Assuming active low sensors (triggered when pressed)
    return gpio_get_level(pin) == 0;
}

int sensor_detect_current_floor(void)
{
    for (int i = 0; i < NUM_FLOORS; i++) {
        if (sensor_read_floor(i)) {
            return i;
        }
    }
    return -1; // Floor not detected
}

bool sensor_read_button(int floor)
{
    gpio_num_t pin;
    
    switch (floor) {
        case 0: pin = BUTTON_FLOOR_0; break;
        case 1: pin = BUTTON_FLOOR_1; break;
        case 2: pin = BUTTON_FLOOR_2; break;
        case 3: pin = BUTTON_FLOOR_3; break;
        default: return false;
    }
    
    // Read button (active low)
    if (gpio_get_level(pin) == 0) {
        // Check debounce
        uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (current_time - last_button_press[floor] > DEBOUNCE_TIME_MS) {
            last_button_press[floor] = current_time;
            ESP_LOGI(TAG, "Button pressed for floor %d", floor);
            return true;
        }
    }
    
    return false;
}

void sensor_set_floor_led(int floor, bool on)
{
    gpio_num_t pin;
    
    switch (floor) {
        case 0: pin = LED_FLOOR_0; break;
        case 1: pin = LED_FLOOR_1; break;
        case 2: pin = LED_FLOOR_2; break;
        case 3: pin = LED_FLOOR_3; break;
        default: return;
    }
    
    gpio_set_level(pin, on ? 1 : 0);
}

void sensor_update_leds(bool floor_requests[])
{
    for (int i = 0; i < NUM_FLOORS; i++) {
        sensor_set_floor_led(i, floor_requests[i]);
    }
}

bool sensor_read_emergency_stop(void)
{
    // Assuming active low (button pressed = emergency)
    return gpio_get_level(EMERGENCY_STOP_PIN) == 0;
}

bool sensor_read_door_obstruction(void)
{
    // Assuming active low (obstruction detected = 0)
    return gpio_get_level(DOOR_OBSTRUCTION_PIN) == 0;
}

bool sensor_check_overload(void)
{
    // Read weight from HX711 load cell
    float weight_kg = hx711_get_units(&load_cell, 3);
    
    // Log weight reading periodically (every ~10 seconds at 100ms intervals = every 100 calls)
    static int log_counter = 0;
    if (log_counter++ >= 100) {
        ESP_LOGI(TAG, "Current weight: %.2f kg", weight_kg);
        log_counter = 0;
    }
    
    // Compare with threshold
    if (weight_kg > OVERLOAD_THRESHOLD_KG) {
        ESP_LOGW(TAG, "Overload detected: %.2f kg (max: %.2f kg)", weight_kg, OVERLOAD_THRESHOLD_KG);
        return true;
    }
    
    return false;
}

