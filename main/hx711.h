#ifndef HX711_H
#define HX711_H

#include <stdbool.h>
#include "driver/gpio.h"

// HX711 Gain settings
typedef enum {
    HX711_GAIN_128 = 1,  // Channel A, gain 128
    HX711_GAIN_32 = 2,   // Channel B, gain 32
    HX711_GAIN_64 = 3    // Channel A, gain 64
} hx711_gain_t;

// HX711 structure
typedef struct {
    gpio_num_t dout_pin;
    gpio_num_t sck_pin;
    hx711_gain_t gain;
    long offset;
    float scale;
} hx711_t;

// Function prototypes
void hx711_init(hx711_t* hx711, gpio_num_t dout, gpio_num_t sck);
bool hx711_is_ready(hx711_t* hx711);
long hx711_read(hx711_t* hx711);
long hx711_read_average(hx711_t* hx711, int times);
void hx711_set_gain(hx711_t* hx711, hx711_gain_t gain);
void hx711_tare(hx711_t* hx711, int times);
void hx711_set_scale(hx711_t* hx711, float scale);
void hx711_set_offset(hx711_t* hx711, long offset);
void hx711_zero_scale(hx711_t* hx711);
float hx711_get_units(hx711_t* hx711, int times);
float hx711_get_value(hx711_t* hx711, int times);
void hx711_power_down(hx711_t* hx711);
void hx711_power_up(hx711_t* hx711);

#endif // HX711_H

