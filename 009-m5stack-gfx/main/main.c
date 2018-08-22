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
#include "fps.h"
#include "fps2.h"
#include "sdkconfig.h"

static const char *TAG = "main";

static SemaphoreHandle_t mutex;
static float fb_fps;
static float fx_fps;
static uint16_t demo = 0;

#define FRAMEBUFFER_WIDTH   320
#define FRAMEBUFFER_HEIGHT  240

/*
 * Flushes the framebuffer to display in a loop. With the ILI9341 DMA
 * driver can currently do 29 FPS.
 */
void framebuffer_task(void *params)
{
    while (1) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        pod_flush();
        xSemaphoreGive(mutex);
        fb_fps = fps();
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
    uint16_t color = RGB565(0, 0, 255);
    char message[42];

#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    while (1) {
        sprintf(message, " FX %.*f FPS              FB %.*f FPS", 1, fx_fps, 1, fb_fps);
        pod_puttext(message, 0, 4, color, font8x8_basic);
        ESP_LOGI(TAG, "FX %.*f FPS / FB %.*f FPS", 1, fx_fps, 1, fb_fps);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
#else
    while (1) {
        sprintf(message, " FX %.*f FPS", 1, fx_fps);
        xSemaphoreTake(mutex, portMAX_DELAY);
        //pod_puttext(message, 0, 4, color, font8x8_basic);
        xSemaphoreGive(mutex);
        ESP_LOGI(TAG, "FX %.*f FPS", 1, fx_fps);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
#endif
    vTaskDelete(NULL);
}

void switch_task(void *params)
{
    while (1) {
        pod_cls();
        demo = demo + 1;
        fx_fps = fps2(true);
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void demo_task(void *params)
{
    while (1) {
        if (0 == demo % 8) {
        //if (true) {
            /* Random filled circles, 120 per second. */
            uint16_t x0 = (rand() % 320);
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t r = (rand() % 100);
            uint16_t colour = rand() % 0xffff;

            pod_fillcircle(x0, y0, r, colour);
        } else if (0 == demo % 7) {

            /* Random circles, 3300 per second. */
            uint16_t x0 = (rand() % 320);
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t r = (rand() % 100);
            uint16_t colour = rand() % 0xffff;

            pod_circle(x0, y0, r, colour);

        } else if (0 == demo % 6) {

            /* Random pixels, 148000 per second. */
            uint16_t x0 = (rand() % 320);
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t colour = rand() % 0xffff;

            pod_putpixel(x0, y0, colour);

        } else if (0 == demo % 5) {

            /* Random lines, 10950 per second. */
            uint16_t x0 = (rand() % 370) - 50;
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t x1 = (rand() % 320) + 50;
            uint16_t y1 = (rand() % 240) + 20;
            uint16_t colour = rand() % 0xffff;

            pod_line(x0, y0, x1, y1, colour);

        } else if (0 == demo % 4) {

            /* Random rectangles, 21370 per second. */
            uint16_t x0 = (rand() % 370) - 50;
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t x1 = (rand() % 320) + 50;
            uint16_t y1 = (rand() % 220) + 20;
            uint16_t colour = rand() % 0xffff;

            pod_rectangle(x0, y0, x1, y1, colour);

        } else if (0 == demo % 3) {

            /* Random filled rectangles, 2090 per second. */
            uint16_t x0 = (rand() % 370) - 50;
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t x1 = (rand() % 320) + 50;
            uint16_t y1 = (rand() % 220) + 20;
            uint16_t colour = rand() % 0xffff;

            pod_fillrectangle(x0, y0, x1, y1, colour);

        } else if (0 == demo % 2) {

            /* Random ascii characters, 35400 per second. */
            uint16_t x0 = (rand() % 320);
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t colour = rand() % 0xffff;
            char ascii = rand() % 127;
            pod_putchar(ascii, x0, y0, colour, font8x8_basic);

        } else if (0 == demo % 1) {

            /* String in random positions, 4300 strings per second. */
            uint16_t x0 = (rand() % 370) - 50 ;
            uint16_t y0 = (rand() % 220) + 20;
            uint16_t colour = rand() % 0xffff;
            pod_puttext("YO! MTV Raps", x0, y0, colour, font8x8_basic);
        }

        /* Update the FX fps counter. */
        fx_fps = fps2(false);

        //vTaskDelay(100 / portTICK_RATE_MS);
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
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
        xTaskCreatePinnedToCore(framebuffer_task, "Framebuffer", 8192, NULL, 1, NULL, 0);
#endif
        xTaskCreatePinnedToCore(fps_task, "FPS", 8092, NULL, 2, NULL, 1);
        xTaskCreatePinnedToCore(demo_task, "Demo", 4096, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(switch_task, "Switch", 2048, NULL, 2, NULL, 0);
    } else {
        ESP_LOGE(TAG, "No mutex?");
    }
}
