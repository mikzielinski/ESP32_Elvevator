#ifndef HX711_CONFIG_H
#define HX711_CONFIG_H

#include "driver/gpio.h"

// HX711 Load Cell Amplifier Pins
#define HX711_DT_PIN GPIO_NUM_13    // Orange wire - Data pin
#define HX711_SCK_PIN GPIO_NUM_14   // Yellow wire - Clock pin

// Calibration values (calibrated with 631g weight on supported beam)
#define HX711_CALIBRATION_FACTOR 36687.5   // Calibrated: 23150 / 0.631 kg (with beam on supports)
#define HX711_OFFSET -104190               // Tare value (no weight, beam on supports)

// Measurement settings
#define READINGS_PER_SAMPLE 3   // Number of readings to average (reduced for faster response)
#define UPDATE_INTERVAL_MS 2000 // How often to read weight (milliseconds) (increased to give HX711 more time)

#endif // HX711_CONFIG_H

