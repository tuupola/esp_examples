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

***

Contains code from http://lodev.org/cgtutor/fire.html

Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// #include <driver/spi_master.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

//#include "fps.h"
// #include "alien.h"

#include "bitmap.h"
#include "byteswap.h"
#include "ili9341.h"
#include "copepod.h"
#include "font8x8.h"
#include "fps.h"
#include "fps2.h"

static const char *TAG = "main";

spi_device_handle_t g_spi;
SemaphoreHandle_t g_mutex;
float g_fps, g_fps2;

#define FB_WIDTH        320
#define FB_HEIGHT       240
#define FIRE_WIDTH      160
#define FIRE_HEIGHT     100

framebuffer_t g_fb = {
    .width = FB_WIDTH,
    .height = FB_HEIGHT,
    .depth = 16,
};

void framebuffer_task(void *params)
{
    while (1) {
        xSemaphoreTake(g_mutex, portMAX_DELAY);
        ili9431_bitmap(g_spi, 0, 0, FB_WIDTH, FB_HEIGHT, g_fb.buffer);
        xSemaphoreGive(g_mutex);
        g_fps = fps();
        //vTaskDelay(1 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);

}

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

/* http://lodev.org/cgtutor/fire.html */
void demo_task(void *params)
{
    uint16_t palette[256];
    uint8_t fire[FIRE_HEIGHT][FIRE_WIDTH];
    hsl_t hsl;
    rgb_t rgb;

    uint16_t x1, y1, x2, y2;
    uint8_t r, g, b;
    uint16_t color;

    ESP_LOGI(TAG, "Generating palette.");
    for (uint16_t x = 0; x < 256; x++) {
        /* Hue goes from 0 to 85: red to yellow. */
        /* Saturation is always the maximum: 255. */
        /* Lightness is 0..255 for x=0..128, and 255 for x=128..255. */
        hsl.h = x / 3;
        hsl.s = 255;
        hsl.l = min(255, x * 2);

        hsl2rgb(&hsl, &rgb);

        color = RGB565(rgb.r, rgb.g, rgb.b);
        palette[x] = bswap_16(color);
    }

    /* Clear the fire. */
    memset(fire, 0x00, FIRE_HEIGHT * FIRE_WIDTH);

    ESP_LOGI(TAG, "Heap before fire bitmap: %d", esp_get_free_heap_size());

    bitmap_t bitmap = {
        .width = FIRE_WIDTH,
        .height = FIRE_HEIGHT,
        .depth = 16,
    };
    bitmap_init(&bitmap);

    ESP_LOGI(TAG, "Heap after fire bitmap: %d", esp_get_free_heap_size());

    while (1) {
        /* Random bright line in the bottom. */
        for (uint16_t x = 0; x < FIRE_WIDTH; x++) {
            color = abs(32768 + rand());
            fire[FIRE_HEIGHT - 1][x] = color;
        }

        for(uint16_t y = 0; y < FIRE_HEIGHT - 1; y++) {
            for(uint16_t x = 0; x < FIRE_WIDTH; x++) {
                fire[y][x] =
                    ((fire[(y + 1) % FIRE_HEIGHT][(x - 1 + FIRE_WIDTH) % FIRE_WIDTH]
                    + fire[(y + 1) % FIRE_HEIGHT][(x) % FIRE_WIDTH]
                    + fire[(y + 1) % FIRE_HEIGHT][(x + 1) % FIRE_WIDTH]
                    + fire[(y + 2) % FIRE_HEIGHT][(x) % FIRE_WIDTH])
                    * 32) / 129;
            }
        }

        /* Make sure watchdog does not trigger. */
        esp_task_wdt_reset();

        for(uint16_t y = 0; y < FIRE_HEIGHT - 1; y++) {
            for(uint16_t x = 0; x < FIRE_WIDTH; x++) {
                uint16_t *fireptr = bitmap.buffer + bitmap.pitch * y + bitmap.bpp * x;
                *fireptr = palette[fire[y][x]];
            }
        }

        /* Blit the fire bitmap scaled up to framebuffer. */
        pod_scale_blit(0, 20, 320, 220, &bitmap);
        g_fps2 = fps2();

        //vTaskDelay(1 / portTICK_RATE_MS);
    }

    bitmap_destroy(&bitmap);

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());

    spi_master_init(&g_spi);
    ili9341_init(&g_spi);
    framebuffer_init(&g_fb);

    g_mutex = xSemaphoreCreateMutex();

    ESP_LOGI(
        TAG, "bpp: %d width: %d height: %d depth: %d pitch: %d size: %d",
        g_fb.bpp, g_fb.width, g_fb.height, g_fb.depth, g_fb.pitch, g_fb.size
    );

    if (NULL == g_fb.buffer) {
        ESP_LOGE(TAG, "Malloc failed.");
    } else {
        ESP_LOGI(TAG, "Heap after framebuffer: %d", esp_get_free_heap_size());

        memset(g_fb.buffer, 0x00, g_fb.size);

        if (NULL != g_mutex) {
            xTaskCreatePinnedToCore(framebuffer_task, "Framebuffer", 8192, NULL, 1, NULL, 0);
            xTaskCreatePinnedToCore(fps_task, "FPS", 4096, NULL, 2, NULL, 1);
            xTaskCreatePinnedToCore(demo_task, "Demo", 76800 + 4096, NULL, 1, NULL, 1);
        } else {
            ESP_LOGE(TAG, "No mutex?");
        }
    }
}
