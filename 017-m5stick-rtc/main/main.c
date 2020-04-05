/*

SPDX-License-Identifier: MIT-0

MIT No Attribution

Copyright (c) 2020 Mika Tuupola

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

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include "esp_i2c_hal.h"
#include "axp192.h"
#include "bm8563.h"
#include "sdkconfig.h"

static const char *TAG = "main";
bm8563_datetime_t rtc;

void demo_task(void *params)
{
    float vacin, iacin, vvbus, ivbus, temp, pbat, vbat, icharge, idischarge, vaps;

    while (1) {
        bm8563_read(&rtc);
        ESP_LOGI(TAG,
            "RTC: %04d-%02d-%02d %02d:%02d:%02d",
            rtc.year, rtc.month, rtc.day, rtc.hours, rtc.minutes, rtc.seconds
        );

        axp192_read(AXP192_ACIN_VOLTAGE, &vacin);
        axp192_read(AXP192_ACIN_CURRENT, &iacin);
        axp192_read(AXP192_VBUS_VOLTAGE, &vvbus);
        axp192_read(AXP192_VBUS_CURRENT, &ivbus);
        axp192_read(AXP192_TEMP, &temp);
        axp192_read(AXP192_BATTERY_POWER, &pbat);
        axp192_read(AXP192_BATTERY_VOLTAGE, &vbat);
        axp192_read(AXP192_CHARGE_CURRENT, &icharge);
        axp192_read(AXP192_DISCHARGE_CURRENT, &idischarge);
        axp192_read(AXP192_APS_VOLTAGE, &vaps);

        ESP_LOGI(TAG,
            "vacin: %.2fV iacin: %.2fA vvbus: %.2fV ivbus: %.2fA temp: %.0fC",
            vacin, iacin, vvbus, ivbus, temp
        );

        ESP_LOGI(TAG,
            "pbat: %.2fmW vbat: %.2fV icharge: %.2fA idischarge: %.2fA, vaps: %.2fV",
            pbat, vbat, icharge, idischarge, vaps
        );

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    rtc.year = 2020;
    rtc.month = 12;
    rtc.day = 31;
    rtc.hours = 23;
    rtc.minutes = 59;
    rtc.seconds = 45;

    i2c_hal_master_init();
    axp192_init(i2c_hal_master_read, i2c_hal_master_write);
    bm8563_init(i2c_hal_master_read, i2c_hal_master_write);
    bm8563_write(&rtc);

    ESP_LOGI(TAG, "Heap after init: %d", esp_get_free_heap_size());

    xTaskCreatePinnedToCore(demo_task, "Demo", 4096, NULL, 1, NULL, 1);
}
