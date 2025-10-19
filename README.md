# ESP32-Elevator

ESP32-based elevator control system

## Overview

This project implements an elevator control system using the ESP32 microcontroller. The system manages elevator operations including floor selection, door control, safety features, and motor control.

## Features

- Multi-floor elevator control
- Floor selection buttons with LED indicators
- Door open/close control
- Emergency stop functionality
- Overload detection
- Position tracking
- Real-time status monitoring

## Hardware Requirements

- ESP32 development board (ESP32-DevKitC or similar)
- Motor driver module (L298N or similar)
- DC motor for elevator movement
- Door servo motor
- Floor sensors (limit switches or IR sensors)
- Push buttons for floor selection
- LED indicators
- Emergency stop button
- **HX711 Load Cell Amplifier** - connected to GPIO13 (DT-orange) and GPIO14 (SCK-yellow)
- **Load Cell (Tensometric Beam)** - for weight/overload detection

## Software Requirements

- ESP-IDF (Espressif IoT Development Framework) v5.0 or later
- Python 3.7 or later
- Git

## Getting Started

### 1. Install ESP-IDF

Follow the official ESP-IDF installation guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

### 2. Clone the Repository

```bash
git clone <repository-url>
cd ESP32-Elevator
```

### 3. Configure the Project

```bash
idf.py menuconfig
```

### 4. Build the Project

```bash
idf.py build
```

### 5. Flash to ESP32

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with your ESP32's serial port.

## Project Structure

```
ESP32-Elevator/
├── main/
│   ├── main.c              # Main application entry point
│   ├── elevator_control.c  # Elevator logic
│   ├── elevator_control.h
│   ├── motor_control.c     # Motor driver interface
│   ├── motor_control.h
│   ├── door_control.c      # Door servo control
│   ├── door_control.h
│   ├── sensor_interface.c  # Floor sensors and buttons
│   ├── sensor_interface.h
│   └── CMakeLists.txt
├── CMakeLists.txt
├── README.md
└── sdkconfig
```

## Configuration

Edit `main/elevator_config.h` to configure:
- Number of floors
- GPIO pin assignments
- Motor parameters
- Safety timeouts
- Sensor thresholds
- **HX711 calibration values** (see CALIBRATION_GUIDE.md)

### Load Cell Calibration

Before first use, calibrate the HX711 load cell:

1. See detailed instructions in `CALIBRATION_GUIDE.md`
2. Run calibration procedure to get OFFSET and CALIBRATION_FACTOR
3. Update values in `main/elevator_config.h`

Current HX711 pin assignment:
- **DT (Data)**: GPIO13 (Orange wire)
- **SCK (Clock)**: GPIO14 (Yellow wire)

## Usage

1. Power on the ESP32
2. The elevator will initialize and move to the ground floor
3. Press floor buttons to select destination
4. The elevator will queue multiple requests
5. Use the emergency stop button in case of emergency

## Safety Features

- Emergency stop override
- Door obstruction detection
- Overload protection
- Floor limit switches
- Motor current monitoring
- Timeout protections

## Development

### Adding New Features

1. Create feature branch: `git checkout -b feature/your-feature`
2. Make changes and test thoroughly
3. Commit changes: `git commit -m "Add your feature"`
4. Push branch: `git push origin feature/your-feature`

### Testing

Always test on the hardware before deploying to production. Use the monitor for debugging:

```bash
idf.py monitor
```

## Troubleshooting

### Elevator doesn't move
- Check motor driver connections
- Verify GPIO pin configuration
- Check power supply

### Incorrect floor detection
- Calibrate floor sensors
- Check sensor wiring
- Adjust sensor thresholds in config

### Door issues
- Check servo connections
- Verify servo power supply
- Adjust PWM parameters

## License

MIT License - See LICENSE file for details

## Contributing

Contributions are welcome! Please open an issue or submit a pull request.

## Contact

For questions or support, please open an issue on GitHub.

