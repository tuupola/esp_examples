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

#include "i2c.h"
#include "software_i2c.h"
#include "sdkconfig.h"

static const char* TAG = "main";

void i2c_scan_task(void *params)
{
    while(1) {
        i2c_master_scan();
        vTaskDelay(10000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}


void i2c_read_0x11_task(void *params)
{
    uint8_t *data_read = (uint8_t *) malloc(SLAVE_1_DATA_LENGTH);
    ESP_LOGI(TAG, "Allocated %d bytes for reading from 0x11.", SLAVE_1_DATA_LENGTH);
    ESP_LOG_BUFFER_HEXDUMP(TAG, data_read, SLAVE_1_DATA_LENGTH, ESP_LOG_DEBUG);

    while(1) {
        ESP_ERROR_CHECK(sw_i2c_master_start());
        ESP_ERROR_CHECK(
            sw_i2c_master_write_byte((TELE_DEVICE_AIRSPEED << 1) | I2C_MASTER_READ)
        )
        if (SLAVE_1_DATA_LENGTH > 1) {
            ESP_ERROR_CHECK(
                sw_i2c_master_read(data_read, SLAVE_1_DATA_LENGTH - 1, ACK)
            )
        }
        ESP_ERROR_CHECK(
            sw_i2c_master_read_byte(data_read + SLAVE_1_DATA_LENGTH - 1, NAK)
        );
        ESP_ERROR_CHECK(sw_i2c_master_stop());
        ESP_LOG_BUFFER_HEXDUMP(TAG, data_read, SLAVE_1_DATA_LENGTH, ESP_LOG_INFO);

        uint16_t delay = 2000 + (rand() % 10) * 10;
        vTaskDelay(delay / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void i2c_read_0x12_task(void *params)
{
    uint8_t *data_read = (uint8_t *) malloc(SLAVE_2_DATA_LENGTH);
    ESP_LOGI(TAG, "Allocated %d bytes for reading from 0x12.", SLAVE_2_DATA_LENGTH);
    ESP_LOG_BUFFER_HEXDUMP(TAG, data_read, SLAVE_2_DATA_LENGTH, ESP_LOG_DEBUG);

    while(1) {
        ESP_ERROR_CHECK(sw_i2c_master_start());
        ESP_ERROR_CHECK(
            sw_i2c_master_write_byte((TELE_DEVICE_ALTITUDE << 1) | I2C_MASTER_READ)
        )
        if (SLAVE_2_DATA_LENGTH > 1) {
            ESP_ERROR_CHECK(
                sw_i2c_master_read(data_read, SLAVE_2_DATA_LENGTH - 1, ACK)
            )
        }
        ESP_ERROR_CHECK(
            sw_i2c_master_read_byte(data_read + SLAVE_2_DATA_LENGTH - 1, NAK)
        );
        ESP_ERROR_CHECK(sw_i2c_master_stop());
        ESP_LOG_BUFFER_HEXDUMP(TAG, data_read, SLAVE_2_DATA_LENGTH, ESP_LOG_INFO);

        uint16_t delay = 2000 + (rand() % 10) * 10;
        vTaskDelay(delay / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    i2c_master_init();
    xTaskCreatePinnedToCore(i2c_scan_task, "I2C scan", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(i2c_read_0x11_task, "Read 0x11", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(i2c_read_0x12_task, "Read 0x12", 2048, NULL, 1, NULL, 1);
}