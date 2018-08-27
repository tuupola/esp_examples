/*

Adapted from Espressif examples:
https://github.com/espressif/esp-idf/blob/master/examples/wifi/simple_wifi/
https://github.com/espressif/esp-mqtt/tree/master/examples/mqtt_tcp

Copyright (c) 2016 Tuan PM
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

#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>

#include <mqtt_client.h>
#include <mjson.h>

#include "sdkconfig.h"

#define WIFI_SSID               CONFIG_WIFI_SSID
#define WIFI_PASSWORD           CONFIG_WIFI_PASSWORD
#define WIFI_CONNECTED_BIT      BIT0

#define MQTT_USERNAME           CONFIG_MQTT_USERNAME
#define MQTT_PASSWORD           CONFIG_MQTT_PASSWORD
#define MQTT_CONNECTED_BIT      BIT1
#define MQTT_SUBSCRIBED_BIT     BIT2

/* FreeRTOS event group to signal when we are connected. */
static EventGroupHandle_t wifi_event_group;
static EventGroupHandle_t mqtt_event_group;
static const char* TAG = "main";
static esp_mqtt_client_handle_t mqtt_client;

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGD(TAG, "SYSTEM_EVENT_STA_START");
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(
                TAG, "SYSTEM_EVENT_STA_GOT_IP: %s",
                ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip)
            );
            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGD(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        default:
            break;
    }

    return ESP_OK;
}

static void wifi_init()
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(
        TAG, "Connecting to SSID: [%s] with password: [%s]",
        CONFIG_WIFI_SSID, "********"
    );
    ESP_ERROR_CHECK(esp_wifi_start());
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    int16_t msg_id;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);

            msg_id = esp_mqtt_client_subscribe(mqtt_client, "/test/telemetry", 1);
            ESP_LOGI(TAG, "sent telemetry subscribe msg_id=%d", msg_id);

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
            xEventGroupClearBits(mqtt_event_group, MQTT_SUBSCRIBED_BIT);
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            xEventGroupSetBits(mqtt_event_group, MQTT_SUBSCRIBED_BIT);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            xEventGroupClearBits(mqtt_event_group, MQTT_SUBSCRIBED_BIT);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

static void mqtt_init(void)
{
    mqtt_event_group = xEventGroupCreate();

    const esp_mqtt_client_config_t mqtt_config = {
        //.uri = "mqtt://demo.thingsboard.io",
        // .username = MQTT_USERNAME,
        // .password = MQTT_PASSWORD
        .uri = "mqtt://iot.eclipse.org",
        .event_handle = mqtt_event_handler,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_start(mqtt_client);
}

void mqtt_task(void *params)
{
    ESP_LOGI(TAG, "Starting MQTT task.");
    uint8_t length;
    int16_t msg_id;
    char buffer[255];

    struct mjson_out json = MJSON_OUT_FIXED_BUF(buffer, sizeof(buffer));
    length = mjson_printf(
                &json, "{%Q:%d, %Q:%f, %Q:%B}",
                "temperature", 21,
                "humidity", 55.2,
                "active", false
            );
    ESP_LOGI(TAG, "%.*s", length, buffer);

    while(1) {
        xEventGroupWaitBits(mqtt_event_group, MQTT_SUBSCRIBED_BIT, false, true, portMAX_DELAY);

        msg_id = esp_mqtt_client_publish(
            mqtt_client,
            "/test/telemetry",
            buffer, length, 0, 0
        );
        ESP_LOGI(TAG, "sent publish, msg_id=%d", msg_id);
        vTaskDelay(3000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}


void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap at start: %d", esp_get_free_heap_size());

    wifi_init();
    mqtt_init();

    ESP_LOGI(TAG, "Heap after init: %d", esp_get_free_heap_size());

    xTaskCreatePinnedToCore(mqtt_task, "MQTT", 2048, NULL, 10, NULL, 1);
}