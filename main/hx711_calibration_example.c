/*
 * HX711 Calibration Example
 * 
 * This file contains example code for calibrating the HX711 load cell.
 * DO NOT include this in the main build - it's for reference only.
 * 
 * To calibrate:
 * 1. Upload this code instead of the main elevator code
 * 2. Remove all weight from the load cell
 * 3. Note the OFFSET value from serial output
 * 4. Place a known weight (e.g., 1kg) on the load cell
 * 5. Calculate CALIBRATION_FACTOR = (reading - offset) / known_weight_kg
 * 6. Update elevator_config.h with these values
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hx711.h"
#include "elevator_config.h"

static const char *TAG = "HX711_CALIBRATION";
static hx711_t scale;

void app_main(void)
{
    ESP_LOGI(TAG, "=== HX711 Calibration Tool ===");
    ESP_LOGI(TAG, "Initializing HX711...");
    
    // Initialize HX711
    hx711_init(&scale, HX711_DT_PIN, HX711_SCK_PIN);
    
    ESP_LOGI(TAG, "Remove all weight from the load cell");
    ESP_LOGI(TAG, "Waiting 3 seconds...");
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "Taring...");
    hx711_tare(&scale, 20);
    long offset = scale.offset;
    ESP_LOGI(TAG, "===========================================");
    ESP_LOGI(TAG, "OFFSET = %ld", offset);
    ESP_LOGI(TAG, "===========================================");
    
    ESP_LOGI(TAG, "\nPlace a known weight on the load cell (e.g., 1kg)");
    ESP_LOGI(TAG, "Waiting 5 seconds...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    ESP_LOGI(TAG, "\nReading values...");
    
    while (1) {
        long raw_value = hx711_read_average(&scale, 10);
        long value_with_offset = raw_value - offset;
        
        ESP_LOGI(TAG, "Raw: %ld | Value: %ld", raw_value, value_with_offset);
        ESP_LOGI(TAG, "For known weight (e.g. 1.0 kg):");
        ESP_LOGI(TAG, "  CALIBRATION_FACTOR = %ld / known_weight_kg", value_with_offset);
        ESP_LOGI(TAG, "  Example: %.2f for 1kg", (float)value_with_offset / 1.0);
        ESP_LOGI(TAG, "---");
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

