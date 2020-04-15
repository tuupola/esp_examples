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
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_task_wdt.h>
#include <bitmap.h>
#include <copepod_hal.h>
#include <copepod.h>
#include <font8x8.h>
#include <rgb565.h>
#include <fps.h>

#include "fire.h"
#include "fps2.h"

static const char *TAG = "main";

static SemaphoreHandle_t mutex;
static float fb_fps;
static float fx_fps;
static uint16_t demo = 0;

/*
 * Flushes the framebuffer to display in a loop. This demo is
 * capped to 30 fps.
 */
void framebuffer_task(void *params)
{
    TickType_t last;
    const TickType_t frequency = 1000 / 30 / portTICK_RATE_MS;

    last = xTaskGetTickCount();

    while (1) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        pod_flush();
        xSemaphoreGive(mutex);
        fb_fps = fps();
        vTaskDelayUntil(&last, frequency);
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
        color = rgb565(0, 255, 0);

        sprintf(message, "%.*f FXPS    ", 0, fx_fps);
        pod_put_text(message, 8, 4, color, font8x8);
        sprintf(message, "%.*f FPS  ", 1, fb_fps);
        pod_put_text(message, DISPLAY_WIDTH - 72, 4, color, font8x8);

        ESP_LOGI(TAG, "%.*f FXPS / %.*f FPS", 1, fx_fps, 1, fb_fps);
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
        .depth = DISPLAY_DEPTH,
    };

    /* But buffer to heap, there was not enough stack. */
    uint8_t *buffer = malloc(bitmap_size(&bitmap));

    bitmap_init(&bitmap, buffer);
    ESP_LOGI(TAG, "Heap after fire bitmap: %d", esp_get_free_heap_size());

    fire_init();
    ESP_LOGI(TAG, "Heap after fire init: %d", esp_get_free_heap_size());

    while (1) {

        /* Fine sire scroller. */
        if (0 == demo % 3) {
            fire_putstring(" IS IT 90'S AGAIN?      HELLO M5STACK!      THANKS LODE...", sx, FIRE_HEIGHT / 2, font8x8);

            sx = sx - 2;
            if (sx < -440) {
                sx = FIRE_WIDTH - 8;
            }

            fire_effect(&bitmap, 30, 130);
        }

        /* Static M5Stack text. */
        if (1 == demo % 3) {
            fire_putchar('M', (FIRE_WIDTH / 2) - 28, (FIRE_HEIGHT / 2) - 2, font8x8);
            fire_putchar('5', (FIRE_WIDTH / 2) - 20, (FIRE_HEIGHT / 2), font8x8);
            fire_putchar('S', (FIRE_WIDTH / 2) - 12, (FIRE_HEIGHT / 2) + 2, font8x8);
            fire_putchar('T', (FIRE_WIDTH / 2) - 4, (FIRE_HEIGHT / 2) + 3, font8x8);
            fire_putchar('A', (FIRE_WIDTH / 2) + 4, (FIRE_HEIGHT / 2), font8x8);
            fire_putchar('C', (FIRE_WIDTH / 2) + 12, (FIRE_HEIGHT / 2) - 2, font8x8);
            fire_putchar('K', (FIRE_WIDTH / 2) + 20, (FIRE_HEIGHT / 2) - 1, font8x8);
            fire_effect(&bitmap, 30, 130);
        }

        /* Can't believe it is not fireplace! */
        if (2 == demo % 3) {
            fire_feed();
            fire_effect(&bitmap, 32, 129);
        }

        /* Blit the fire bitmap scaled up to framebuffer. */
        pod_scale_blit(0, 20, DISPLAY_WIDTH, DISPLAY_HEIGHT - 20, &bitmap);
        //esp_task_wdt_reset();

        /* Update the FX fps counter. */
        fx_fps = fps2();
    }

    vTaskDelete(NULL);
}

void switch_task(void *params)
{
    while (1) {
        demo = demo + 1;
        vTaskDelay(10000 / portTICK_RATE_MS);
        fire_clear();
    }

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    pod_init();

    ESP_LOGI(TAG, "Heap after pod init: %d", esp_get_free_heap_size());

    mutex = xSemaphoreCreateMutex();

    if (NULL != mutex) {
        xTaskCreatePinnedToCore(framebuffer_task, "Framebuffer", 8192, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(fps_task, "FPS", 4096, NULL, 2, NULL, 1);
        xTaskCreatePinnedToCore(fire_task, "Fire", 8192, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(switch_task, "Switch", 2048, NULL, 1, NULL, 1);
    } else {
        ESP_LOGE(TAG, "No mutex?");
    }
}
