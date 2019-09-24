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
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_task_wdt.h>

#include "axp192.h"
#include "bitmap.h"
#include "st7735s.h"
#include "copepod.h"
#include "copepod-hal.h"
#include "esp-i2c-hal.h"
#include "font8x8.h"
#include "fps.h"
#include "fps2.h"
#include "sdkconfig.h"

static const char *TAG = "main";
static char primitive[32];

static SemaphoreHandle_t mutex;
static float fb_fps;
static float fx_fps;
static uint16_t demo = 0;

/*
 * Flushes the framebuffer to display in a loop. M5Stick can currently
 * do about 85 fps. This demo has has fps capped to 30 fps.
 */
void framebuffer_task(void *params)
{
    TickType_t last;
    const TickType_t frequency = 1000 / 30 / portTICK_RATE_MS;

    last = xTaskGetTickCount();

    while (1) {
        vTaskDelayUntil(&last, frequency);
        xSemaphoreTake(mutex, portMAX_DELAY);
        pod_flush();
        xSemaphoreGive(mutex);
        fb_fps = fps();
    }

    vTaskDelete(NULL);
}

/*
 * Displays the info bar on top of the screen.
 */
void fps_task(void *params)
{
    uint16_t color = RGB565(0, 0, 255);
    char message[42];

#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    while (1) {
        // sprintf(message, "FX %.*f FPS          ", 1, fx_fps);
        // pod_put_text(message, 8, 4, color, font8x8);
        // sprintf(message, "FB %.*f FPS  ", 1, fb_fps);
        pod_put_text(message, 224, 4, color, font8x8);
        ESP_LOGI(TAG, "%.*f %s per second, FB %.*f FPS", 1, fx_fps, primitive, 1, fb_fps);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
#else
    while (1) {
        // sprintf(message, "FX %.*f FPS          ", 1, fx_fps);
        // pod_put_text(message, 0, 4, color, font8x8);
        ESP_LOGI(TAG, "FX %.*f FPS", 1, fx_fps);

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
#endif
    vTaskDelete(NULL);
}

void switch_task(void *params)
{
    while (1) {
        demo = (demo + 1) % 12;
        fx_fps = fps2(true);
        pod_fill_rectangle(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT -1, RGB565(0, 0, 0));
        vTaskDelay(5000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void polygon_demo()
{
    strcpy(primitive, "polygons");

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
    strcpy(primitive, "filled polygons");

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
    strcpy(primitive, "circles");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t r = (rand() % 40);
    uint16_t colour = rand() % 0xffff;
    pod_draw_circle(x0, y0, r, colour);
}

void fill_circle_demo()
{
    strcpy(primitive, "filled circles");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t r = (rand() % 40);
    uint16_t colour = rand() % 0xffff;
    pod_fill_circle(x0, y0, r, colour);
}

void line_demo()
{
    strcpy(primitive, "lines");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_draw_line(x0, y0, x1, y1, colour);
}

void rectangle_demo()
{
    strcpy(primitive, "rectangles");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_draw_rectangle(x0, y0, x1, y1, colour);
}

void fill_rectangle_demo()
{
    strcpy(primitive, "filled rectangles");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    int16_t x1 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y1 = (rand() % DISPLAY_HEIGHT + 20) - 20;
    uint16_t colour = rand() % 0xffff;
    pod_fill_rectangle(x0, y0, x1, y1, colour);
}

void put_character_demo()
{
    strcpy(primitive, "characters");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;

    uint16_t colour = rand() % 0xffff;
    char ascii = rand() % 127;
    pod_put_char(ascii, x0, y0, colour, font8x8);
}

void put_text_demo()
{
    strcpy(primitive, "strings");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;

    uint16_t colour = rand() % 0xffff;
    pod_put_text("YO! MTV raps.", x0, y0, colour, font8x8);
}

void put_pixel_demo()
{
    strcpy(primitive, "pixels");

    int16_t x0 = (rand() % DISPLAY_WIDTH + 20) - 20;
    int16_t y0 = (rand() % DISPLAY_HEIGHT + 20) - 20;

    uint16_t colour = rand() % 0xffff;
    pod_put_pixel(x0, y0, colour);
}

void triangle_demo()
{
    strcpy(primitive, "triangles");

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
    strcpy(primitive, "filled triangles");

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
    strcpy(primitive, "rgb bars");

    uint16_t red = RGB565(255, 0, 0);
    uint16_t green = RGB565(0, 255, 0);
    uint16_t blue = RGB565(0, 0, 255);
    pod_fill_rectangle(0, 0, DISPLAY_WIDTH / 3, DISPLAY_HEIGHT, red);
    pod_fill_rectangle(DISPLAY_WIDTH / 3, 0, DISPLAY_WIDTH / 3 * 2, DISPLAY_HEIGHT, green);
    pod_fill_rectangle(DISPLAY_WIDTH / 3 * 2, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, blue);
}

void demo_task(void *params)
{
    while (1) {
        if (0 == demo) {
            rgb_demo();
        } else if (1 == demo) {
            put_character_demo();
        } else if (2 == demo) {
            put_pixel_demo();
        } else if (3 == demo) {
            fill_triangle_demo();
        } else if (4 == demo) {
            triangle_demo();
        } else if (5 == demo) {
            fill_rectangle_demo();
        } else if (6 == demo) {
            rectangle_demo();
        } else if (7 == demo) {
            line_demo();
        } else if (8 == demo) {
            circle_demo();
        } else if (9 == demo) {
            fill_circle_demo();
        } else if (10 == demo) {
            polygon_demo();
        } else if (11 == demo) {
            fill_polygon_demo();
        } else if (12 == demo) {
            put_text_demo();
        }

        /* Update the FX fps counter. */
        fx_fps = fps2(false);
    }


    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    i2c_hal_master_init();
    axp192_init();
    pod_init();
    //pod_set_clip_window(0, 20, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);

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
