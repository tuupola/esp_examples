/*

SPDX-License-Identifier: MIT

MIT License

Copyright (c) 2018-2020 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "uart.h"

static const char* TAG = "uart";

void uart_init()
{
    ESP_LOGI(TAG, "Init UART.");

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(
        uart_param_config(UART_NUM_2, &uart_config)
    );
    ESP_ERROR_CHECK(
        uart_driver_install(
            UART_NUM_2,
            UART_RX_BUF_SIZE,
            UART_TX_BUF_SIZE,
            UART_QUEUE_SIZE,
            UART_QUEUE_HANDLE,
            UART_INTR_FLAGS
        )
    );

    ESP_ERROR_CHECK(
        uart_set_pin(
            UART_NUM_2,
            GPIO_NUM_17, // M5Stack TX SIM800l module pins
            GPIO_NUM_16, // M5Stack RX SIM800l module pins
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );
}

esp_err_t uart_read_line(uart_port_t uart_port, char *buffer) {

    uint32_t start = xTaskGetTickCount();
    uint32_t delay_ms = 0;

    *buffer = '\0';

    while (delay_ms < 1000) {

        int16_t bytes_read = uart_read_bytes(
            uart_port,
            (unsigned char *)buffer,
            1,
            //portMAX_DELAY
            100 / portTICK_RATE_MS

        );

        if (1 == bytes_read) {
            if ('\n' == *buffer || '\r' == *buffer) {
                //buffer++;
                *buffer = '\0';
                return 0;
            }
            buffer++;
        }

        delay_ms = (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
    }

    return 1; /* Timeout */
}

// void uart_task(void *params)
// {
//     ESP_LOGI(TAG, "Start UART task.");
//     while(1) {
//         char *line = uart_read_line(UART_NUM_2);
//         ESP_LOGD(TAG, "%s", line);
//         vTaskDelay(100 / portTICK_RATE_MS);
//     }

//     vTaskDelete(NULL);
// }
