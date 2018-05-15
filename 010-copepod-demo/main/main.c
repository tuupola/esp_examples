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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "bitmap.h"
#include "byteswap.h"
#include "ili9341.h"
#include "copepod.h"
#include "font8x8.h"
#include "fire.h"
#include "fps.h"
#include "fps2.h"

static const char *TAG = "main";

SemaphoreHandle_t g_mutex;
float g_fps;
float g_fps2;
uint16_t g_demo = 0;

#define FRAMEBUFFER_WIDTH   320
#define FRAMEBUFFER_HEIGHT  240

/*
 * Flushes the framebuffer to display in a loop. With the ILI9341 DMA
 * driver can currently do 29 FPS.
 */
void framebuffer_task(void *params)
{
    while (1) {
        xSemaphoreTake(g_mutex, portMAX_DELAY);
        pod_flush();
        xSemaphoreGive(g_mutex);
        g_fps = fps();
        //vTaskDelay(1 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

/*
 * Displays the info bar on top of the screen. Separate FPS values
 * for the fire effect and the framebuffer. Effect is actually drawn
 * faster the display is refreshed.
 */
void fps_task(void *params)
{
    uint16_t color;
    char message[42];

    while (1) {
        color = RGB565(0, 0, 255);
        sprintf(message, " FX %.*f FPS                FB %.*f FPS", 1, g_fps2, 1, g_fps);
        pod_puttext(message, 0, 4, color, font8x8_basic);

        ESP_LOGI(TAG, "FX %.*f FPS / FB %.*f FPS", 1, g_fps2, 1, g_fps);
        //ESP_LOG_BUFFER_HEXDUMP(TAG, g_fb.buffer, 16 * 8, ESP_LOG_DEBUG);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

/*
 * Animated fire effect. Adapted from http://lodev.org/cgtutor/fire.html.
 */
void fire_task(void *params)
{
    ESP_LOGI(TAG, "Heap before initialising fire: %d", esp_get_free_heap_size());

    int16_t sx = 100; /* Sine scroller x position. */

    bitmap_t bitmap = {
        .width = FIRE_WIDTH,
        .height = FIRE_HEIGHT,
        .depth = 16,
    };
    bitmap_init(&bitmap);
    ESP_LOGI(TAG, "Heap after fire bitmap: %d", esp_get_free_heap_size());

    fire_init();
    ESP_LOGI(TAG, "Heap after fire init: %d", esp_get_free_heap_size());

    while (1) {

        /* Fine sire scroller. */
        if (0 == g_demo % 3) {
            fire_putstring(" IS IT 90'S AGAIN?      HELLO M5STACK!      GREETZ LODE :D", sx, FIRE_HEIGHT / 2, font8x8_basic);

            sx = sx - 1;
            if (sx < -440) {
                sx = FIRE_WIDTH - 8;
            }

            fire_effect(&bitmap, 30, 130);
        }

        /* Static M5Stack text. */
        if (1 == g_demo % 3) {
            fire_putchar('M', (FIRE_WIDTH / 2) - 28, (FIRE_HEIGHT / 2) - 2, font8x8_basic);
            fire_putchar('5', (FIRE_WIDTH / 2) - 20, (FIRE_HEIGHT / 2), font8x8_basic);
            fire_putchar('S', (FIRE_WIDTH / 2) - 12, (FIRE_HEIGHT / 2) + 2, font8x8_basic);
            fire_putchar('T', (FIRE_WIDTH / 2) - 4, (FIRE_HEIGHT / 2) + 3, font8x8_basic);
            fire_putchar('A', (FIRE_WIDTH / 2) + 4, (FIRE_HEIGHT / 2), font8x8_basic);
            fire_putchar('C', (FIRE_WIDTH / 2) + 12, (FIRE_HEIGHT / 2) - 2, font8x8_basic);
            fire_putchar('K', (FIRE_WIDTH / 2) + 20, (FIRE_HEIGHT / 2) - 1, font8x8_basic);
            fire_effect(&bitmap, 30, 130);
        }

        /* Can't believe it is not fireplace! */
        if (2 == g_demo % 3) {
            fire_feed();
            fire_effect(&bitmap, 32, 129);
        }

        /* Blit the fire bitmap scaled up to framebuffer. */
        pod_scale_blit(0, 20, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT - 20, &bitmap);
        //esp_task_wdt_reset();

        /* Update the FX fps counter. */
        g_fps2 = fps2();

        //vTaskDelay(10 / portTICK_RATE_MS);
    }

    bitmap_destroy(&bitmap);

    vTaskDelete(NULL);
}

void switch_task(void *params)
{
    while (1) {
        g_demo = g_demo + 1;
        fire_clear();
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    pod_init();

    ESP_LOGI(TAG, "Heap after pod init: %d", esp_get_free_heap_size());

    g_mutex = xSemaphoreCreateMutex();

    if (NULL != g_mutex) {
        xTaskCreatePinnedToCore(framebuffer_task, "Framebuffer", 8192, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(fps_task, "FPS", 4096, NULL, 2, NULL, 1);
        xTaskCreatePinnedToCore(fire_task, "Fire", 8192, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(switch_task, "switch", 2048, NULL, 1, NULL, 1);
    } else {
        ESP_LOGE(TAG, "No mutex?");
    }
}
