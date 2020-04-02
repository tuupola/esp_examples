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
#include <stdint.h>
#include <string.h>

#include <esp_log.h>
#include <driver/i2c.h>
#include <time.h>
#include <stdlib.h>

#include "sdkconfig.h"
#include "i2c.h"

static const char* TAG = "main";

void i2c_slave_1_fill_task(void *params)
{
    uint8_t *data_write = (uint8_t*) malloc(SLAVE_1_DATA_LENGTH);
    memset(data_write, 0, SLAVE_1_DATA_LENGTH);

    while(1) {
        size_t data_size = i2c_slave_write_buffer(
            I2C_SLAVE_1_NUM,
            data_write,
            SLAVE_1_DATA_LENGTH,
            1000 / portTICK_RATE_MS
        );
        ESP_LOGD(TAG, "Wrote %d bytes to slave 1.", data_size);
        ESP_LOG_BUFFER_HEX("0x11", data_write, SLAVE_1_DATA_LENGTH);

        *data_write += 1;

        uint16_t delay = 950 + (rand() % 10) * 10 ;
        vTaskDelay(delay / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void i2c_slave_2_fill_task(void *params)
{
    uint8_t *data_write = (uint8_t*) malloc(SLAVE_2_DATA_LENGTH);
    memset(data_write, 0, SLAVE_2_DATA_LENGTH);

    while(1) {
        size_t data_size = i2c_slave_write_buffer(
            I2C_SLAVE_2_NUM,
            data_write,
            SLAVE_2_DATA_LENGTH,
            1000 / portTICK_RATE_MS
        );
        ESP_LOGD(TAG, "Wrote %d bytes to slave 2.", data_size);
        ESP_LOG_BUFFER_HEX("0x12", data_write, SLAVE_2_DATA_LENGTH);

        *data_write += 1;

        uint16_t delay = 950 + (rand() % 10) * 10;
        vTaskDelay(delay / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void app_main()
{
    srand(time(NULL));

    i2c_slave_1_init();
    i2c_slave_2_init();

    xTaskCreatePinnedToCore(i2c_slave_1_fill_task, "Slave 1", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(i2c_slave_2_fill_task, "Slave 2", 2048, NULL, 1, NULL, 1);
}