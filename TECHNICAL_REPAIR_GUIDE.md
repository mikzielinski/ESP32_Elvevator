# ESP32-Elevator - Instrukcja naprawy i konfiguracji

## 🚀 Instrukcja naprawy i konfiguracji projektu ESP32-Elevator

**(Silnik DFRobot DRI0050 + ESP32-S3 + czujnik HX711)**

## 1️⃣ Problem, który trzeba naprawić

Silnik DC nie kręci, ponieważ sterownik DFRobot DRI0050 nie reaguje na ramki UART.

**Przyczyna:**
→ Ramki Modbus RTU wysyłane przez ESP32 nie mają poprawnego CRC-16 (Modbus).
→ Sterownik odrzuca komendy typu `0x32 0x06 0x00 0x06 ....`

## 2️⃣ Co musisz zrobić

### ➤ A. Dodać obliczanie CRC-16 (Modbus RTU)

Utwórz nowy plik `main/motor_control.c` i wklej poniższy kod:

```c
#include "motor_control.h"
#include "driver/uart.h"
#include <stdio.h>

uint16_t modbus_crc16(const uint8_t *buffer, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++) {
        crc ^= buffer[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

void send_modbus_cmd(uint8_t dev, uint8_t func, uint16_t reg, uint16_t val) {
    uint8_t frame[8];
    frame[0] = dev;
    frame[1] = func;
    frame[2] = reg >> 8;
    frame[3] = reg & 0xFF;
    frame[4] = val >> 8;
    frame[5] = val & 0xFF;

    uint16_t crc = modbus_crc16(frame, 6);
    frame[6] = crc & 0xFF;       // CRC Low
    frame[7] = crc >> 8;         // CRC High

    uart_write_bytes(MOTOR_UART_NUM, (const char*)frame, 8);
}

void motor_init() {
    uart_config_t uart_config = {
        .baud_rate = MOTOR_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(MOTOR_UART_NUM, &uart_config);
    uart_set_pin(MOTOR_UART_NUM, MOTOR_TX_PIN, MOTOR_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(MOTOR_UART_NUM, 256, 0, 0, NULL, 0);
}

void motor_stop() {
    send_modbus_cmd(MODBUS_DEV_ADDR, MODBUS_FUNC_WRITE, MODBUS_REG_DUTY, 0);
}

void motor_set_speed(uint16_t duty) {
    send_modbus_cmd(MODBUS_DEV_ADDR, MODBUS_FUNC_WRITE, MODBUS_REG_DUTY, duty);
}
```

### ➤ B. Upewnij się, że `motor_control.h` zawiera:

```c
#define MOTOR_UART_NUM UART_NUM_1
#define MOTOR_TX_PIN GPIO_NUM_12
#define MOTOR_RX_PIN GPIO_NUM_11
#define MOTOR_BAUD_RATE 9600

#define MODBUS_DEV_ADDR 0x32
#define MODBUS_FUNC_WRITE 0x06
#define MODBUS_REG_DUTY 0x0006
#define MODBUS_REG_FREQ 0x0007
```

### ➤ C. Zainicjuj silnik w `app_main()`

```c
void app_main(void) {
    motor_init();
    motor_set_speed(200);  // ok. 80% mocy
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    motor_stop();
}
```

## 3️⃣ Konfiguracja środowiska

### ESP-IDF Framework

Projekt korzysta z **ESP-IDF** (nie Arduino).
Nie wymaga żadnych zewnętrznych bibliotek.

### ✅ Wymagane komponenty (wbudowane):

- `driver/uart`
- `driver/gpio`
- `freertos`
- `esp_wifi`
- `esp_http_server`

### Python testy (opcjonalne)

Aby uruchomić testy z PC:

```bash
pip install pyserial
python3 test_motor_final.py
```

## 4️⃣ Zasilanie i połączenia

| Element | Napięcie | Uwagi |
|---------|----------|-------|
| DRI0050 VIN | 12 V | z zasilacza przez bezpiecznik 10 A |
| ESP32 5 V | z przetwornicy 12 → 5 V | logika |
| GND | wspólna masa 5 V i 12 V | obowiązkowo |
| UART TX/RX | TX ESP32 → RX DRI0050, RX ESP32 ← TX DRI0050 | 3.3 V logiczne |
| M+ / M– | silnik DC 12 V | kierunek stały |

## 5️⃣ Sprawdzenie działania

1. **Wgraj nowy firmware**
2. **W monitorze UART** zobacz, czy wysyłane są ramki:
   ```
   32 06 00 06 00 C8 CRC_L CRC_H
   ```
3. **Dioda na sterowniku** powinna migać, a silnik zacząć się kręcić

### Jeśli nie, sprawdź:

- [ ] wspólną masę
- [ ] kierunek TX/RX
- [ ] napięcie VIN 12 V

## 6️⃣ Dodatkowe zalecenia dla CursorAI

- **Framework**: ESP-IDF (nie Arduino)
- **Nowy plik**: `main/motor_control.c` z funkcjami CRC i sterowania
- **Test**: użyj `idf.py monitor` i obserwuj UART
- **Debug**: w razie braku reakcji, obniż duty do 128 lub sprawdź CRC (powinno być jak w dokumentacji DRI0050)

## ✅ 7️⃣ Szybka checklista

- [ ] Wspólna masa 5 V i 12 V
- [ ] TX↔RX poprawnie podłączone
- [ ] VIN 12 V na sterownik
- [ ] Baudrate = 9600 bps
- [ ] Poprawne CRC Modbus
- [ ] UART1 aktywny na GPIO 12 (TX) i 11 (RX)

## 🎯 Rezultat

Po wdrożeniu tych zmian silnik DFRobot DRI0050 będzie się uruchamiał i reagował na polecenia z ESP32 oraz dashboardu.

## 📋 Dodatkowe informacje

### Protokół Modbus RTU dla DRI0050

**Adres urządzenia**: 0x32 (50 decimal)
**Funkcja zapisu**: 0x06
**Rejestry**:
- 0x0006: Duty cycle (0-1000)
- 0x0007: Frequency (1-1000 Hz)

### Przykładowe ramki:

**Uruchomienie silnika (80% duty)**:
```
32 06 00 06 03 20 CRC_L CRC_H
```

**Zatrzymanie silnika**:
```
32 06 00 06 00 00 CRC_L CRC_H
```

### Debugowanie

1. **Sprawdź CRC** - użyj kalkulatora online lub funkcji `modbus_crc16()`
2. **Monitor UART** - `idf.py monitor` pokaże wysyłane ramki
3. **Oscilloscope** - sprawdź sygnały TX/RX
4. **Multimeter** - sprawdź napięcia zasilania

### Troubleshooting

| Problem | Przyczyna | Rozwiązanie |
|---------|-----------|-------------|
| Silnik nie reaguje | Błędne CRC | Sprawdź funkcję `modbus_crc16()` |
| Brak komunikacji | Błędne połączenia TX/RX | Zamień miejscami TX i RX |
| Silnik nie kręci | Brak zasilania 12V | Sprawdź VIN na DRI0050 |
| Niestabilna praca | Złe uziemienie | Sprawdź wspólną masę |
