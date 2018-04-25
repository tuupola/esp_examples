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
#include "plankton.h"

static const char *TAG = "main";

#define swap(x,y) do \
    { unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp,&y,sizeof(x)); \
        memcpy(&y,&x,       sizeof(x)); \
        memcpy(&x,swap_temp,sizeof(x)); \
    } while (0)

spi_device_handle_t g_spi;
SemaphoreHandle_t g_mutex;
float g_fps;

void fps_task(void *params)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t colour;
    char message[42];

    while (1) {
        colour = RGB565(0, 0, 255);
        sprintf(message, " %.*f FPS ", 1, g_fps);

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        pln_put_text(message, 232, 0, colour);
        xSemaphoreGive(g_mutex);

        ESP_LOGI(TAG, "FPS: %f", g_fps);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

/* Random -> 4388.220703 sprites/s @48khz */
void alien_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;

    while (1) {
        x1 = (rand() % 310);
        x2 = x1 + 15;
        y1 = (rand() % 220) + 10;
        y2 = y1 + 9;

        if (x1 > x2) {
            swap(x1, x2);
        }

        if (y1 > y2) {
            swap(y1, y2);
        }

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        ili9431_bitmap(g_spi, x1, y1, 16, 10, &alien2);
        xSemaphoreGive(g_mutex);
        g_fps = fps();
        //vTaskDelay(500 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}


/* Random -> 4230.000000 characters/s @48khz */
void put_char_task(void *params)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t colour;
    char ascii;

    while (1) {
        x1 = (rand() % 313);
        y1 = (rand() % 223) + 10;
        colour = rand() % 0xffff;
        ascii = rand() % 127;

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        pln_put_char(ascii, x1, y1, colour);
        xSemaphoreGive(g_mutex);

        g_fps = fps();
        //vTaskDelay(500 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

/* Random -> 360.922150 YO! MTV Raps/s @48khz */
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

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        pln_put_text("YO! MTV Raps", x1, y1, colour);
        xSemaphoreGive(g_mutex);
        g_fps = fps();
    }

    vTaskDelete(NULL);
}


/* Random -> 5541.737793 pixels/s @48khz */
void pixel_task(void *params)
{
    uint16_t x1;
    uint16_t y1;
    uint16_t colour;

    while (1) {
        x1 = (rand() % 320);
        y1 = (rand() % 240);
        colour = rand() % 0xffff;

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        ili9431_putpixel(g_spi, x1, y1, colour);
        xSemaphoreGive(g_mutex);

        g_fps = fps();
        //vTaskDelay(500 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

/* Random -> 39.344261 lines/s @48khz */
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
        y1 = (rand() % 240);
        y2 = (rand() % 320);
        colour = rand() % 0xffff;

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        pln_draw_line(x1, y1, x2, y2, colour);
        xSemaphoreGive(g_mutex);
        g_fps = fps();
        //vTaskDelay(500 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

/* Random -> 64.413460 fps @48khz */
/* 320x240 -> 100.222466 fps @48khz */
void rectangle_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
    uint16_t colour;

    while (1) {
        // x1 = (rand() % 290);
        // x2 = x1 + 30;
        // y1 = (rand() % 220);
        // y2 = y1 + 20;
        colour = rand() % 0xffff;

        // x1 = (rand() % 320);
        // x2 = (rand() % 320);
        // y1 = (rand() % 240);
        // y2 = (rand() % 320);
        x1 = 0;
        y1 = 0;
        x2 = 320;
        y2 = 240;

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        pln_draw_rectangle(x1, y1, x2, y2, colour);
        xSemaphoreGive(g_mutex);

        g_fps = fps();

        vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

/* Random -> 146.971924 fps @48khz */
/* 320x240 -> 29.214964 fps @48khz */
void fill_rectangle_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
    uint16_t colour;

    while (1) {
        // x1 = (rand() % 200);
        // x2 = x1 + 10;
        // y1 = (rand() % 220);
        // y2 = y1 + 10;
        colour = rand() % 0xffff;

        // x1 = 0;
        // y1 = 0;
        // x2 = 320;
        // y2 = 240;
        x1 = (rand() % 320);
        x2 = (rand() % 320);
        y1 = (rand() % 230) + 10;
        y2 = (rand() % 230) + 10;

        xSemaphoreTake(g_mutex, portMAX_DELAY);
        pln_fill_rectangle(x1, y1, x2, y2, colour);
        xSemaphoreGive(g_mutex);

        g_fps = fps();
        //vTaskDelay(500 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void spi_queue_task(void *params)
{
    while (1) {
        ili9341_wait(g_spi);
        vTaskDelay(10 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGD(TAG, "Wheeee!");
    spi_master_init(&g_spi);
    ili9341_init(&g_spi);

    g_mutex = xSemaphoreCreateMutex();

    if (NULL != g_mutex) {
        xTaskCreatePinnedToCore(fps_task, "FPS task", 4096, NULL, 2, NULL, 1);


        //xTaskCreatePinnedToCore(put_text_task, "Put text task", 153600 + 2048, NULL, 1, NULL, 1);
        //xTaskCreatePinnedToCore(put_char_task, "Put char task", 2048, NULL, 1, NULL, 1);
        //xTaskCreatePinnedToCore(alien_task, "Alien task", 2048, NULL, 1, NULL, 1);
        //xTaskCreatePinnedToCore(pixel_task, "Pixel task", 2048, NULL, 1, NULL, 1);
        //xTaskCreatePinnedToCore(line_task, "Line task", 2048, NULL, 1, NULL, 1);
        //xTaskCreatePinnedToCore(rectangle_task, "Rectangle task", 4096, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(fill_rectangle_task, "Fill rectangle task", 153600 + 2048, NULL, 1, NULL, 1);
    }

    //xTaskCreatePinnedToCore(spi_queue_task, "SPI queue task", 2048, NULL, 1, NULL, 1);
}
