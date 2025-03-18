#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"
#include "esp_task_wdt.h"

#define BUF_SIZE 1024
// Augmenter les buffers UART et réduire la taille des trames
#define UART_TX_BUF_SIZE 512
#define UART_RX_BUF_SIZE 512
#define CHUNK_SIZE 16  // Réduire la taille des chunks

// Définir d'abord la structure avant de l'utiliser
typedef struct {
    char iinst[8];
    char papp[8];
    uint8_t data[BUF_SIZE];
} TeleinfoValues;

#define portTICK_RATE_MS 100
#define UART_TX_NUM UART_NUM_1  // Pour la génération (TX sur pin 1)
#define UART_RX_NUM UART_NUM_0  // Pour la lecture (RX sur pin 0)
#define TXD_PIN 24
#define RXD_PIN 23


#define HOOK_TELEINFO 0

// Définir la structure d'une trame Teleinfo
static const char TELEINFO_START_FRAME = 0x02;  // STX
static const char TELEINFO_END_FRAME = 0x03;    // ETX
static const char TELEINFO_START_LINE = 0x0A;   // LF
static const char TELEINFO_END_LINE = 0x0D;     // CR

// Déclarations des variables globales avec extern
extern SemaphoreHandle_t teleinfo_mutex;
extern char frame_buffer[BUF_SIZE];
extern TeleinfoValues teleinfo_data;

// Configurations watchdog
#define UART_TASK_WDT_TIMEOUT_MS 3000
#define UART_TASK_STACK_SIZE 4096
#define UART_TASK_PRIORITY 5

void parse_teleinfo(const char *trame, TeleinfoValues *data);

void uart_init();

TeleinfoValues* teleinfo_measure();

void simulated_teleinfo_frame(char *buffer, size_t buffer_size);

void teleinfo_generator_task(void *pvParameters);

#ifdef __cplusplus
} // extern "C"
#endif