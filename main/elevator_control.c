#include "elevator_control.h"
#include "motor_control.h"
#include "door_control.h"
#include "sensor_interface.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "ELEVATOR_CTRL";
static elevator_t elevator;

void elevator_init(void)
{
    memset(&elevator, 0, sizeof(elevator_t));
    elevator.state = ELEVATOR_IDLE;
    elevator.current_floor = 0;
    elevator.target_floor = 0;
    
    // Detect initial floor position
    int detected_floor = sensor_detect_current_floor();
    if (detected_floor >= 0) {
        elevator.current_floor = detected_floor;
        ESP_LOGI(TAG, "Elevator initialized at floor %d", detected_floor);
    } else {
        ESP_LOGI(TAG, "Floor position unknown, moving to ground floor");
        elevator_request_floor(GROUND_FLOOR);
    }
}

void elevator_request_floor(int floor)
{
    if (floor >= 0 && floor < NUM_FLOORS) {
        elevator.floor_requests[floor] = true;
        sensor_set_floor_led(floor, true);
        ESP_LOGI(TAG, "Floor %d requested", floor);
    }
}

void elevator_emergency_stop(void)
{
    elevator.emergency_stop_active = true;
    elevator.state = ELEVATOR_EMERGENCY_STOP;
    motor_stop();
    ESP_LOGW(TAG, "EMERGENCY STOP ACTIVATED!");
}

void elevator_reset_emergency(void)
{
    elevator.emergency_stop_active = false;
    elevator.state = ELEVATOR_IDLE;
    ESP_LOGI(TAG, "Emergency stop reset");
}

int elevator_get_current_floor(void)
{
    return elevator.current_floor;
}

elevator_state_t elevator_get_state(void)
{
    return elevator.state;
}

bool elevator_has_pending_requests(void)
{
    for (int i = 0; i < NUM_FLOORS; i++) {
        if (elevator.floor_requests[i]) {
            return true;
        }
    }
    return false;
}

int elevator_get_next_floor(void)
{
    // Simple algorithm: serve closest floor first
    int min_distance = NUM_FLOORS + 1;
    int next_floor = -1;
    
    for (int i = 0; i < NUM_FLOORS; i++) {
        if (elevator.floor_requests[i]) {
            int distance = abs(i - elevator.current_floor);
            if (distance < min_distance) {
                min_distance = distance;
                next_floor = i;
            }
        }
    }
    
    return next_floor;
}

void elevator_process_state_machine(void)
{
    // Check safety conditions
    if (sensor_read_emergency_stop() && !elevator.emergency_stop_active) {
        elevator_emergency_stop();
        return;
    }
    
    elevator.overload_detected = sensor_check_overload();
    elevator.door_obstructed = sensor_read_door_obstruction();
    
    switch (elevator.state) {
        case ELEVATOR_IDLE:
            if (elevator_has_pending_requests()) {
                int next_floor = elevator_get_next_floor();
                if (next_floor >= 0) {
                    elevator.target_floor = next_floor;
                    
                    if (next_floor > elevator.current_floor) {
                        elevator.state = ELEVATOR_MOVING_UP;
                        motor_move_up(MOTOR_SPEED_NORMAL);
                        ESP_LOGI(TAG, "Moving up to floor %d", next_floor);
                    } else if (next_floor < elevator.current_floor) {
                        elevator.state = ELEVATOR_MOVING_DOWN;
                        motor_move_down(MOTOR_SPEED_NORMAL);
                        ESP_LOGI(TAG, "Moving down to floor %d", next_floor);
                    } else {
                        // Already at requested floor
                        elevator.state = ELEVATOR_DOOR_OPENING;
                    }
                }
            }
            break;
            
        case ELEVATOR_MOVING_UP:
            // Check if we reached the target floor
            if (sensor_read_floor(elevator.target_floor)) {
                motor_stop();
                elevator.current_floor = elevator.target_floor;
                elevator.floor_requests[elevator.target_floor] = false;
                sensor_set_floor_led(elevator.target_floor, false);
                elevator.state = ELEVATOR_DOOR_OPENING;
                ESP_LOGI(TAG, "Arrived at floor %d", elevator.current_floor);
            }
            break;
            
        case ELEVATOR_MOVING_DOWN:
            // Check if we reached the target floor
            if (sensor_read_floor(elevator.target_floor)) {
                motor_stop();
                elevator.current_floor = elevator.target_floor;
                elevator.floor_requests[elevator.target_floor] = false;
                sensor_set_floor_led(elevator.target_floor, false);
                elevator.state = ELEVATOR_DOOR_OPENING;
                ESP_LOGI(TAG, "Arrived at floor %d", elevator.current_floor);
            }
            break;
            
        case ELEVATOR_DOOR_OPENING:
            door_open();
            elevator.state = ELEVATOR_DOOR_OPEN;
            elevator.door_open_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
            ESP_LOGI(TAG, "Door opening");
            break;
            
        case ELEVATOR_DOOR_OPEN:
            {
                uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
                uint32_t door_open_duration = current_time - elevator.door_open_start_time;
                
                // Check if door should close
                if (door_open_duration >= DOOR_OPEN_TIME_MS && !elevator.door_obstructed) {
                    if (!elevator.overload_detected) {
                        elevator.state = ELEVATOR_DOOR_CLOSING;
                        ESP_LOGI(TAG, "Door closing");
                    } else {
                        ESP_LOGW(TAG, "Overload detected, keeping door open");
                        // Reset timer to keep door open
                        elevator.door_open_start_time = current_time;
                    }
                }
                
                // Safety timeout
                if (door_open_duration >= MAX_DOOR_OPEN_TIME_MS) {
                    ESP_LOGW(TAG, "Door open timeout, forcing close");
                    elevator.state = ELEVATOR_DOOR_CLOSING;
                }
            }
            break;
            
        case ELEVATOR_DOOR_CLOSING:
            if (elevator.door_obstructed) {
                ESP_LOGW(TAG, "Door obstruction detected, reopening");
                elevator.state = ELEVATOR_DOOR_OPENING;
            } else {
                door_close();
                if (door_is_closed()) {
                    elevator.state = ELEVATOR_IDLE;
                    ESP_LOGI(TAG, "Door closed, elevator idle");
                }
            }
            break;
            
        case ELEVATOR_EMERGENCY_STOP:
            // Wait for manual reset
            if (!sensor_read_emergency_stop()) {
                elevator_reset_emergency();
            }
            break;
            
        case ELEVATOR_ERROR:
            ESP_LOGE(TAG, "Elevator in error state");
            motor_stop();
            // Could implement error recovery here
            break;
    }
}

void elevator_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Elevator task started");
    
    while (1) {
        // Check for button presses
        for (int i = 0; i < NUM_FLOORS; i++) {
            if (sensor_read_button(i)) {
                elevator_request_floor(i);
            }
        }
        
        // Update LED indicators
        sensor_update_leds(elevator.floor_requests);
        
        // Process state machine
        elevator_process_state_machine();
        
        // Small delay
        vTaskDelay(pdMS_TO_TICKS(SENSOR_CHECK_INTERVAL_MS));
    }
}

