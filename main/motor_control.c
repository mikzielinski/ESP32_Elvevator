#include "motor_control.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MOTOR_CONTROL";
static motor_state_t current_motor_state = MOTOR_STATE_STOPPED;

// CRC calculation for DRI0050 (returns raw CRC)
uint16_t calculate_crc(const uint8_t *data, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t pos = 0; pos < len; pos++) {
        crc ^= data[pos];
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else         crc >>= 1;
        }
    }
    return crc;  // Return raw CRC
}

void motor_control_init(void)
{
    // Configure UART for motor control
    uart_config_t uart_config = {
        .baud_rate = MOTOR_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Install UART driver
    uart_driver_install(MOTOR_UART_NUM, 1024, 1024, 0, NULL, 0);
    uart_param_config(MOTOR_UART_NUM, &uart_config);
    uart_set_pin(MOTOR_UART_NUM, MOTOR_TX_PIN, MOTOR_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(TAG, "Motor control initialized on UART%d (TX=GPIO%d, RX=GPIO%d)", 
             MOTOR_UART_NUM, MOTOR_TX_PIN, MOTOR_RX_PIN);
    
    // Initialize PWM according to DFRobot documentation
    // Step 1: Set PWM frequency (register 0x0007)
    uint8_t freq_cmd[8];
    freq_cmd[0] = MODBUS_DEV_ADDR;
    freq_cmd[1] = MODBUS_FUNC_WRITE;
    freq_cmd[2] = (MODBUS_REG_FREQ >> 8) & 0xFF;
    freq_cmd[3] = MODBUS_REG_FREQ & 0xFF;
    freq_cmd[4] = 0x00;
    freq_cmd[5] = 0x64;  // Frequency = 100 Hz (increased from 10)
    uint16_t crc = calculate_crc(freq_cmd, 6);
    freq_cmd[6] = crc & 0xFF;          // CRC_L
    freq_cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    uart_write_bytes(MOTOR_UART_NUM, freq_cmd, 8);
    ESP_LOGI(TAG, "PWM Frequency set to 100 Hz");
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Step 2: Set PWM duty to 0 (register 0x0006)
    uint8_t duty_cmd[8];
    duty_cmd[0] = MODBUS_DEV_ADDR;
    duty_cmd[1] = MODBUS_FUNC_WRITE;
    duty_cmd[2] = (MODBUS_REG_DUTY >> 8) & 0xFF;
    duty_cmd[3] = MODBUS_REG_DUTY & 0xFF;
    duty_cmd[4] = 0x00;
    duty_cmd[5] = 0x00;  // Duty = 0
    crc = calculate_crc(duty_cmd, 6);
    duty_cmd[6] = crc & 0xFF;          // CRC_L
    duty_cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    uart_write_bytes(MOTOR_UART_NUM, duty_cmd, 8);
    ESP_LOGI(TAG, "PWM Duty set to 0");
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Step 3: Enable PWM output (register 0x0008)
    uint8_t enable_cmd[8];
    enable_cmd[0] = MODBUS_DEV_ADDR;
    enable_cmd[1] = MODBUS_FUNC_WRITE;
    enable_cmd[2] = (MODBUS_REG_ENABLE >> 8) & 0xFF;
    enable_cmd[3] = MODBUS_REG_ENABLE & 0xFF;
    enable_cmd[4] = 0x00;
    enable_cmd[5] = 0x01;  // Enable = 1
    crc = calculate_crc(enable_cmd, 6);
    enable_cmd[6] = crc & 0xFF;          // CRC_L
    enable_cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    uart_write_bytes(MOTOR_UART_NUM, enable_cmd, 8);
    ESP_LOGI(TAG, "PWM Output ENABLED");
    vTaskDelay(pdMS_TO_TICKS(50));
    
    ESP_LOGI(TAG, "Motor driver PWM initialized successfully!");
}

void motor_start_forward(void)
{
    // Set direction register first, then duty
    uint8_t cmd[8];
    
    // Step 1: Set direction register (0x000A) to 0 for forward
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = 0x00;                // Register high byte
    cmd[3] = 0x0A;                // Register low byte (0x000A - direction register)
    cmd[4] = 0x00;                // Value high byte (0)
    cmd[5] = 0x00;                // Value low byte (0 = forward direction)
    
    uint16_t crc = calculate_crc(cmd, 6);
    cmd[6] = crc & 0xFF;          // CRC_L
    cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    
    ESP_LOGI(TAG, "Sending FORWARD direction command (0x000A=0): %02x %02x %02x %02x %02x %02x %02x %02x", 
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Step 2: Set duty for forward
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = (MODBUS_REG_DUTY >> 8) & 0xFF;  // 0x00
    cmd[3] = MODBUS_REG_DUTY & 0xFF;         // 0x06
    cmd[4] = 0x00;                // Value high byte (0)
    cmd[5] = 0xFF;                // Value low byte (255 = 100% duty)
    
    crc = calculate_crc(cmd, 6);
    cmd[6] = crc & 0xFF;          // CRC_L
    cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    
    ESP_LOGI(TAG, "Sending FORWARD duty command: %02x %02x %02x %02x %02x %02x %02x %02x", 
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    int bytes_written = uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    ESP_LOGI(TAG, "Bytes written to UART: %d", bytes_written);
    vTaskDelay(pdMS_TO_TICKS(30)); // 30ms delay as per DRI0050 documentation
    current_motor_state = MOTOR_STATE_FORWARD;
    ESP_LOGI(TAG, "Motor started FORWARD");
}

void motor_start_backward(void)
{
    // Try different approach: Set direction register first, then duty
    uint8_t cmd[8];
    
    // Step 1: Set direction register (0x000A) to 1 for reverse
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = 0x00;                // Register high byte
    cmd[3] = 0x0A;                // Register low byte (0x000A - direction register)
    cmd[4] = 0x00;                // Value high byte (0)
    cmd[5] = 0x01;                // Value low byte (1 = reverse direction)
    
    uint16_t crc = calculate_crc(cmd, 6);
    cmd[6] = crc & 0xFF;          // CRC_L
    cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    
    ESP_LOGI(TAG, "Sending BACKWARD direction command (0x000A=1): %02x %02x %02x %02x %02x %02x %02x %02x", 
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // Step 2: Set duty for backward
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = (MODBUS_REG_DUTY >> 8) & 0xFF;  // 0x00
    cmd[3] = MODBUS_REG_DUTY & 0xFF;         // 0x06
    cmd[4] = 0x00;                // Value high byte (0)
    cmd[5] = 0xFF;                // Value low byte (255 = 100% duty)
    
    crc = calculate_crc(cmd, 6);
    cmd[6] = crc & 0xFF;          // CRC_L
    cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    
    ESP_LOGI(TAG, "Sending BACKWARD duty command: %02x %02x %02x %02x %02x %02x %02x %02x", 
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    int bytes_written = uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    ESP_LOGI(TAG, "Bytes written to UART: %d", bytes_written);
    vTaskDelay(pdMS_TO_TICKS(30)); // 30ms delay as per DRI0050 documentation
    current_motor_state = MOTOR_STATE_BACKWARD;
    ESP_LOGI(TAG, "Motor started BACKWARD");
}

void motor_stop(void)
{
    // Build ModBus RTU frame: [DEV_ADDR][FUNC][REG_HI][REG_LO][VAL_HI][VAL_LO][CRC_HI][CRC_LO]
    uint8_t cmd[8];
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = (MODBUS_REG_DUTY >> 8) & 0xFF;  // 0x00
    cmd[3] = MODBUS_REG_DUTY & 0xFF;         // 0x06
    cmd[4] = 0x00;  // Value high byte (0)
    cmd[5] = 0x00;  // Value low byte (0 = stop)
    
    // Calculate CRC for first 6 bytes
    uint16_t crc = calculate_crc(cmd, 6);
    cmd[6] = crc & 0xFF;          // CRC_L
    cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    
    ESP_LOGI(TAG, "Sending STOP command (ModBus RTU): %02x %02x %02x %02x %02x %02x %02x %02x", 
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    int bytes_written = uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    ESP_LOGI(TAG, "Bytes written to UART: %d", bytes_written);
    vTaskDelay(pdMS_TO_TICKS(30)); // 30ms delay as per DRI0050 documentation
    current_motor_state = MOTOR_STATE_STOPPED;
    ESP_LOGI(TAG, "Motor STOPPED");
}

motor_state_t motor_get_state(void)
{
    return current_motor_state;
}

void motor_set_state(motor_state_t state)
{
    switch (state) {
        case MOTOR_STATE_FORWARD:
            motor_start_forward();
            break;
        case MOTOR_STATE_BACKWARD:
            motor_start_backward();
            break;
        case MOTOR_STATE_STOPPED:
        default:
            motor_stop();
            break;
    }
}

void motor_test_commands(void)
{
    ESP_LOGI(TAG, "=== TESTING MODBUS RTU MOTOR COMMANDS ===");
    
    // Test 1: Forward command
    ESP_LOGI(TAG, "Test 1: Forward command");
    uint8_t forward_cmd[] = MOTOR_FORWARD_CMD_MODBUS;
    uart_write_bytes(MOTOR_UART_NUM, forward_cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Test 2: Stop command
    ESP_LOGI(TAG, "Test 2: Stop command");
    uint8_t stop_cmd[] = MOTOR_STOP_CMD_MODBUS;
    uart_write_bytes(MOTOR_UART_NUM, stop_cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Test 3: Backward command
    ESP_LOGI(TAG, "Test 3: Backward command");
    uint8_t backward_cmd[] = MOTOR_BACKWARD_CMD_MODBUS;
    uart_write_bytes(MOTOR_UART_NUM, backward_cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Test 4: Stop again
    ESP_LOGI(TAG, "Test 4: Stop again");
    uart_write_bytes(MOTOR_UART_NUM, stop_cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    ESP_LOGI(TAG, "=== MODBUS RTU MOTOR COMMAND TESTS COMPLETED ===");
}

void motor_check_power(void)
{
    ESP_LOGI(TAG, "=== CHECKING MOTOR POWER STATUS ===");
    
    // Read ENABLE register (0x0008)
    uint8_t read_enable_cmd[8];
    read_enable_cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    read_enable_cmd[1] = 0x03;                // Read Holding Registers
    read_enable_cmd[2] = 0x00;                 // Register high byte
    read_enable_cmd[3] = 0x08;                 // Register low byte (0x0008)
    read_enable_cmd[4] = 0x00;                 // Number of registers high
    read_enable_cmd[5] = 0x01;                 // Number of registers low (1 register)
    
    uint16_t crc = calculate_crc(read_enable_cmd, 6);
    read_enable_cmd[6] = crc & 0xFF;           // CRC_L
    read_enable_cmd[7] = (crc >> 8) & 0xFF;    // CRC_H
    
    ESP_LOGI(TAG, "Reading ENABLE register: %02x %02x %02x %02x %02x %02x %02x %02x", 
             read_enable_cmd[0], read_enable_cmd[1], read_enable_cmd[2], read_enable_cmd[3], 
             read_enable_cmd[4], read_enable_cmd[5], read_enable_cmd[6], read_enable_cmd[7]);
    
    uart_write_bytes(MOTOR_UART_NUM, read_enable_cmd, 8);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Read DUTY register (0x0006)
    uint8_t read_duty_cmd[8];
    read_duty_cmd[0] = MODBUS_DEV_ADDR;        // 0x32
    read_duty_cmd[1] = 0x03;                   // Read Holding Registers
    read_duty_cmd[2] = 0x00;                   // Register high byte
    read_duty_cmd[3] = 0x06;                   // Register low byte (0x0006)
    read_duty_cmd[4] = 0x00;                   // Number of registers high
    read_duty_cmd[5] = 0x01;                   // Number of registers low (1 register)
    
    crc = calculate_crc(read_duty_cmd, 6);
    read_duty_cmd[6] = crc & 0xFF;             // CRC_L
    read_duty_cmd[7] = (crc >> 8) & 0xFF;      // CRC_H
    
    ESP_LOGI(TAG, "Reading DUTY register: %02x %02x %02x %02x %02x %02x %02x %02x", 
             read_duty_cmd[0], read_duty_cmd[1], read_duty_cmd[2], read_duty_cmd[3], 
             read_duty_cmd[4], read_duty_cmd[5], read_duty_cmd[6], read_duty_cmd[7]);
    
    uart_write_bytes(MOTOR_UART_NUM, read_duty_cmd, 8);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    ESP_LOGI(TAG, "Power status check commands sent to motor driver");
    ESP_LOGI(TAG, "=== MOTOR POWER STATUS CHECK COMPLETED ===");
}

// Set motor speed (0-100%)
void motor_set_speed(uint8_t speed_percent)
{
    if (speed_percent > 100) speed_percent = 100;
    
    uint16_t duty_value = (speed_percent * 255) / 100;  // Convert to 0-255 range
    
    uint8_t cmd[8];
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = (MODBUS_REG_DUTY >> 8) & 0xFF;  // 0x00
    cmd[3] = MODBUS_REG_DUTY & 0xFF;         // 0x06
    cmd[4] = (duty_value >> 8) & 0xFF;       // High byte
    cmd[5] = duty_value & 0xFF;              // Low byte
    
    uint16_t crc = calculate_crc(cmd, 6);
    cmd[6] = crc & 0xFF;          // CRC_L
    cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    
    ESP_LOGI(TAG, "Setting motor speed to %d%% (duty: %d): %02x %02x %02x %02x %02x %02x %02x %02x", 
             speed_percent, duty_value, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(30));
}

// Set motor frequency (1-1000 Hz)
void motor_set_frequency(uint16_t freq_hz)
{
    if (freq_hz < 1) freq_hz = 1;
    if (freq_hz > 1000) freq_hz = 1000;
    
    uint8_t cmd[8];
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = (MODBUS_REG_FREQ >> 8) & 0xFF;  // 0x00
    cmd[3] = MODBUS_REG_FREQ & 0xFF;         // 0x07
    cmd[4] = (freq_hz >> 8) & 0xFF;          // High byte
    cmd[5] = freq_hz & 0xFF;                 // Low byte
    
    uint16_t crc = calculate_crc(cmd, 6);
    cmd[6] = crc & 0xFF;          // CRC_L
    cmd[7] = (crc >> 8) & 0xFF;   // CRC_H
    
    ESP_LOGI(TAG, "Setting motor frequency to %d Hz: %02x %02x %02x %02x %02x %02x %02x %02x", 
             freq_hz, cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    vTaskDelay(pdMS_TO_TICKS(30));
}

// High speed forward (100% speed)
void motor_forward_fast(void)
{
    ESP_LOGI(TAG, "Starting motor FORWARD at HIGH SPEED");
    motor_start_forward();
    motor_set_speed(100);  // 100% speed
}

// High speed backward (100% speed)
void motor_backward_fast(void)
{
    ESP_LOGI(TAG, "Starting motor BACKWARD at HIGH SPEED");
    motor_start_backward();
    motor_set_speed(100);  // 100% speed
}

// Medium speed forward (50% speed)
void motor_forward_medium(void)
{
    ESP_LOGI(TAG, "Starting motor FORWARD at MEDIUM SPEED");
    motor_start_forward();
    motor_set_speed(50);  // 50% speed
}

// Medium speed backward (50% speed)
void motor_backward_medium(void)
{
    ESP_LOGI(TAG, "Starting motor BACKWARD at MEDIUM SPEED");
    motor_start_backward();
    motor_set_speed(50);  // 50% speed
}