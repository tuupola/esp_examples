/*

SPDX-License-Identifier: MIT-0

MIT No Attribution

Copyright (c) 2017 Neil Kolban
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

#include <driver/i2c.h>
#include <esp_log.h>

#include "i2c.h"

static const char* TAG = "i2cscanner";

void i2c_scan()
{
    ESP_LOGD(TAG, "Scanning I2C bus.");

    uint8_t address;
    esp_err_t result;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:         ");
    for (address = 3; address < 0x78; address++) {
    result = i2c_probe(address);

    if (address % 16 == 0) {
        printf("\n%.2x:", address);
    }
    if (result == ESP_OK) {
        printf(" %.2x", address);
    } else {
        printf(" --");
    }
}
    printf("\n");
}
