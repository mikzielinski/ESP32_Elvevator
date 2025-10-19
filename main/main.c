#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hx711.h"
#include "hx711_config.h"

static const char *TAG = "HX711_DEMO";
static hx711_t scale;

void app_main(void)
{
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "    HX711 Load Cell Weight Monitor");
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "DT Pin:  GPIO%d (Orange)", HX711_DT_PIN);
    ESP_LOGI(TAG, "SCK Pin: GPIO%d (Yellow)", HX711_SCK_PIN);
    ESP_LOGI(TAG, "===========================================");
    
    // Initialize HX711
    ESP_LOGI(TAG, "Initializing HX711...");
    hx711_init(&scale, HX711_DT_PIN, HX711_SCK_PIN);
    
    // Set calibration values from config
    hx711_set_scale(&scale, HX711_CALIBRATION_FACTOR);
    hx711_set_offset(&scale, HX711_OFFSET);
    
    ESP_LOGI(TAG, "HX711 initialized successfully!");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "NOTE: To calibrate, see CALIBRATION_GUIDE.md");
    ESP_LOGI(TAG, "");
    
    // Option to run calibration mode
    // Uncomment the section below for calibration:
    /*
    ESP_LOGI(TAG, "=== CALIBRATION MODE ===");
    ESP_LOGI(TAG, "Remove all weight from scale...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Taring...");
    hx711_tare(&scale, 20);
    ESP_LOGI(TAG, "OFFSET = %ld", scale.offset);
    ESP_LOGI(TAG, "Update HX711_OFFSET in hx711_config.h with this value");
    ESP_LOGI(TAG, "");
    
    ESP_LOGI(TAG, "Place known weight (e.g., 1 kg) and wait...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    long raw_value = hx711_read_average(&scale, 10);
    long value = raw_value - scale.offset;
    ESP_LOGI(TAG, "Raw value: %ld", raw_value);
    ESP_LOGI(TAG, "Value (after offset): %ld", value);
    ESP_LOGI(TAG, "For known weight, calculate:");
    ESP_LOGI(TAG, "  CALIBRATION_FACTOR = %ld / weight_in_kg", value);
    ESP_LOGI(TAG, "  Example for 1kg: %.2f", (float)value / 1.0);
    ESP_LOGI(TAG, "Update HX711_CALIBRATION_FACTOR in hx711_config.h");
    ESP_LOGI(TAG, "=== END CALIBRATION ===");
    ESP_LOGI(TAG, "");
    */
    
    // Main loop - continuous weight reading
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
