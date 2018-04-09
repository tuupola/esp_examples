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

#include <driver/i2c.h>
#include <esp_log.h>

#include "i2c.h"

static const char* TAG = "i2c";

void i2c_slave_1_init()
{
    int i2c_slave_port = I2C_SLAVE_1_NUM;
    ESP_LOGI(TAG, "Starting I2C slave 1 at port %d.", i2c_slave_port);

    i2c_config_t conf;
    conf.sda_io_num = I2C_SLAVE_1_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = I2C_SLAVE_1_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.mode = I2C_MODE_SLAVE;
    conf.slave.addr_10bit_en = 0;
    conf.slave.slave_addr = TELE_DEVICE_AIRSPEED;

    ESP_ERROR_CHECK(i2c_param_config(i2c_slave_port, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(
        i2c_slave_port,
        conf.mode,
        I2C_SLAVE_1_RX_BUF_LEN,
        I2C_SLAVE_1_TX_BUF_LEN,
        0
    ));
}

void i2c_slave_2_init()
{
    int i2c_slave_port = I2C_SLAVE_2_NUM;
    ESP_LOGI(TAG, "Starting I2C slave 2 at port %d.", i2c_slave_port);

    i2c_config_t conf;
    conf.sda_io_num = I2C_SLAVE_2_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_DISABLE;
    conf.scl_io_num = I2C_SLAVE_2_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_DISABLE;
    conf.mode = I2C_MODE_SLAVE;
    conf.slave.addr_10bit_en = 0;
    conf.slave.slave_addr = TELE_DEVICE_ALTITUDE;

    ESP_ERROR_CHECK(i2c_param_config(i2c_slave_port, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(
        i2c_slave_port,
        conf.mode,
        I2C_SLAVE_2_RX_BUF_LEN,
        I2C_SLAVE_2_TX_BUF_LEN,
        0
    ));
}

