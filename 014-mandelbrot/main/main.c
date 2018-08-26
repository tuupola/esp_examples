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

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

//#include <esp_task_wdt.h>

#include <esp_log.h>
// #include <esp_task_wdt.h>

// #include "bitmap.h"
// #include "byteswap.h"
// #include "ili9341.h"
#include "copepod.h"
// #include "font8x8.h"
// #include "fire.h"
#include "fps.h"
// #include "fps2.h"

static const char *TAG = "main";

static SemaphoreHandle_t mutex;
static float fb_fps;
//static float fx_fps;

// #define FRAMEBUFFER_WIDTH   320
// #define FRAMEBUFFER_HEIGHT  240

/*
 * Flushes the framebuffer to display in a loop. With the ILI9341 DMA
 * driver can currently do around 29 FPS.
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
 * Flushes the framebuffer to display in a loop. With the ILI9341 DMA
 * driver can currently do around 29 FPS.
 */
void mandelbrot_task(void *params)
{
    uint16_t max_x = 320;
    uint16_t max_y = 240;
    uint16_t max_iters = 32;

    uint16_t n;
    uint16_t px;
    uint16_t py;

    double x;
    double y;
    double x0;
    double y0;
    double xtemp;
    double ytemp;

    double zoom = 0.0;

    clock_t start;
    double time_spent;

    while (1) {
        start = clock();

        n = 0;

        for (py = 1; py <= max_y / 2; py++)  {
            for (px = 1; px <= max_x; px++)  {
                x0 = (((float) px) / ((float) max_x) - 0.5) / zoom * 3.0 - 0.7;
                y0 = (((float) py) / ((float) max_y) - 0.5) / zoom * 3.0;
                x = 0.0;
                y = 0.0;
                n = 0;

                while ((x * x + y * y < 4) && (n != max_iters)) {
                    // xtemp = x * x - y * y + x0;
                    // y = 2.0 * x * y + y0;
                    // x = xtemp;
                    // n++;
                    xtemp = x * x - y * y + x0;
                    ytemp = 2 * x * y + y0;

                    if (x == xtemp && y == ytemp) {
                        n = max_iters;
                        break;
                    }
                    x = xtemp;
                    y = ytemp;
                    n++;
                }
                if (n < max_iters) {
                    pod_putpixel(px, py, n * 2);
                    pod_putpixel(px, max_y - py, n * 2);
                } else {
                    pod_putpixel(px, py, 0);
                    pod_putpixel(px, max_y - py, 0);
                }
            }
        }
        time_spent = (double)(clock() - start) / CLOCKS_PER_SEC;
        printf("Generated in %g seconds.\n", time_spent);

        start = clock();
        xSemaphoreTake(mutex, portMAX_DELAY);
        pod_flush();
        xSemaphoreGive(mutex);
        time_spent = (double)(clock() - start) / CLOCKS_PER_SEC;
        printf("Flushed in %g seconds.\n", time_spent);

        zoom = zoom + 0.1;
    }

    vTaskDelete(NULL);
}

/*
 * Displays the info bar on top of the screen. Separate FPS values
 * for the fire effect and the framebuffer. Effect is actually drawn
 * faster the display is refreshed.
//  */
// void fps_task(void *params)
// {
//     uint16_t color;
//     char message[42];

//     while (1) {
//         color = RGB565(0, 0, 255);
//         sprintf(message, " FX %.*f FPS                FB %.*f FPS", 1, fx_fps, 1, fb_fps);
//         pod_puttext(message, 0, 4, color, font8x8_basic);

//         ESP_LOGI(TAG, "FX %.*f FPS / FB %.*f FPS", 1, fx_fps, 1, fb_fps);

//         vTaskDelay(1000 / portTICK_RATE_MS);
//     }
//     vTaskDelete(NULL);
// }

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    pod_init();

    ESP_LOGI(TAG, "Heap after init: %d", esp_get_free_heap_size());

    mutex = xSemaphoreCreateMutex();

    if (NULL != mutex) {
        //xTaskCreatePinnedToCore(framebuffer_task, "FB", 8192, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(mandelbrot_task, "Mandelbrot", 8192, NULL, 1, NULL, 0);
        //xTaskCreatePinnedToCore(fps_task, "FPS", 2048, NULL, 2, NULL, 1);
    } else {
        ESP_LOGE(TAG, "No mutex?");
    }

    ESP_LOGI(TAG, "Heap after tasks: %d", esp_get_free_heap_size());
}
