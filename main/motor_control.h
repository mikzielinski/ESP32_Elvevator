#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdbool.h>

// Motor Direction
typedef enum {
    MOTOR_STOP,
    MOTOR_UP,
    MOTOR_DOWN
} motor_direction_t;

// Function Prototypes
void motor_init(void);
void motor_set_direction(motor_direction_t direction);
void motor_set_speed(int speed);
void motor_stop(void);
void motor_move_up(int speed);
void motor_move_down(int speed);
motor_direction_t motor_get_direction(void);

#endif // MOTOR_CONTROL_H

