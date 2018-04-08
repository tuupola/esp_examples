#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "sdkconfig.h"

static const char* TAG = "hello_world";

void hello_world_task(void *pvParameters)
{
    while(1) {
        ESP_LOGD(TAG, "Hello world!");
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void hello_moon_task(void *pvParameters)
{
    while(1) {
        ESP_LOGD(TAG, "Hello moon!");
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

// ESP32 application entry point
void app_main()
{
    ESP_LOGD(TAG, "Starting...");
    xTaskCreatePinnedToCore(hello_world_task, "Hello world task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(hello_moon_task, "Hello moon task", 2048, NULL, 1, NULL, 1);
}