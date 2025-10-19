#ifndef SENSOR_INTERFACE_H
#define SENSOR_INTERFACE_H

#include <stdbool.h>
#include "elevator_config.h"

// Function Prototypes
void sensors_init(void);
bool sensor_read_floor(int floor);
int sensor_detect_current_floor(void);
bool sensor_read_button(int floor);
void sensor_update_leds(bool floor_requests[]);
bool sensor_read_emergency_stop(void);
bool sensor_read_door_obstruction(void);
bool sensor_check_overload(void);
void sensor_set_floor_led(int floor, bool on);

#endif // SENSOR_INTERFACE_H

