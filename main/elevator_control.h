#ifndef ELEVATOR_CONTROL_H
#define ELEVATOR_CONTROL_H

#include <stdbool.h>
#include "elevator_config.h"

// Elevator States
typedef enum {
    ELEVATOR_IDLE,
    ELEVATOR_MOVING_UP,
    ELEVATOR_MOVING_DOWN,
    ELEVATOR_DOOR_OPENING,
    ELEVATOR_DOOR_OPEN,
    ELEVATOR_DOOR_CLOSING,
    ELEVATOR_EMERGENCY_STOP,
    ELEVATOR_ERROR
} elevator_state_t;

// Elevator Structure
typedef struct {
    elevator_state_t state;
    int current_floor;
    int target_floor;
    bool floor_requests[NUM_FLOORS];
    bool emergency_stop_active;
    bool door_obstructed;
    bool overload_detected;
    uint32_t door_open_start_time;
} elevator_t;

// Function Prototypes
void elevator_init(void);
void elevator_task(void *pvParameters);
void elevator_request_floor(int floor);
void elevator_emergency_stop(void);
void elevator_reset_emergency(void);
int elevator_get_current_floor(void);
elevator_state_t elevator_get_state(void);
void elevator_process_state_machine(void);
bool elevator_has_pending_requests(void);
int elevator_get_next_floor(void);

#endif // ELEVATOR_CONTROL_H

