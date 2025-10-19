#ifndef HX711_CONFIG_H
#define HX711_CONFIG_H

#include "driver/gpio.h"

// HX711 Load Cell Amplifier Pins
#define HX711_DT_PIN GPIO_NUM_13    // Orange wire - Data pin
#define HX711_SCK_PIN GPIO_NUM_14   // Yellow wire - Clock pin

// Calibration values (update after calibration)
#define HX711_CALIBRATION_FACTOR -7050.0  // Adjust during calibration
#define HX711_OFFSET 50682624              // Adjust during calibration

// Measurement settings
#define READINGS_PER_SAMPLE 10  // Number of readings to average
#define UPDATE_INTERVAL_MS 1000 // How often to read weight (milliseconds)

#endif // HX711_CONFIG_H

