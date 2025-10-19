#ifndef ELEVATOR_CONFIG_H
#define ELEVATOR_CONFIG_H

// System Configuration
#define NUM_FLOORS 4
#define GROUND_FLOOR 0

// GPIO Pin Assignments
#define MOTOR_PIN_1 GPIO_NUM_12
#define MOTOR_PIN_2 GPIO_NUM_27
#define MOTOR_PWM_PIN GPIO_NUM_26
#define DOOR_SERVO_PIN GPIO_NUM_15

// HX711 Load Cell Amplifier Pins
#define HX711_DT_PIN GPIO_NUM_13    // Orange - Data pin
#define HX711_SCK_PIN GPIO_NUM_14   // Yellow - Clock pin

// Floor Sensor Pins (Limit Switches)
#define FLOOR_SENSOR_0 GPIO_NUM_16
#define FLOOR_SENSOR_1 GPIO_NUM_17
#define FLOOR_SENSOR_2 GPIO_NUM_18
#define FLOOR_SENSOR_3 GPIO_NUM_19

// Floor Button Pins
#define BUTTON_FLOOR_0 GPIO_NUM_21
#define BUTTON_FLOOR_1 GPIO_NUM_22
#define BUTTON_FLOOR_2 GPIO_NUM_23
#define BUTTON_FLOOR_3 GPIO_NUM_25

// LED Indicator Pins
#define LED_FLOOR_0 GPIO_NUM_26
#define LED_FLOOR_1 GPIO_NUM_27
#define LED_FLOOR_2 GPIO_NUM_32
#define LED_FLOOR_3 GPIO_NUM_33

// Emergency and Safety Pins
#define EMERGENCY_STOP_PIN GPIO_NUM_34
#define DOOR_OBSTRUCTION_PIN GPIO_NUM_35
#define OVERLOAD_SENSOR_PIN GPIO_NUM_36

// Motor Configuration
#define MOTOR_PWM_FREQUENCY 1000  // Hz
#define MOTOR_PWM_RESOLUTION LEDC_TIMER_10_BIT
#define MOTOR_SPEED_NORMAL 700    // Out of 1023
#define MOTOR_SPEED_SLOW 400      // For approaching floor

// Door Configuration
#define DOOR_SERVO_FREQUENCY 50   // Hz (standard servo)
#define DOOR_OPEN_ANGLE 90
#define DOOR_CLOSE_ANGLE 0
#define DOOR_OPEN_TIME_MS 3000    // Time to keep door open

// Timing Configuration
#define FLOOR_TRAVEL_TIME_MS 5000 // Estimated time between floors
#define DEBOUNCE_TIME_MS 50
#define SENSOR_CHECK_INTERVAL_MS 100

// Safety Thresholds
#define MAX_DOOR_OPEN_TIME_MS 30000
#define EMERGENCY_STOP_TIMEOUT_MS 60000
#define OVERLOAD_THRESHOLD_KG 500.0  // Maximum weight in kg
#define HX711_CALIBRATION_FACTOR -7050.0  // Calibration factor (adjust during calibration)
#define HX711_OFFSET 50682624  // Tare offset (adjust during calibration)

#endif // ELEVATOR_CONFIG_H

