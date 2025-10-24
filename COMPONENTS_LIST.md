# ESP32 Elevator Control System - Components List

## Main Components

### 1. ESP32 Development Board
- **Model**: ESP32-WROOM-32 or ESP32-DevKitC
- **Specifications**:
  - Dual-core 32-bit processor
  - WiFi and Bluetooth connectivity
  - 34 GPIO pins
  - 3.3V logic level
- **Usage**: Main controller for motor control and WiFi communication
- **Power**: 5V input (regulated from 12V)

### 2. DC Motor
- **Model**: GW4632-370 Worm Gear Motor
- **Specifications**:
  - Voltage: 12V DC
  - Gear ratio: 370:1
  - Worm gear design for high torque
  - Low speed, high torque output
- **Usage**: Primary elevator motor
- **Control**: PWM speed control via DR10050 driver

### 3. Motor Driver
- **Model**: DR10050 UART Motor Driver
- **Specifications**:
  - Input voltage: 12V (motor power)
  - Logic voltage: 5V
  - UART communication interface
  - PWM output for motor control
- **Usage**: Interface between ESP32 and motor
- **Features**: Speed and direction control via UART commands

### 4. Load Cell (HX711)
- **Model**: HX711 24-bit ADC
- **Specifications**:
  - 24-bit precision
  - 3.3V or 5V compatible
  - Serial communication
- **Usage**: Weight measurement for elevator
- **Integration**: Connected to ESP32 via GPIO pins

## Power Supply Components

### 5. AC/DC Power Supply
- **Input**: 220V AC
- **Output**: 12V DC
- **Current**: 10A minimum
- **Usage**: Main power source for the system

### 6. DC/DC Converter (12V to 5V)
- **Model**: Step-down converter module
- **Input**: 12V DC
- **Output**: 5V DC
- **Current**: 3A minimum
- **Usage**: Power supply for ESP32 and logic circuits

### 7. Fuse
- **Rating**: 10A
- **Type**: Fast-blow fuse
- **Usage**: Circuit protection

### 8. Emergency Cut-off Button
- **Type**: Normally closed (NC) push button
- **Rating**: 10A minimum
- **Usage**: Emergency system shutdown

## Power Distribution

### 9. Power Distribution Board
- **12V Distribution**: For motor driver and motor
- **5V Distribution**: For ESP32 and logic circuits
- **Features**: Multiple output terminals

## Communication and Interface

### 10. Level Converter (if needed)
- **Model**: Bi-directional level converter
- **Specifications**: 5V to 3.3V conversion
- **Usage**: UART communication between ESP32 and DR10050

### 11. WiFi Antenna
- **Type**: External antenna (optional)
- **Usage**: Improved WiFi connectivity
- **Connection**: SMA connector to ESP32

## Mechanical Components

### 12. Motor Mounting Bracket
- **Material**: Aluminum or steel
- **Usage**: Secure motor mounting
- **Specifications**: Compatible with GW4632-370 motor

### 13. Coupling
- **Type**: Flexible coupling
- **Usage**: Connect motor to elevator mechanism
- **Specifications**: Match motor shaft diameter

## Safety Components

### 14. Emergency Stop Switch
- **Type**: Red mushroom button
- **Rating**: 10A minimum
- **Usage**: Emergency system shutdown
- **Installation**: Easily accessible location

### 15. Status LEDs
- **Power LED**: System power indication
- **Motor LED**: Motor operation status
- **WiFi LED**: Network connection status

## Cables and Connectors

### 16. Power Cables
- **Motor Power**: 12V, 10A minimum
- **Logic Power**: 5V, 3A minimum
- **Gauge**: 14 AWG for motor, 18 AWG for logic

### 17. Communication Cables
- **UART**: 3-wire cable (TX, RX, GND)
- **Load Cell**: 4-wire cable (VCC, GND, DT, SCK)
- **Length**: As needed for installation

### 18. Connectors
- **Power Connectors**: Screw terminals or XT60
- **Communication**: JST or Dupont connectors
- **Motor**: Appropriate motor connector

## Optional Components

### 19. Display Module
- **Model**: 0.96" OLED or LCD
- **Interface**: I2C or SPI
- **Usage**: System status display

### 20. Buzzer
- **Type**: Piezo buzzer
- **Usage**: Audio alerts and notifications
- **Connection**: GPIO pin on ESP32

### 21. Temperature Sensor
- **Model**: DS18B20 or similar
- **Usage**: Motor temperature monitoring
- **Interface**: One-wire communication

## Tools and Equipment

### 22. Multimeter
- **Usage**: Voltage and current measurements
- **Features**: DC voltage, current, continuity

### 23. Oscilloscope (optional)
- **Usage**: Signal analysis and debugging
- **Features**: PWM signal monitoring

### 24. Wire Strippers
- **Usage**: Cable preparation
- **Specifications**: Various gauge compatibility

## Installation Materials

### 25. Electrical Box
- **Material**: Plastic or metal
- **Size**: Appropriate for all components
- **Features**: Mounting holes, cable entries

### 26. Cable Management
- **Cable Ties**: Organize and secure cables
- **Cable Tray**: Route cables safely
- **Labels**: Identify different connections

## Software Components

### 27. ESP-IDF Framework
- **Version**: Latest stable
- **Usage**: ESP32 development environment
- **Features**: WiFi, UART, GPIO libraries

### 28. Arduino IDE (alternative)
- **Usage**: Alternative development environment
- **Libraries**: ESP32, HX711, WiFi libraries

## Testing Equipment

### 29. Load Cell Tester
- **Usage**: Calibrate weight measurements
- **Features**: Known weight standards

### 30. Motor Test Bench
- **Usage**: Test motor performance
- **Features**: Variable load, speed measurement

## Procurement Notes

- **Quantity**: Order spares for critical components
- **Compatibility**: Verify voltage and current ratings
- **Quality**: Use reputable suppliers for safety components
- **Documentation**: Keep datasheets and manuals

## Cost Estimation

- **ESP32 Board**: $10-15
- **Motor**: $50-100
- **Motor Driver**: $20-30
- **Power Supply**: $20-40
- **Load Cell + HX711**: $10-15
- **Miscellaneous**: $20-30
- **Total Estimated Cost**: $130-230

## Suppliers

- **Electronics**: Amazon, AliExpress, local electronics stores
- **Motors**: Specialized motor suppliers
- **Safety Components**: Electrical supply stores
- **Tools**: Hardware stores, online retailers
