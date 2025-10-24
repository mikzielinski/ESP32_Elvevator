/*
 * ESP32-Elevator Main Application - Fixed Version
 * 
 * This file demonstrates the corrected motor control implementation
 * with proper Modbus RTU CRC-16 calculation.
 * 
 * Usage:
 * 1. Replace your main.c with this file
 * 2. Include motor_control_fixed.h instead of motor_control.h
 * 3. Build and flash the firmware
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "motor_control_fixed.h"

static const char *TAG = "ESP32_ELEVATOR";

void app_main(void) {
    ESP_LOGI(TAG, "ESP32-Elevator Control System Starting...");
    ESP_LOGI(TAG, "Fixed version with proper Modbus RTU CRC-16");
    
    // Initialize motor control system
    motor_init();
    
    // Setup motor with default settings
    motor_setup_default();
    
    // Wait for system to stabilize
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    ESP_LOGI(TAG, "Starting motor test sequence...");
    
    // Test motor at different speeds
    uint16_t test_speeds[] = {100, 200, 300, 500, 800, 1000};
    int num_tests = sizeof(test_speeds) / sizeof(test_speeds[0]);
    
    for (int i = 0; i < num_tests; i++) {
        ESP_LOGI(TAG, "Test %d/%d: Setting speed to %d (%.1f%%)", 
                 i+1, num_tests, test_speeds[i], (float)test_speeds[i]/10.0);
        
        motor_set_speed(test_speeds[i]);
        vTaskDelay(2000 / portTICK_PERIOD_MS);  // Run for 2 seconds
    }
    
    // Stop motor
    ESP_LOGI(TAG, "Stopping motor...");
    motor_stop();
    
    // Wait before emergency stop test
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    // Test emergency stop
    ESP_LOGI(TAG, "Testing emergency stop...");
    motor_set_speed(500);  // Set some speed
    vTaskDelay(500 / portTICK_PERIOD_MS);
    motor_emergency_stop();
    
    ESP_LOGI(TAG, "Motor test completed successfully!");
    ESP_LOGI(TAG, "If motor was running, the fix is working correctly.");
    
    // Main application loop
    while (1) {
        ESP_LOGI(TAG, "System running... Motor control ready.");
        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Log every 10 seconds
    }
}

/*
 * Alternative main function for continuous operation
 * Uncomment this and comment out the above app_main() if you want
 * continuous motor operation instead of test sequence.
 */
/*
void app_main(void) {
    ESP_LOGI(TAG, "ESP32-Elevator Control System Starting...");
    
    // Initialize motor control
    motor_init();
    motor_setup_default();
    
    // Start motor at 50% speed
    motor_set_speed(500);
    
    ESP_LOGI(TAG, "Motor started at 50% speed");
    ESP_LOGI(TAG, "System ready for operation");
    
    // Main loop
    while (1) {
        // Your application logic here
        // For example: read sensors, update web interface, etc.
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
*/

/*
 * Example of integrating with web server
 * This shows how to control motor from web interface
 */
void motor_web_control_example(void) {
    // Example web server integration
    ESP_LOGI(TAG, "Web control example:");
    ESP_LOGI(TAG, "GET /motor/start - Start motor");
    ESP_LOGI(TAG, "GET /motor/stop - Stop motor");
    ESP_LOGI(TAG, "GET /motor/speed?value=500 - Set speed to 50%");
    ESP_LOGI(TAG, "GET /motor/emergency - Emergency stop");
}

/*
 * Example of sensor integration
 * This shows how to integrate with HX711 load cell
 */
void sensor_integration_example(void) {
    ESP_LOGI(TAG, "Sensor integration example:");
    ESP_LOGI(TAG, "1. Read weight from HX711");
    ESP_LOGI(TAG, "2. Adjust motor speed based on load");
    ESP_LOGI(TAG, "3. Implement safety limits");
    ESP_LOGI(TAG, "4. Log sensor data");
}
