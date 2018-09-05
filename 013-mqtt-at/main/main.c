/*

Copyright (c) 2018 Mika Tuupola

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

/*
#include <stdio.h>
#include <stdint.h>
#include <string.h>

*/

#include <string.h>
//#include <driver/uart.h>
// #include <esp_log.h>
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

// #include "copepod.h"
// #include "font8x8.h"
// #include "minmea.h"
#include "uart.h"
#include "hex.h"
#include "sdkconfig.h"

static const char* TAG = "main";

char* MQTT_CONNECT = "101800044d51545404020078000c45535033325f3466394433431a";
//static char* MQTT_SUBSCRIBE = "821427ea000f2f746573742f74656c656d65747279011a";
char* MQTT_PUBLISH = "3044000f2f746573742f74656c656d657472797b2274656d7065726174757265223a32312c202268756d6964697479223a35352e322c2022616374697665223a66616c73657d1a";

char mqtt[255];

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    uart_init();

    ESP_LOGI(TAG, "Heap after init: %d", esp_get_free_heap_size());

    xTaskCreatePinnedToCore(uart_task, "UART", 2048, NULL, 10, NULL, 1);

    ESP_LOGI(TAG, "Heap after tasks: %d", esp_get_free_heap_size());

    /* Check if module is connected. */
    uart_write_bytes(UART_NUM_2, "AT\n", strlen("AT\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    uart_write_bytes(UART_NUM_2, "ATI\n", strlen("ATI\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    uart_write_bytes(UART_NUM_2, "ATI+CMEE=2\n", strlen("ATI+CMEE=2\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    /* Set phone functionatiity. 1 = full, 0 = minumum, 4 = disable rf */
    uart_write_bytes(UART_NUM_2, "AT+CFUN=1\n", strlen("AT+CFUN=1\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    /* Make sure SIM does not expect PIN. Should reply +CPIN:READY. */
    uart_write_bytes(UART_NUM_2, "AT+CPIN?\n", strlen("AT+CPIN?\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    /* Signal quality report. */
    uart_write_bytes(UART_NUM_2, "AT+CSQ\n", strlen("AT+CSQ\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    /* Start task and set APN, username and password. */
    uart_write_bytes(UART_NUM_2, "AT+CSTT=\"internet\",\"\",\"\"\n", strlen("AT+CSTT=\"internet\",\"\",\"\"\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    /* Bring up wireless connection with GPRS or CSD. */
    uart_write_bytes(UART_NUM_2, "AT+CIICR\n", strlen("AT+CIICR\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    /* Get local ip address. */
    uart_write_bytes(UART_NUM_2, "AT+CIFSR\n", strlen("AT+CIFSR\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    uart_write_bytes(UART_NUM_2, "AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",\"1883\"\n", strlen("AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",\"1883\"\n"));
    vTaskDelay(3000 / portTICK_RATE_MS);
    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    uart_write_bytes(UART_NUM_2, "AT+CIPSEND\n", strlen("AT+CIPSEND\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    hex2string(MQTT_CONNECT, mqtt);
    fwrite(mqtt, 1, 27, stdout);
    uart_write_bytes(UART_NUM_2, mqtt, 27);
    vTaskDelay(1000 / portTICK_RATE_MS);


    uart_write_bytes(UART_NUM_2, "AT+CIPSEND\n", strlen("AT+CIPSEND\n"));
    vTaskDelay(1000 / portTICK_RATE_MS);
    hex2string(MQTT_PUBLISH, mqtt);
    fwrite(mqtt, 1, 71, stdout);
    uart_write_bytes(UART_NUM_2, mqtt, 71);
    vTaskDelay(1000 / portTICK_RATE_MS);

    //ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    // uart_write_bytes(UART_NUM_2, "AT+COPS?\n", strlen("AT+COPS?\n"));
    // vTaskDelay(1000 / portTICK_RATE_MS);
    // ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    // uart_write_bytes(UART_NUM_2, "AT+CSQ\n", strlen("AT+CSQ\n"));
    // vTaskDelay(1000 / portTICK_RATE_MS);
    // ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));

    // uart_write_bytes(UART_NUM_2, "AT+CBC\n", strlen("AT+CBC\n"));
    // vTaskDelay(1000 / portTICK_RATE_MS);
    // ESP_LOGD(TAG, "%s", uart_read_line(UART_NUM_2));
}