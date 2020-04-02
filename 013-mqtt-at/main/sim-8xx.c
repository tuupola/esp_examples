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

#include <string.h>

#include "uart.h"

static const char* TAG = "sim-8xx";

esp_err_t sim8xx_at_expect(const char *command, const char *expect, char *buffer)
{
    uint32_t start = xTaskGetTickCount();
    uint32_t delay_ms = 0;

    vTaskDelay(100 / portTICK_PERIOD_MS);
    uart_flush(UART_NUM_2);
    uart_write_bytes(UART_NUM_2, command, strlen(command));
    uart_write_bytes(UART_NUM_2, "\n", 1);

    ESP_LOGD(TAG, "> %s", command);

    while(delay_ms < 10000) {
        esp_err_t status = uart_read_line(UART_NUM_2, buffer);

        if (strstr(buffer, expect) != NULL) {
            ESP_LOGD(TAG, "<<< %s", buffer);
            return 0;
        } else {
            if (strlen(buffer)) {
                ESP_LOGD(TAG, "< %s", buffer);
            }
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        delay_ms = (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
    }

    ESP_LOGD(TAG, "Bailing out...");
    return 1;
}

esp_err_t sim8xx_register_network()
{
    char buffer[32];
    int status = 2;

    /* Test network registration <n>,<stat>. */
    /* 0 = not registered, 1 = registered home network, 2 = not registered searching */
    /* 3 = registration denied, 4 = unknown, 5 = registered roaming */
    while (2 == status) {
        sim8xx_at_expect("AT+CREG?", "+CREG: 0,", buffer);
        sscanf(buffer, "+CREG: 0,%d", &status);
    }

    if (1 == status || 5 == status) {
        return 0;
    }

    return 1;
}

esp_err_t sim8xx_enable_gprs()
{
    char buffer[32];
    int bytes[4];

    /* Get local IP address */
    /* TODO: do not expect. */
    sim8xx_at_expect("AT+CIFSR", "10", buffer);

    /* If we already have ip address return early. */
    if (4 == sscanf(buffer, "%d.%d.%d.%d", &bytes[3], &bytes[2], &bytes[1], &bytes[0])) {
        return 0;
    }

    /* Attach or detach from GPRS service. */
    sim8xx_at_expect("AT+CGATT=1", "OK", buffer);

    /* Start task and set APN, USER NAME, PASSWORD. */
    sim8xx_at_expect("AT+CSTT=\"internet\",\"\",\"\"", "OK",  buffer);

    /* Bring up wireless connection with GPRS or CSD. */
    sim8xx_at_expect("AT+CIICR", "OK", buffer);

    /* Get local IP address */
    sim8xx_at_expect("AT+CIFSR", "OK", buffer);

    return 0;
}

esp_err_t sim8xx_init()
{
    char buffer[64];

    /* Check if the device responds. */
    sim8xx_at_expect("AT", "OK", buffer);

    /* Reset default configuration. */
    sim8xx_at_expect("ATZ", "OK", buffer);

    /* Echo off, cleans up logging a bit. */
    sim8xx_at_expect("ATE0", "OK", buffer);

    /* Report mobile equipment error. 0 = disable, 1 = numeric, 2 = verbose */
    sim8xx_at_expect("ATI+CMEE=2", "OK", buffer);

    /* Set phone functionatility. 1 = full, 0 = minumum, 4 = disable rf */
    sim8xx_at_expect("AT+CFUN=1", "OK", buffer);

    /* Test enter pin, currently assumes pin free SIM. */
    sim8xx_at_expect("AT+CPIN?", "CPIN: READY", buffer);

    sim8xx_register_network();

    /* Signal quality report. Just for debugging. */
    sim8xx_at_expect("AT+CSQ", "OK", buffer);

    return 0;
}

esp_err_t sim8xx_tcp_connect(const char *host, const uint16_t port)
{
    char buffer[64];
    char command[64];

    snprintf(command, 64, "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"", host, port);
    /* Can be ALREADY CONNECT or CONNECT OK. */
    sim8xx_at_expect(command, "CONNECT", buffer);

    return 0;
}

esp_err_t sim8xx_tcp_write_bytes(const char *bytes, size_t length)
{
    char buffer[64] = {0x00};
    char command[64] = {0x00};

    uint32_t start = xTaskGetTickCount();
    uint32_t delay_ms = 0;


    vTaskDelay(100 / portTICK_PERIOD_MS);
    uart_flush(UART_NUM_2);

    snprintf(command, 64, "AT+CIPSEND=%d", length);
    sim8xx_at_expect(command, ">", buffer);

    vTaskDelay(100 / portTICK_PERIOD_MS);
    uart_write_bytes(UART_NUM_2, bytes, length);
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, bytes, length, ESP_LOG_DEBUG);

    while(delay_ms < 50000) {
        esp_err_t status = uart_read_line(UART_NUM_2, buffer);

        if (strstr(buffer, "SEND OK") != NULL) {
            ESP_LOGD(TAG, "<<< %s", buffer);
            return 0;
        } else {
            if (strlen(buffer)) {
                ESP_LOGD(TAG, "< %s", buffer);
            }
            vTaskDelay(100 / portTICK_RATE_MS);
        }

        delay_ms = (xTaskGetTickCount() - start) * portTICK_PERIOD_MS;
    }

    ESP_LOGD(TAG, "Bailing out...");
    return 1;
}


