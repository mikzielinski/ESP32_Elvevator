#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hx711.h"
#include "hx711_config.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "motor_control_bts7960.h"

static const char *TAG = "HX711_DEMO";
static hx711_t scale;

void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "  HX711 Load Cell WiFi Dashboard");
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "DT Pin:  GPIO%d (Orange)", HX711_DT_PIN);
    ESP_LOGI(TAG, "SCK Pin: GPIO%d (Yellow)", HX711_SCK_PIN);
    ESP_LOGI(TAG, "===========================================");
    
    // Initialize WiFi
    ESP_LOGI(TAG, "Connecting to WiFi...");
    wifi_init();
    ESP_LOGI(TAG, "WiFi connected! IP: %s", wifi_get_ip());
    
    // Initialize HX711
    ESP_LOGI(TAG, "Initializing HX711...");
    hx711_init(&scale, HX711_DT_PIN, HX711_SCK_PIN);
    
    // Set calibration values from config
    hx711_set_scale(&scale, HX711_CALIBRATION_FACTOR);
    hx711_set_offset(&scale, HX711_OFFSET);
    
    ESP_LOGI(TAG, "HX711 initialized successfully!");
    
    // Initialize motor control
    ESP_LOGI(TAG, "Initializing motor control...");
    motor_control_init();
    ESP_LOGI(TAG, "Motor control initialized!");
    
    // Test motor commands
    ESP_LOGI(TAG, "Testing BTS7960 motor commands...");
    ESP_LOGI(TAG, "Test 1: Forward command");
    motor_start_forward();
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "Test 2: Backward command");
    motor_start_backward();
    vTaskDelay(pdMS_TO_TICKS(2000));
    motor_stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "BTS7960 motor tests completed!");
    
    // Check motor power status
    ESP_LOGI(TAG, "Checking BTS7960 power status...");
    motor_check_power();
    
    // Start web server
    ESP_LOGI(TAG, "Starting web server...");
    web_server_init();
    
    // Reset motor state on system startup (in case of power restoration)
    ESP_LOGI(TAG, "Resetting motor state on startup...");
    web_server_reset_motor_state();
    
    // Set HX711 pointer for web server zeroing functionality
    web_server_set_hx711(&scale);
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "  Dashboard: http://%s", wifi_get_ip());
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    
    // Calibration mode - DISABLED (final calibration completed)
    ESP_LOGI(TAG, "HX711 final calibration completed and ready!");
    
    // Main loop - continuous weight reading and web updates
    ESP_LOGI(TAG, "Starting continuous weight monitoring...");
    ESP_LOGI(TAG, "");
    
    int reading_count = 0;
    
    while (1) {
        // Check if HX711 is ready
        if (hx711_is_ready(&scale)) {
            // Read weight in configured units (kg)
            float weight = hx711_get_units(&scale, READINGS_PER_SAMPLE);
            
            // Read raw value for debugging
            long raw_value = hx711_read_average(&scale, READINGS_PER_SAMPLE);
            
            // Display results
            reading_count++;
            ESP_LOGI(TAG, "[%d] Weight: %.2f kg | Raw: %ld",
                     reading_count, weight, raw_value);

            // Process weight with smart averaging
            if (wifi_is_connected()) {
                web_server_process_weight(weight, raw_value);
            }
            
            // Check for extreme values (possible error)
            if (weight < -10.0 || weight > 10000.0) {
                ESP_LOGW(TAG, "⚠️  Unusual reading - check sensor or calibration!");
            }
            
        } else {
            ESP_LOGW(TAG, "HX711 not ready!");
        }
        
        // Wait before next reading
        vTaskDelay(pdMS_TO_TICKS(UPDATE_INTERVAL_MS));
    }
}
