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
    freq_cmd[5] = 0x0A;  // Frequency = 10 (from Arduino example)
    uint16_t crc = calculate_crc(freq_cmd, 6);
    freq_cmd[6] = (crc >> 8) & 0xFF;   // CRC_H (swap for DRI0050 HIGH→LOW)
    freq_cmd[7] = crc & 0xFF;          // CRC_L (swap for DRI0050 HIGH→LOW)
    uart_write_bytes(MOTOR_UART_NUM, freq_cmd, 8);
    ESP_LOGI(TAG, "PWM Frequency set to 10");
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
    duty_cmd[6] = crc & 0xFF;          // CRC_L (already swapped in calculate_crc)
    duty_cmd[7] = (crc >> 8) & 0xFF;   // CRC_H (already swapped in calculate_crc)
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
    enable_cmd[6] = crc & 0xFF;          // CRC_L (already swapped in calculate_crc)
    enable_cmd[7] = (crc >> 8) & 0xFF;   // CRC_H (already swapped in calculate_crc)
    uart_write_bytes(MOTOR_UART_NUM, enable_cmd, 8);
    ESP_LOGI(TAG, "PWM Output ENABLED");
    vTaskDelay(pdMS_TO_TICKS(50));
    
    ESP_LOGI(TAG, "Motor driver PWM initialized successfully!");
}

void motor_start_forward(void)
{
    // Build ModBus RTU frame: [DEV_ADDR][FUNC][REG_HI][REG_LO][VAL_HI][VAL_LO][CRC_HI][CRC_LO]
    uint8_t cmd[8];
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = (MODBUS_REG_DUTY >> 8) & 0xFF;  // 0x00
    cmd[3] = MODBUS_REG_DUTY & 0xFF;         // 0x06
    cmd[4] = 0x00;  // Value high byte (0)
    cmd[5] = 0xFF;  // Value low byte (255 = 100% duty)
    
    // Calculate CRC for first 6 bytes
    uint16_t crc = calculate_crc(cmd, 6);
    cmd[6] = (crc >> 8) & 0xFF;   // CRC_H (swap for DRI0050 HIGH→LOW)
    cmd[7] = crc & 0xFF;          // CRC_L (swap for DRI0050 HIGH→LOW)
    
    ESP_LOGI(TAG, "Sending FORWARD command (ModBus RTU): %02x %02x %02x %02x %02x %02x %02x %02x", 
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5], cmd[6], cmd[7]);
    int bytes_written = uart_write_bytes(MOTOR_UART_NUM, cmd, MOTOR_CMD_LEN);
    ESP_LOGI(TAG, "Bytes written to UART: %d", bytes_written);
    vTaskDelay(pdMS_TO_TICKS(30)); // 30ms delay as per DRI0050 documentation
    current_motor_state = MOTOR_STATE_FORWARD;
    ESP_LOGI(TAG, "Motor started FORWARD");
}

void motor_start_backward(void)
{
    // Build ModBus RTU frame: [DEV_ADDR][FUNC][REG_HI][REG_LO][VAL_HI][VAL_LO][CRC_HI][CRC_LO]
    uint8_t cmd[8];
    cmd[0] = MODBUS_DEV_ADDR;     // 0x32
    cmd[1] = MODBUS_FUNC_WRITE;   // 0x06
    cmd[2] = (MODBUS_REG_DUTY >> 8) & 0xFF;  // 0x00
    cmd[3] = MODBUS_REG_DUTY & 0xFF;         // 0x06
    cmd[4] = 0x00;  // Value high byte (0)
    cmd[5] = 0xFF;  // Value low byte (255 = 100% duty)
    
    // Calculate CRC for first 6 bytes
    uint16_t crc = calculate_crc(cmd, 6);
    cmd[6] = (crc >> 8) & 0xFF;   // CRC high byte
    cmd[7] = crc & 0xFF;            // CRC low byte
    
    ESP_LOGI(TAG, "Sending BACKWARD command (ModBus RTU): %02x %02x %02x %02x %02x %02x %02x %02x", 
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
    cmd[6] = (crc >> 8) & 0xFF;   // CRC_H (swap for DRI0050 HIGH→LOW)
    cmd[7] = crc & 0xFF;          // CRC_L (swap for DRI0050 HIGH→LOW)
    
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