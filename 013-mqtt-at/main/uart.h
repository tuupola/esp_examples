#ifndef _UART_H
#define _UART_H

#include <driver/uart.h>
#include <esp_log.h>

#define UART_RX_BUF_SIZE    2048
#define UART_TX_BUF_SIZE    1024
#define UART_QUEUE_SIZE     0
#define UART_QUEUE_HANDLE   NULL
#define UART_INTR_FLAGS     0

void uart_init();
void uart_task(void *params);
esp_err_t uart_read_line(uart_port_t uart_port, char *buffer);

#endif