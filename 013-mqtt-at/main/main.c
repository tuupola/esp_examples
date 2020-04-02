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

#include <mqtt_client.h>
#include "mqtt_msg.h"

#include "uart.h"
#include "hex.h"
#include "sim-8xx.h"
#include "sdkconfig.h"

static const char* TAG = "main";

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    uart_init();

    ESP_LOGI(TAG, "Heap after init: %d", esp_get_free_heap_size());

    //xTaskCreatePinnedToCore(uart_task, "UART", 2048, NULL, 10, NULL, 1);

    ESP_LOGI(TAG, "Heap after tasks: %d", esp_get_free_heap_size());

    sim8xx_init();
    sim8xx_enable_gprs();

    // uart_write_bytes(UART_NUM_2, "AT+CIPMUX=0\n", strlen("AT+CIPMUX=0\n"));
    // vTaskDelay(3000 / portTICK_RATE_MS);

    mqtt_message_t *message;

    mqtt_connection_t connection;
    connection.buffer_length = MQTT_BUFFER_SIZE_BYTE;
    connection.buffer = (uint8_t *)malloc(MQTT_BUFFER_SIZE_BYTE);

    mqtt_connect_info_t info;
    //info.client_id = platform_create_id_string();
    info.username = NULL;
    info.client_id = "ESP32_4f9D3C";
    info.clean_session = false;
    info.keepalive = 60;
    message = mqtt_msg_connect(&connection, &info);

    sim8xx_tcp_connect("test.mosquitto.org", 1883);
    sim8xx_tcp_write_bytes((const char *)message->data, message->length);

    const char *topic = "test/telemetry";
    const char *data = "Hello world!";
    uint16_t message_id;

    message = mqtt_msg_publish(&connection, topic, data, 12, 0, 0, &message_id);

    sim8xx_tcp_connect("test.mosquitto.org", 1883);
    sim8xx_tcp_write_bytes((const char *)message->data, message->length);
}
