/*

SPDX-License-Identifier: MIT-0

MIT No Attribution

Copyright (c) 2017-2020 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "sdkconfig.h"

static const char* TAG = "hello_world";

void hello_world_task(void *pvParameters)
{
    while(1) {
        ESP_LOGI(TAG, "Hello world!");
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void hello_moon_task(void *pvParameters)
{
    while(1) {
        ESP_LOGI(TAG, "Hello moon!");
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