#include <string.h>
#include "esp_zb_tic_linky.h"
#include "esp_zb_uart.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <inttypes.h>

static const char *TAG = "ESP_ZB_UART";

// Définitions des variables globales
SemaphoreHandle_t teleinfo_mutex;
char frame_buffer[BUF_SIZE];
TeleinfoValues teleinfo_data;

void parse_teleinfo(const char *trame, TeleinfoValues *data)
{
    char *start, *end;

    // Pas besoin de prendre le mutex ici car parse_teleinfo est appelé 
    // uniquement depuis teleinfo_measure qui a déjà le mutex
    
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
}

void uart_init()
{
    // Configuration UART avec des paramètres plus simples
    uart_config_t uart_config = {
        .baud_rate = 1200,
        .data_bits = UART_DATA_7_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Configurer RX et TX séparément
    ESP_ERROR_CHECK(uart_driver_delete(UART_RX_NUM));
    ESP_ERROR_CHECK(uart_driver_delete(UART_TX_NUM));
    
    ESP_ERROR_CHECK(uart_param_config(UART_RX_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_param_config(UART_TX_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(UART_RX_NUM, UART_PIN_NO_CHANGE, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_pin(UART_TX_NUM, TXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Installation simple des drivers
    ESP_ERROR_CHECK(uart_driver_install(UART_RX_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_driver_install(UART_TX_NUM, BUF_SIZE * 2, 0, 0, NULL, 0));

    // Créer le mutex
    teleinfo_mutex = xSemaphoreCreateMutex();
    if (teleinfo_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex");
        return;
    }

    ESP_LOGI(TAG, "UART initialized successfully");
}

void simulated_teleinfo_frame(char *buffer, size_t buffer_size)
{
    static uint32_t counter = 0;

    // Simuler des valeurs qui changent
    uint32_t simulated_iinst = 10 + (counter % 5);           // Valeur entre 10 et 14 A
    uint32_t simulated_papp = 2300 + (counter * 100) % 1000; // Valeur entre 2300 et 3200 VA

    // Créer une trame au format téléinfo
    snprintf(buffer, buffer_size,
             "\x02\n" // Start frame
             "ADCO 031428097115 B\n"
             "OPTARIF BASE 0\n"
             "IINST %" PRIu32 " A\n"
             "PAPP %" PRIu32 " VA\n"
             "MOTDETAT 000000 B\n"
             "\x03", // End frame
             simulated_iinst, simulated_papp);

    counter++;
}

TeleinfoValues *teleinfo_measure()
{
    static TeleinfoValues local_data;
    uint8_t data[BUF_SIZE];
    
    // Prendre le mutex avec un timeout plus long
    if (xSemaphoreTake(teleinfo_mutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
        // Vérifier d'abord s'il y a des données à lire
        size_t available_bytes = 0;
        esp_err_t err = uart_get_buffered_data_len(UART_RX_NUM, &available_bytes);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Failed to get buffered data length: %d", err);
            xSemaphoreGive(teleinfo_mutex);
            return &teleinfo_data;
        }
        
        // ESP_LOGD(TAG, "Available bytes to read: %d", available_bytes);
        
        if (available_bytes > 0) {
            // Limiter la lecture à la taille disponible
            size_t to_read = (available_bytes < BUF_SIZE - 1) ? available_bytes : BUF_SIZE - 1;
            
            int len = uart_read_bytes(UART_RX_NUM, 
                                    data, 
                                    to_read,
                                    pdMS_TO_TICKS(50));  // timeout de 50ms
                                    
            if (len > 0) {
                data[len] = '\0';
                parse_teleinfo((char *)data, &local_data);
                memcpy(&teleinfo_data, &local_data, sizeof(TeleinfoValues));
            } else if (len < 0) {
                ESP_LOGW(TAG, "UART read error");
            }
        }

        if (HOOK_TELEINFO) {
            parse_teleinfo(frame_buffer, &local_data);
            memcpy(&teleinfo_data, &local_data, sizeof(TeleinfoValues));
            ESP_LOGI(TAG, "Using simulated data: IINST=%s A, PAPP=%s VA", 
                    local_data.iinst, local_data.papp);
        }
        
        xSemaphoreGive(teleinfo_mutex);
    } else {
        ESP_LOGW(TAG, "Failed to get mutex for teleinfo measure");
    }

    return &teleinfo_data;
}

// Créer une tâche pour générer périodiquement les trames
void teleinfo_generator_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Teleinfo generator task started");
    
    // Délai initial plus long
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    while(1) {
        // Générer la trame
        simulated_teleinfo_frame(frame_buffer, BUF_SIZE);
        
        // Attendre que l'UART soit prêt
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Envoyer la trame complète
        int written = uart_write_bytes(UART_TX_NUM, frame_buffer, strlen(frame_buffer));
        if (written < 0) {
            ESP_LOGE(TAG, "UART write error");
        } else {
            ESP_LOGD(TAG, "Written %d bytes", written);
        }
        
        // Attendre que tout soit envoyé
        uart_wait_tx_done(UART_TX_NUM, pdMS_TO_TICKS(1000));
        
        // Pause plus longue entre les trames
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
