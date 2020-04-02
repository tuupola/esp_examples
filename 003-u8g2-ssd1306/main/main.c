/*

SPDX-License-Identifier: MIT-0

MIT No Attribution

Copyright (c) 2017-2020 Mika Tuupola

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
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "u8g2.h"

#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"

#define SSD1306_SDA 4
#define SSD1306_SCL 15
#define SSD1306_RST 16

static const char* TAG = "u8g2_ssd1306";

u8g2_t u8g2;

void ssd1306_reset()
{
    ESP_LOGI(TAG, "Reset SSD1306 to make it appear in the I2C bus.");
    gpio_pad_select_gpio(SSD1306_RST);
    ESP_ERROR_CHECK(gpio_set_direction(SSD1306_RST, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(SSD1306_RST, 0));
    vTaskDelay(50/portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(gpio_set_level(SSD1306_RST, 1));
}

void ssd1306_init()
{
    ESP_LOGI(TAG, "Initialize, clear and power up the display.");
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.sda = SSD1306_SDA;
    u8g2_esp32_hal.scl = SSD1306_SCL;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    u8g2_Setup_ssd1306_i2c_128x64_alt0_f(
        &u8g2,
        U8G2_R0,
        u8g2_esp32_i2c_byte_cb,
        u8g2_esp32_gpio_and_delay_cb
    );

    u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}

void ssd1306_task(void *pvParameters)
{
    while(1) {
        u8g2_DrawFrame(&u8g2, 13, 15, 100, 6);

        for (int i = 0; i <= 100; i = i + 10) {
            u8g2_DrawBox(&u8g2, 13, 15, i, 6);
            u8g2_SendBuffer(&u8g2);
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
        u8g2_ClearBuffer(&u8g2);

        ESP_LOGI(TAG, "WHEEE!!");

        u8g2_SetFont(&u8g2, u8g2_font_profont10_tf);
        u8g2_DrawStr(&u8g2, 48, 21, "WHEEE!!");
        u8g2_SendBuffer(&u8g2);
        vTaskDelay(500 / portTICK_RATE_MS);

        u8g2_ClearBuffer(&u8g2);
        u8g2_SendBuffer(&u8g2);
    }

    vTaskDelete(NULL);
}


/* ESP32 application entry point. */
void app_main()
{
    ssd1306_reset();
    ssd1306_init();

    xTaskCreatePinnedToCore(ssd1306_task, "SSD1306 task", 4096, NULL, 1, NULL, 1);
}