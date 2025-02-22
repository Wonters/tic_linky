#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_BATTERY_SENSOR_UPDATE_INTERVAL (1)     /* Local sensor update interval (second) */
#define ESP_BATTERY_SENSOR_MIN_VALUE       (0)   /* Local sensor min measured value (Volt) */
#define ESP_BATTERY_SENSOR_MAX_VALUE       (4)    /* Local sensor max measured value (Volt) */


/** @brief Temperature Measurement cluster attribute identifiers
*/
typedef enum {
    ESP_ZB_ZCL_ATTR_ELECTRONICAL_MEASUREMENT_VALUE_ID         = 0x0000,  /*!< MeasuredValue */
    ESP_ZB_ZCL_ATTR_ELECTRONICAL_MEASUREMENT_MIN_VALUE_ID     = 0x0001,  /*!< MinMeasuredValue*/
    ESP_ZB_ZCL_ATTR_ELECTRONICAL_MEASUREMENT_MAX_VALUE_ID     = 0x0002,  /*!< MaxMeasuredValue */
    ESP_ZB_ZCL_ATTR_ELECTRONICAL_MEASUREMENT_TOLERANCE_ID     = 0x0003,  /*!< Tolerance */
} esp_zb_zcl_electronical_measurement_attr_t;

typedef struct {
    int range_min;   /**< the minimum value of the temperature you want to test */
    int range_max;   /**< the maximum value of the temperature you want to test */
} battery_sensor_config_t;


#define BATTERY_SENSOR_CONFIG_DEFAULT(min, max)    \
    {                                              \
        .range_min = min,                          \
        .range_max = max,                          \
    }

/** @brief Default value for Value attribute */
#define ESP_ZB_ZCL_ELECTRONICAL_MEASUREMENT_VALUE_DEFAULT_VALUE ((int16_t)0x8000)

/** @brief Default value for MinValue attribute */
#define ESP_ZB_ZCL_ELECTRONICAL_MEASUREMENT_MIN_VALUE_DEFAULT_VALUE ((int16_t)0x8000)

/** @brief Default value for MaxValue attribute */
#define ESP_ZB_ZCL_ELECTRONICAL_MEASUREMENT_MAX_VALUE_DEFAULT_VALUE ((int16_t)0x8000)

#define ESP_ZB_ZCL_ATTR_ELECTRONICAL_MEASUREMENT_VALUE_UNKNOWN ESP_ZB_ZCL_ELECTRONICAL_MEASUREMENT_VALUE_DEFAULT_VALUE

#define ESP_ZB_ZCL_ATTR_ELECTRONICAL_MEASUREMENT_MIN_VALUE_INVALID ESP_ZB_ZCL_ELECTRONICAL_MEASUREMENT_MIN_VALUE_DEFAULT_VALUE

#define ESP_ZB_ZCL_ATTR_ELECTRONICAL_MEASUREMENT_MAX_VALUE_INVALID ESP_ZB_ZCL_ELECTRONICAL_MEASUREMENT_MAX_VALUE_DEFAULT_VALUE

typedef void (*esp_battery_sensor_callback_t)(float voltage);


float read_battery_voltage();

static void battery_sensor_driver_value_update(void *arg);

static esp_err_t battery_sensor_driver_sensor_init(battery_sensor_config_t *config);

esp_err_t init_battery_sensor(battery_sensor_config_t *config, uint16_t update_interval, esp_battery_sensor_callback_t cb);

void init_adc();


#ifdef __cplusplus
} // extern "C"
#endif