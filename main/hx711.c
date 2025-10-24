#include "hx711.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

static const char *TAG = "HX711";

void hx711_init(hx711_t* hx711, gpio_num_t dout, gpio_num_t sck)
{
    hx711->dout_pin = dout;
    hx711->sck_pin = sck;
    hx711->gain = HX711_GAIN_128;
    hx711->offset = 0;
    hx711->scale = 1.0;
    
    // Configure DOUT as input
    gpio_config_t dout_conf = {
        .pin_bit_mask = (1ULL << dout),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&dout_conf);
    
    // Configure SCK as output
    gpio_config_t sck_conf = {
        .pin_bit_mask = (1ULL << sck),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&sck_conf);
    
    gpio_set_level(sck, 0);
    
    ESP_LOGI(TAG, "HX711 initialized on DT=GPIO%d, SCK=GPIO%d", dout, sck);
    
    // Wait for HX711 to stabilize after power-up
    ESP_LOGI(TAG, "Waiting for HX711 to stabilize...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Read once to set gain
    hx711_read(hx711);
}

bool hx711_is_ready(hx711_t* hx711)
{
    return gpio_get_level(hx711->dout_pin) == 0;
}

long hx711_read(hx711_t* hx711)
{
    // Wait for the chip to become ready (increased timeout for slower modules)
    int timeout = 0;
    while (!hx711_is_ready(hx711)) {
        vTaskDelay(pdMS_TO_TICKS(10));  // Increased delay
        timeout++;
        if (timeout > 100) {  // 1000ms total timeout
            ESP_LOGW(TAG, "HX711 not ready timeout");
            return 0;
        }
    }
    
    unsigned long value = 0;
    uint8_t data[3] = {0};
    uint8_t filler = 0x00;
    
    // Disable interrupts during bit-banging
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    
    // Pulse the clock pin 24 times to read the data
    for (int j = 3; j > 0; j--) {
        for (int i = 8; i > 0; i--) {
            gpio_set_level(hx711->sck_pin, 1);
            ets_delay_us(1);
            
            data[j - 1] = (data[j - 1] << 1);
            if (gpio_get_level(hx711->dout_pin)) {
                data[j - 1]++;
            }
            
            gpio_set_level(hx711->sck_pin, 0);
            ets_delay_us(1);
        }
    }
    
    // Set the gain for next reading
    for (int i = 0; i < (int)hx711->gain; i++) {
        gpio_set_level(hx711->sck_pin, 1);
        ets_delay_us(1);
        gpio_set_level(hx711->sck_pin, 0);
        ets_delay_us(1);
    }
    
    portEXIT_CRITICAL(&mux);
    
    // Construct the 24-bit value
    value = ((unsigned long)data[2] << 16) | ((unsigned long)data[1] << 8) | (unsigned long)data[0];
    
    // Convert from 2's complement if negative
    if (data[2] & 0x80) {
        value |= 0xFF000000;
    }
    
    return (long)value;
}

long hx711_read_average(hx711_t* hx711, int times)
{
    long sum = 0;
    for (int i = 0; i < times; i++) {
        sum += hx711_read(hx711);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return sum / times;
}

void hx711_set_gain(hx711_t* hx711, hx711_gain_t gain)
{
    hx711->gain = gain;
    gpio_set_level(hx711->sck_pin, 0);
    hx711_read(hx711);
}

void hx711_tare(hx711_t* hx711, int times)
{
    long sum = hx711_read_average(hx711, times);
    hx711->offset = sum;
    ESP_LOGI(TAG, "Tare completed. Offset: %ld", hx711->offset);
}

void hx711_set_scale(hx711_t* hx711, float scale)
{
    hx711->scale = scale;
    ESP_LOGI(TAG, "Scale set to: %.2f", scale);
}

void hx711_set_offset(hx711_t* hx711, long offset)
{
    hx711->offset = offset;
    ESP_LOGI(TAG, "Offset set to: %ld", offset);
}

void hx711_zero_scale(hx711_t* hx711)
{
    // Read current raw value and set it as the new offset
    long raw_value = hx711_read_average(hx711, 10);
    hx711->offset = raw_value;
    ESP_LOGI(TAG, "Scale zeroed. New offset: %ld", raw_value);
}

float hx711_get_value(hx711_t* hx711, int times)
{
    return (float)(hx711_read_average(hx711, times) - hx711->offset);
}

float hx711_get_units(hx711_t* hx711, int times)
{
    return hx711_get_value(hx711, times) / hx711->scale;
}

void hx711_power_down(hx711_t* hx711)
{
    gpio_set_level(hx711->sck_pin, 0);
    gpio_set_level(hx711->sck_pin, 1);
    ets_delay_us(60);
}

void hx711_power_up(hx711_t* hx711)
{
    gpio_set_level(hx711->sck_pin, 0);
}

