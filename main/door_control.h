#ifndef DOOR_CONTROL_H
#define DOOR_CONTROL_H

#include <stdbool.h>

// Door States
typedef enum {
    DOOR_CLOSED,
    DOOR_OPENING,
    DOOR_OPEN,
    DOOR_CLOSING
} door_state_t;

// Function Prototypes
void door_init(void);
void door_open(void);
void door_close(void);
door_state_t door_get_state(void);
bool door_is_open(void);
bool door_is_closed(void);
bool door_check_obstruction(void);

#endif // DOOR_CONTROL_H

