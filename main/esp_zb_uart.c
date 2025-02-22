#include <string.h>
#include "esp_zb_tic_linky.h"
#include "esp_zb_uart.h"
#include "esp_log.h"
#include "freertos/semphr.h"

static const char *TAG = "ESP_ZB_UART";
static TeleinfoValues teleinfo_data;
SemaphoreHandle_t teleinfo_mutex; // Mutex for thread-safe access

void parse_teleinfo(const char *trame, TeleinfoValues *data)
{
    char *start, *end;

    // Lock the mutex for safe update
    if (xSemaphoreTake(teleinfo_mutex, portMAX_DELAY))
    {
        // Initialize the values to empty strings
        data->iinst[0] = '\0';
        data->papp[0] = '\0';

        // Extract IINST
        if ((start = strstr(trame, "IINST")) != NULL)
        {
            start += 6; // Skip "IINST "
            end = strchr(start, '\n');
            if (end != NULL)
            {
                strncpy(data->iinst, start, end - start);
                data->iinst[end - start] = '\0';
            }
        }

        // Extract PAPP
        if ((start = strstr(trame, "PAPP")) != NULL)
        {
            start += 5; // Skip "PAPP "
            end = strchr(start, '\n');
            if (end != NULL)
            {
                strncpy(data->papp, start, end - start);
                data->papp[end - start] = '\0';
            }
        }

        // Release the mutex
        xSemaphoreGive(teleinfo_mutex);
    }
}

void uart_init()
{
    uart_config_t uart_config = {
        .baud_rate = 1200,
        .data_bits = UART_DATA_7_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

TeleinfoValues* teleinfo_measure()
{
    uint8_t data[BUF_SIZE];
    // Lire les données UART
    int len = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, 100 / portTICK_PERIOD_MS);
    if (len > 0)
    {
        data[len] = '\0'; // Terminer la chaîne
        printf("Received frame:\n%s\n", (char *)data);
        // Analyser la trame reçue
        parse_teleinfo((char *)data, &teleinfo_data);
    }
    return &teleinfo_data;
}
