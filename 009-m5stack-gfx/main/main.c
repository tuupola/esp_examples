/*

SPDX-License-Identifier: MIT-0

MIT No Attribution

Copyright (c) 2018-2020 Mika Tuupola

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "bitmap.h"
#include "rgb565.h"
#include "copepod.h"
#include "copepod_hal.h"
#include "font8x8.h"
#include "fps.h"
#include "fps2.h"
#include "sdkconfig.h"

static const char *TAG = "main";
static char primitive[32];

static SemaphoreHandle_t mutex;
static float fb_fps;
static float fx_fps;
static uint16_t current_demo = 0;

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
 * Displays the info bar on top of the screen.
 */
void fps_task(void *params)
{
    uint16_t color = rgb565(0, 255, 0);
    char message[128];

#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
    while (1) {
        pod_set_clip_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);

        sprintf(message, "%.*f %s PER SECOND       ", 0, fx_fps, primitive);
        pod_put_text(message, 8, 4, color, font8x8);
        sprintf(message, "%.*f FPS  ", 1, fb_fps);
        pod_put_text(message, DISPLAY_WIDTH - 72, DISPLAY_HEIGHT - 14, color, font8x8);

        pod_set_clip_window(0, 20, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 21);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
#else
    while (1) {
        sprintf(message, "%.*f %s PER SECOND       ", 0, fx_fps, primitive);
        pod_set_clip_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
        pod_put_text(message, 8, 4, color, font8x8);
        pod_set_clip_window(0, 20, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 21);


        vTaskDelay(2000 / portTICK_RATE_MS);
    }
#endif
    vTaskDelete(NULL);
}

void switch_task(void *params)
{
    while (1) {
        ESP_LOGI(TAG, "%.*f %s per second, FB %.*f FPS", 1, fx_fps, primitive, 1, fb_fps);

        current_demo = (current_demo + 1) % 13;
        pod_clear_screen();
        fx_fps = fps2(true);

        vTaskDelay(6000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void polygon_demo()
{
    strcpy(primitive, "POLYGONS");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x2 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y2 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x3 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y3 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x4 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y4 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    int16_t vertices[10] = {x0, y0, x1, y1, x2, y2, x3, y3, x4, y4};
    pod_draw_polygon(5, vertices, colour);
}

void fill_polygon_demo()
{
    strcpy(primitive, "FILLED POLYGONS");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x2 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y2 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x3 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y3 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x4 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y4 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    int16_t vertices[10] = {x0, y0, x1, y1, x2, y2, x3, y3, x4, y4};
    pod_fill_polygon(5, vertices, colour);
}

void circle_demo()
{
    strcpy(primitive, "CIRCLES");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t r = (rand() % 40);
    uint16_t colour = rand() % 0xffff;
    pod_draw_circle(x0, y0, r, colour);
}

void fill_circle_demo()
{
    strcpy(primitive, "FILLED CIRCLES");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t r = (rand() % 40);
    uint16_t colour = rand() % 0xffff;
    pod_fill_circle(x0, y0, r, colour);
}

void line_demo()
{
    strcpy(primitive, "LINES");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_draw_line(x0, y0, x1, y1, colour);
}

void rectangle_demo()
{
    strcpy(primitive, "RECTANGLES");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_draw_rectangle(x0, y0, x1, y1, colour);
}

void fill_rectangle_demo()
{
    strcpy(primitive, "FILLED RECTANGLES");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_fill_rectangle(x0, y0, x1, y1, colour);
}

void put_character_demo()
{
    strcpy(primitive, "CHARACTERS");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;

    uint16_t colour = rand() % 0xffff;
    char ascii = rand() % 127;
    pod_put_char(ascii, x0, y0, colour, font8x8);
}

void put_text_demo()
{
    strcpy(primitive, "STRINGS");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 100;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;

    uint16_t colour = rand() % 0xffff;
    pod_put_text("YO! MTV raps.", x0, y0, colour, font8x8);
}

void put_pixel_demo()
{
    strcpy(primitive, "PIXELS");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;

    uint16_t colour = rand() % 0xffff;
    pod_put_pixel(x0, y0, colour);
}

void triangle_demo()
{
    strcpy(primitive, "TRIANGLES");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x2 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y2 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_draw_triangle(x0, y0, x1, y1, x2, y2, colour);
}

void fill_triangle_demo()
{
    strcpy(primitive, "FILLED TRIANGLES");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x2 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y2 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_fill_triangle(x0, y0, x1, y1, x2, y2, colour);
}

void rgb_demo()
{
    strcpy(primitive, "RGB BARS");

    uint16_t red = rgb565(255, 0, 0);
    uint16_t green = rgb565(0, 255, 0);
    uint16_t blue = rgb565(0, 0, 255);

    int16_t x0 = 0;
    int16_t x1 = DISPLAY_WIDTH / 3;
    int16_t x2 = 2 * x1;

    pod_fill_rectangle(x0, 0, x1 - 1, DISPLAY_HEIGHT, red);
    pod_fill_rectangle(x1, 0, x2 - 1, DISPLAY_HEIGHT, green);
    pod_fill_rectangle(x2, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, blue);
}

void demo_task(void *params)
{
    void (*demo[13]) ();

    demo[0] = rgb_demo;
    demo[1] = put_character_demo;
    demo[2] = put_pixel_demo;
    demo[3] = fill_triangle_demo;
    demo[4] = triangle_demo;
    demo[5] = fill_rectangle_demo;
    demo[6] = rectangle_demo;
    demo[7] = line_demo;
    demo[8] = circle_demo;
    demo[9] = fill_circle_demo;
    demo[10] = polygon_demo;
    demo[11] = fill_polygon_demo;
    demo[12] = put_text_demo;

    while (1) {
        (*demo[current_demo])();
        /* Update the primitive fps counter. */
        fx_fps = fps2(false);
    }


    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    pod_init();
    pod_set_clip_window(0, 20, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 21);

    ESP_LOGI(TAG, "Heap after pod init: %d", esp_get_free_heap_size());

    mutex = xSemaphoreCreateMutex();

    if (NULL != mutex) {
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
        xTaskCreatePinnedToCore(framebuffer_task, "Framebuffer", 8192, NULL, 1, NULL, 0);
#endif
        xTaskCreatePinnedToCore(fps_task, "FPS", 8092, NULL, 2, NULL, 1);
        xTaskCreatePinnedToCore(demo_task, "Demo", 4096, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(switch_task, "Switch", 2048, NULL, 2, NULL, 1);
    } else {
        ESP_LOGE(TAG, "No mutex?");
    }
}
