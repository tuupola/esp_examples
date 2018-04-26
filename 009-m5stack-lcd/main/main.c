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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"
//#include "driver/gpio.h"
//#include "soc/gpio_struct.h"
//#include "esp_system.h"

#include "fps.h"
#include "spi.h"
#include "ili9341.h"
#include "alien.h"
#include "framebuffer.h"
#include "copepod.h"

static const char *TAG = "main";

spi_device_handle_t g_spi;
SemaphoreHandle_t g_mutex;
float g_fps;

#define FB_WIDTH    320
#define FB_HEIGHT   240

framebuffer_t g_fb = {
    .width = FB_WIDTH,
    .height = FB_HEIGHT,
    .depth = 16,
};

void fps_task(void *params)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t colour;
    char message[42];

    while (1) {
        colour = RGB565(0, 0, 255);
        sprintf(message, " %.*f FPS ", 1, g_fps);
        pod_puttext(message, 232, 0, colour);

        ESP_LOGI(TAG, "FPS: %f", g_fps);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void alien_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;

    while (1) {
        x1 = (rand() % 310);
        y1 = (rand() % 220) + 10;

        pod_blit(x1, y1, 16, 10, &alien2);
        //vTaskDelay(500 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void put_char_task(void *params)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t colour;
    char ascii;

    while (1) {
        x1 = (rand() % 320);
        y1 = (rand() % 220) + 10;
        colour = rand() % 0xffff;
        ascii = rand() % 127;

        pod_putchar(ascii, x1, y1, colour);
        //vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void put_text_task(void *params)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t colour;
    char message[42];

    while (1) {
        x1 = (rand() % 244);
        y1 = (rand() % 222) + 10;
        colour = rand() % 0xffff;

        pod_puttext("YO! MTV Raps", x1, y1, colour);
        //vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void pixel_task(void *params)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t colour;

    while (1) {
        x1 = (rand() % 320);
        y1 = (rand() % 230) + 10;
        colour = rand() % 0xffff;

        pod_putpixel(x1, y1, colour);
        //vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void line_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
    uint16_t colour;

    while (1) {
        x1 = (rand() % 320);
        x2 = (rand() % 320);
        y1 = (rand() % 230) + 10;
        y2 = (rand() % 230) + 10;
        colour = rand() % 0xffff;

        pod_line(x1, y1, x2, y2, colour);
        //vTaskDelay(500 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void rectangle_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
    uint16_t colour;

    while (1) {
        colour = rand() % 0xffff;

        // x1 = (rand() % 320);
        // x2 = (rand() % 320);
        // y1 = (rand() % 240);
        // y2 = (rand() % 320);
        x1 = 0;
        y1 = 10;
        x2 = 319;
        y2 = 239;

        pod_rectangle(x1, y1, x2, y2, colour);
        vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void fill_rectangle_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
    uint16_t colour;

    while (1) {
        colour = rand() % 0xffff;

        x1 = (rand() % 320);
        x2 = (rand() % 320);
        y1 = (rand() % 230) + 10;
        y2 = (rand() % 230) + 10;

        pod_fillrectangle(x1, y1, x2, y2, colour);
        //vTaskDelay(20 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

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

void debug_task(void *params)
{
    while (1) {
        ESP_LOGD(
            TAG, "heap: %d", esp_get_free_heap_size()
        );
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(
        TAG, "SDK version: %s, heap: %d",
        esp_get_idf_version(), esp_get_free_heap_size()
    );

    spi_master_init(&g_spi);
    ili9341_init(&g_spi);

    g_mutex = xSemaphoreCreateMutex();

    framebuffer_init(&g_fb);

    ESP_LOGI(
        TAG, "bpp: %d width: %d height: %d depth: %d pitch: %d size: %d",
        g_fb.bpp, g_fb.width, g_fb.height, g_fb.depth, g_fb.pitch, g_fb.size
    );

    ESP_LOGI(
        TAG, "ESP-IDF version: %s, heap: %d",
        esp_get_idf_version(), esp_get_free_heap_size()
    );

    if (NULL == g_fb.buffer) {
        ESP_LOGE(TAG, "Malloc failed.");
    } else {
        memset(g_fb.buffer, 0x00, g_fb.size);
        //ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, g_fb.size, ESP_LOG_INFO);
        ESP_LOG_BUFFER_HEXDUMP(TAG, g_fb.buffer, 16 * 8, ESP_LOG_INFO);

        if (NULL != g_mutex) {
            xTaskCreatePinnedToCore(fps_task, "FPS task", 4096, NULL, 2, NULL, 1);
            xTaskCreatePinnedToCore(framebuffer_task, "Framebuffer task", 8192, NULL, 1, NULL, 0);
            //xTaskCreatePinnedToCore(put_text_task, "Put text task", 2048, NULL, 1, NULL, 1);
            //xTaskCreatePinnedToCore(put_char_task, "Put char task", 2048, NULL, 1, NULL, 1);
            //xTaskCreatePinnedToCore(alien_task, "Alien task", 2048, NULL, 1, NULL, 1);
            //xTaskCreatePinnedToCore(pixel_task, "Pixel task", 2048, NULL, 1, NULL, 1);
            //xTaskCreatePinnedToCore(line_task, "Line task", 2048, NULL, 1, NULL, 1);
            //xTaskCreatePinnedToCore(rectangle_task, "Rectangle task", 4096, NULL, 1, NULL, 1);
            xTaskCreatePinnedToCore(fill_rectangle_task, "Fill rectangle task", 2048, NULL, 1, NULL, 1);
            xTaskCreatePinnedToCore(debug_task, "Debug task", 4096, NULL, 1, NULL, 1);
        }

    }
}
