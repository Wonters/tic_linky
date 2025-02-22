#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_battery.h"

/* call back function pointer */
static esp_battery_sensor_callback_t func_ptr;
/* update interval in seconds */
static uint16_t interval = 1;

static const char *TAG = "ESP_TEMP_SENSOR_DRIVER";

float read_battery_voltage(){
    int adc_value = adc1_get_raw(ADC1_CHANNEL_6);
    float voltage = adc_value * (3.3 / 4095);  // Convert ADC value to voltage
    ESP_LOGI(TAG, "ADC Value: %d, Voltage: %.2fV", adc_value, voltage);
    return voltage;
}

/**
 * @brief Tasks for updating the sensor value
 *
 * @param arg      Unused value.
 */
static void battery_sensor_driver_value_update(void *arg)
{
    for (;;) {
    float voltage = read_battery_voltage();
        if (func_ptr) {
            func_ptr(voltage);
        }
        vTaskDelay(pdMS_TO_TICKS(interval * 1000));
    }
}

/**
 * @brief init temperature sensor
 *
 * @param config      pointer of temperature sensor config.
 */
static esp_err_t battery_sensor_driver_sensor_init(battery_sensor_config_t *config)
{
    return (xTaskCreate(battery_sensor_driver_value_update, "sensor_update", 2048, NULL, 10, NULL) == pdTRUE) ? ESP_OK : ESP_FAIL;
}

esp_err_t init_battery_sensor(battery_sensor_config_t *config, uint16_t update_interval,
                             esp_battery_sensor_callback_t cb)
{
    if (ESP_OK != battery_sensor_driver_sensor_init(config)) {
        return ESP_FAIL;
    }
    func_ptr = cb;
    interval = update_interval;
    return ESP_OK;
}

void init_adc(){
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_0); // ADC1_CHANNEL_6 correspond au GPIO34
}

