#include <string.h>
#include "esp_zb_tic_linky.h"
#include "esp_zb_uart.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <inttypes.h>

static const char *TAG = "ESP_ZB_UART";
static TeleinfoValues teleinfo_data;
SemaphoreHandle_t teleinfo_mutex; // Mutex for thread-safe access

void parse_teleinfo(const char *trame, TeleinfoValues *data)
{
    char *start, *end;

    // Vérifier que le mutex existe
    if (teleinfo_mutex == NULL) {
        ESP_LOGE(TAG, "Mutex not initialized");
        return;
    }

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
    // Initialiser le mutex
    teleinfo_mutex = xSemaphoreCreateMutex();
    if (teleinfo_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return;
    }

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
    //     uint8_t data[BUF_SIZE];
    // // Lire les données UART
    // int len = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, 100 / portTICK_PERIOD_MS);
    // if (len > 0)
    // {
    //     data[len] = '\0'; // Terminer la chaîne
    //     printf("Received frame:\n%s\n", (char *)data);
    //     // Analyser la trame reçue
    //     parse_teleinfo((char *)data, &teleinfo_data);
    // }

    // Simuler une trame téléinfo
    static uint32_t counter = 0;
    char simulated_frame[BUF_SIZE];
    
    // Simuler des valeurs qui changent
    uint32_t simulated_iinst = 10 + (counter % 5);  // Valeur entre 10 et 14 A
    uint32_t simulated_papp = 2300 + (counter * 100) % 1000;  // Valeur entre 2300 et 3200 VA
    
    // Créer une trame au format téléinfo
    snprintf(simulated_frame, BUF_SIZE,
        "\x02\n"  // Start frame
        "ADCO 031428097115 B\n"
        "OPTARIF BASE 0\n"
        "IINST %" PRIu32 " A\n"
        "PAPP %" PRIu32 " VA\n"
        "MOTDETAT 000000 B\n"
        "\x03",    // End frame
        simulated_iinst, simulated_papp);
    
    // Analyser la trame simulée
    parse_teleinfo(simulated_frame, &teleinfo_data);
    
    // Incrémenter le compteur pour la prochaine simulation
    counter++;
    
    ESP_LOGI(TAG, "Simulated frame:\n%s\n", simulated_frame);
    
    return &teleinfo_data;
}
