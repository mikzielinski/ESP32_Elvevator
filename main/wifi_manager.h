#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>

// Initialize WiFi
void wifi_init(void);

// Check if WiFi is connected
bool wifi_is_connected(void);

// Get IP address string
const char* wifi_get_ip(void);

#endif // WIFI_MANAGER_H

