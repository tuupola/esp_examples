/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
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

#include "spi.h"
#include "ili9341.h"
#include "alien.h"
#include "pretty_effect.h"

static const char *TAG = "main";

#define swap(x,y) do \
    { unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp,&y,sizeof(x)); \
        memcpy(&y,&x,       sizeof(x)); \
        memcpy(&x,swap_temp,sizeof(x)); \
    } while (0)

static spi_device_handle_t g_spi;

void test_task(void *params)
{
    uint16_t x1;
    uint16_t x2;
    uint16_t y1;
    uint16_t y2;
    //uint16_t colour;

    while(1) {
        x1 = (rand() % 310);
        x2 = x1 + 15;
        y1 = (rand() % 230);
        y2 = y1 + 9;
        //colour = rand() % 0xffff;

        if (x1 > x2) {
            swap(x1, x2);
        }

        if (y1 > y2) {
            swap(y1, y2);
        }
        ili9431_bitmap(g_spi, x1, y1, 16, 10, &alien2);
        /* Do some stuff here. */
        ili9341_wait(g_spi);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGD(TAG, "Wheeee!");
    spi_master_init(&g_spi);
    ili9341_init(&g_spi);

    xTaskCreatePinnedToCore(test_task, "Test task", 2048, NULL, 1, NULL, 1);
}
