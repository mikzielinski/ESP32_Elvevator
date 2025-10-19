#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "elevator_control.h"
#include "motor_control.h"
#include "door_control.h"
#include "sensor_interface.h"

static const char *TAG = "MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Elevator System Starting...");
    
    // Initialize all subsystems
    ESP_LOGI(TAG, "Initializing sensors...");
    sensors_init();
    
    ESP_LOGI(TAG, "Initializing motor control...");
    motor_init();
    
    ESP_LOGI(TAG, "Initializing door control...");
    door_init();
    
    ESP_LOGI(TAG, "Initializing elevator control...");
    elevator_init();
    
    ESP_LOGI(TAG, "System initialized successfully!");
    
    // Create elevator task
    xTaskCreate(elevator_task, "elevator_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Elevator system is now operational.");
}

