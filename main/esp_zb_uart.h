#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/uart.h"


#define portTICK_RATE_MS 100
#define UART_NUM UART_NUM_1 // Utilisation de UART1
#define TXD_PIN (GPIO_NUM_24) // Pas utilisé ici, mais nécessaire pour init UART
#define RXD_PIN (GPIO_NUM_23) // Broche connectée au Linky
#define BUF_SIZE 1024 // Taille du buffer UART

typedef struct {
    char iinst[8];
    char papp[8];
    uint8_t data[BUF_SIZE];
} TeleinfoValues;


void parse_teleinfo(const char *trame, TeleinfoValues *data);

void uart_init();

TeleinfoValues* teleinfo_measure();

#ifdef __cplusplus
} // extern "C"
#endif