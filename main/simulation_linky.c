#include <stdio.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define UART_TXD_PIN GPIO_NUM_17 // Pin TXD
#define UART_RXD_PIN GPIO_NUM_16 // Pin RXD (pas utilisé ici)
#define UART_NUM UART_NUM_1      // Utilisation d'UART1
#define BUF_SIZE 1024            // Taille du buffer UART

// Fonction pour calculer le checksum Téléinfo
char calculate_checksum(const char *label, const char *value) {
    int sum = 0;
    const char *ptr = label;

    // Somme des codes ASCII de l'étiquette
    while (*ptr) {
        sum += *ptr++;
    }

    // Somme des codes ASCII de la valeur
    ptr = value;
    while (*ptr) {
        sum += *ptr++;
    }

    // Retourner le checksum Téléinfo (caractère imprimable)
    return (sum & 0x3F) + 0x20;
}

// Fonction pour envoyer une ligne Téléinfo
void send_teleinfo_line(uart_port_t uart_num, const char *label, const char *value) {
    char buffer[64];
    char checksum = calculate_checksum(label, value);

    // Construire la ligne Téléinfo
    snprintf(buffer, sizeof(buffer), "%s %s %c\n", label, value, checksum);

    // Envoyer la ligne via UART
    uart_write_bytes(uart_num, buffer, strlen(buffer));
    printf("Envoyé : %s", buffer);
}

void teleinfo_simulator_task(void *arg) {
    while (1) {
        // Simuler et envoyer les trames Téléinfo
        send_teleinfo_line(UART_NUM, "IINST", "002"); // Intensité instantanée (2 A)
        send_teleinfo_line(UART_NUM, "PAPP", "04500"); // Puissance apparente (4500 VA)

        // Pause avant la prochaine trame (1 seconde)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    // Configuration UART
    const uart_config_t uart_config = {
        .baud_rate = 1200,
        .data_bits = UART_DATA_7_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    // Initialisation UART
    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, UART_TXD_PIN, UART_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Démarrer la tâche de simulation Téléinfo
    xTaskCreate(teleinfo_simulator_task, "teleinfo_simulator_task", 2048, NULL, 10, NULL);
}
