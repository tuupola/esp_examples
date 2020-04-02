/*

SPDX-License-Identifier: MIT

MIT License

Copyright (c) 2019-2020 Mika Tuupola

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
/*

ILI9341
CONFIG_SPI_CLOCK_SPEED_HZ=40000000

I (6237) main: 11842.4 1x1 pixels per second
I (11237) main: 8593.0 8x8 blits per second
I (16237) main: 4680.2 16x16 blits per second
I (21237) main: 1658.9 32x32 blits per second
I (26237) main: 439.9 64x64 blits per second

ILI9341
CONFIG_SPI_CLOCK_SPEED_HZ=20000000

I (6267) main: 11268.2 1x1 pixels per second
I (11267) main: 6735.8 8x8 blits per second
I (16267) main: 3019.0 16x16 blits per second
I (21267) main: 942.1 32x32 blits per second
I (26267) main: 244.3 64x64 blits per second

ST7789V
CONFIG_SPI_CLOCK_SPEED_HZ=40000000

I (6227) main: 11833.4 1x1 pixels per second
I (11227) main: 8593.2 8x8 blits per second
I (16227) main: 4680.2 16x16 blits per second
I (21227) main: 1658.7 32x32 blits per second
I (26227) main: 439.9 64x64 blits per second

ST7789V
CONFIG_SPI_CLOCK_SPEED_HZ=20000000

I (6237) main: 11255.8 1x1 pixels per second
I (11237) main: 6735.8 8x8 blits per second
I (16237) main: 3018.8 16x16 blits per second
I (21237) main: 941.9 32x32 blits per second
I (26237) main: 244.3 64x64 blits per second

*/
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <string.h>
#include <driver/spi_master.h>
#include <stdlib.h>
#include <mipi_dcs.h>
#include <mipi_display.h>

#include "fps.h"


static const char *TAG = "main";
static uint8_t *buffer;
uint32_t buffer_size = (DISPLAY_WIDTH * (DISPLAY_DEPTH / 8) * DISPLAY_HEIGHT);
spi_device_handle_t spi;
static uint8_t current_test = 0;

static float speed;

void switch_task(void *params)
{
    const char *test_name[5];
    test_name[0] = "1x1 pixels";
    test_name[1] = "8x8 blits";
    test_name[2] = "16x16 blits";
    test_name[3] = "32x32 blits";
    test_name[4] = "64x64 blits";

    while (1) {
        vTaskDelay(5000 / portTICK_RATE_MS);
        ESP_LOGI(TAG, "%.*f %s per second", 1, speed, test_name[current_test]);
        current_test = (current_test + 1) % 5;
        speed = fps(true);
    }

    vTaskDelete(NULL);
}

void pixel_test()
{
    uint16_t color = esp_random() % 0xffff;
    int16_t x0 = esp_random() % DISPLAY_WIDTH;
    int16_t y0 = esp_random() % DISPLAY_HEIGHT;

    mipi_display_write(spi, x0, y0, 1, 1, (uint8_t *) &color);
    speed = fps(false);
}

void blit8_test()
{
    uint16_t color = esp_random() % 0xffff;
    int16_t x0 = esp_random() % (DISPLAY_WIDTH - 8);
    int16_t y0 = esp_random() % (DISPLAY_HEIGHT - 8);

    uint16_t *ptr = (uint16_t *) buffer;

    for (uint16_t i = 0; i < (8 * 8); i++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, 8, 8, buffer);
    speed = fps(false);
}

void blit16_test()
{
    uint16_t color = esp_random() % 0xffff;
    int16_t x0 = esp_random() % (DISPLAY_WIDTH - 16);
    int16_t y0 = esp_random() % (DISPLAY_HEIGHT - 16);

    uint16_t *ptr = (uint16_t *) buffer;

    for (uint16_t i = 0; i < (16 * 16); i++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, 16, 16, buffer);
    speed = fps(false);
}

void blit32_test()
{
    uint16_t color = esp_random() % 0xffff;
    int16_t x0 = esp_random() % (DISPLAY_WIDTH - 32);
    int16_t y0 = esp_random() % (DISPLAY_HEIGHT - 32);

    uint16_t *ptr = (uint16_t *) buffer;

    for (uint16_t i = 0; i < (32 * 32); i++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, 32, 32, buffer);
    speed = fps(false);
}

void blit64_test()
{
    uint16_t color = esp_random() % 0xffff;
    int16_t x0 = esp_random() % (DISPLAY_WIDTH - 64);
    int16_t y0 = esp_random() % (DISPLAY_HEIGHT - 64);

    uint16_t *ptr = (uint16_t *) buffer;

    for (uint16_t i = 0; i < (64 * 64); i++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, 64, 64, buffer);
    speed = fps(false);
}

void test_task(void *params)
{
    void (*test[5]) ();

    test[0] = pixel_test;
    test[1] = blit8_test;
    test[2] = blit16_test;
    test[3] = blit32_test;
    test[4] = blit64_test;

    while (1) {
        (*test[current_test])();
    }

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    mipi_display_init(&spi);

    buffer = (uint8_t *) heap_caps_malloc(
        buffer_size,
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );
    memset(buffer, 0x00, buffer_size);
    mipi_display_write(spi, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, buffer);

    uint8_t input[4];
    mipi_display_ioctl(spi, MIPI_DCS_GET_DISPLAY_MODE, input, 4);

    ESP_LOGI(TAG, "Heap after init: %d", esp_get_free_heap_size());

    xTaskCreatePinnedToCore(test_task, "Test", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(switch_task, "Switch", 2048, NULL, 2, NULL, 1);
}