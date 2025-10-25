#ifndef HX711_CONFIG_H
#define HX711_CONFIG_H

#include "driver/gpio.h"

// HX711 Load Cell Amplifier Pins
#define HX711_DT_PIN GPIO_NUM_13    // Orange wire - Data pin (back to original)
#define HX711_SCK_PIN GPIO_NUM_14   // Yellow wire - Clock pin (back to original)

// Calibration values - final calibration with 288g
#define HX711_CALIBRATION_FACTOR 98347.22  // Calibrated with 288g weight (0.288 kg)
#define HX711_OFFSET -104016              // Calibrated offset (zero point)

// Measurement settings
#define READINGS_PER_SAMPLE 3   // Number of readings to average (reduced for faster response)
#define UPDATE_INTERVAL_MS 2000 // How often to read weight (milliseconds) (increased to give HX711 more time)

#endif // HX711_CONFIG_H

