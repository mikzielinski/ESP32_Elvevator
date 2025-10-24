#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdbool.h>
#include "hx711.h"
#include "motor_control.h"

// Initialize web server
void web_server_init(void);

// Send weight update to all connected WebSocket clients
void web_server_send_weight(float weight_kg, long raw_value);

// Set HX711 pointer for zeroing functionality
void web_server_set_hx711(hx711_t* hx711);

// Smart weight processing with 5-second averaging
void web_server_process_weight(float weight_kg, long raw_value);

// Motor control functions
void web_server_set_motor_auto_mode(bool enabled);
bool web_server_get_motor_auto_mode(void);
float web_server_get_weight_threshold(void);
void web_server_set_weight_threshold(float threshold);

#endif // WEB_SERVER_H

