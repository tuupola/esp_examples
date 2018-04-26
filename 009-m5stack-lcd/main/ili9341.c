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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ili9341.h"

static const char *TAG = "ili9341";

DRAM_ATTR static const lcd_init_cmd_t lcd_init_cmds[]={
    {0xCF, {0x00, 0x83, 0X30}, 3},
    {0xED, {0x64, 0x03, 0X12, 0X81}, 4},
    {0xE8, {0x85, 0x01, 0x79}, 3},
    {0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},
    {0xF7, {0x20}, 1},
    {0xEA, {0x00, 0x00}, 2},
    {0xC0, {0x26}, 1},
    {0xC1, {0x11}, 1},
    {0xC5, {0x35, 0x3E}, 2},
    {0xC7, {0xBE}, 1},
    //{0x36, {0x28}, 1},
    {0x36, {0x08}, 1}, // for M5Stack
    {0x3A, {0x55}, 1},
    {0xB1, {0x00, 0x1B}, 2},
    {0xF2, {0x08}, 1},
    {0x26, {0x01}, 1},
    {0xE0, {0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00}, 15},
    {0XE1, {0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F}, 15},
    {0x2A, {0x00, 0x00, 0x00, 0xEF}, 4},
    {0x2B, {0x00, 0x00, 0x01, 0x3f}, 4},
    {0x2C, {0}, 0},
    {0xB7, {0x07}, 1},
    {0xB6, {0x0A, 0x82, 0x27, 0x00}, 4},
    {0x11, {0}, 0x80},
    {0x29, {0}, 0x80},
    {0, {0}, 0xff},
};

/* Uses spi_device_transmit, which waits until the transfer is complete. */
void ili9341_command(spi_device_handle_t spi, const uint8_t command)
{
    spi_transaction_t transaction;

    memset(&transaction, 0, sizeof(transaction));
    transaction.length = 1 * 8; /* Command is 1 byte ie 8 bits. */
    transaction.tx_buffer = &command; /* The data is the cmd itself. */
    transaction.user = (void*)0; /* D/C needs to be set to 0. */
    ESP_ERROR_CHECK(spi_device_transmit(spi, &transaction));
}

/* Uses spi_device_transmit, which waits until the transfer is complete. */
static void ili9341_data(spi_device_handle_t spi, const uint8_t *data, uint16_t length)
{
    spi_transaction_t transaction;

    if (0 == length) { return; };
    memset(&transaction, 0, sizeof(transaction));
    transaction.length = length * 8; /* Length in bits. */
    transaction.tx_buffer = data;
    transaction.user = (void*)1; /* D/C needs to be set to 1. */
    ESP_ERROR_CHECK(spi_device_transmit(spi, &transaction));
}


/* This function is called (in irq context!) just before a transmission starts. */
/* It will set the D/C line to the value indicated in the user field. */
void ili9341_pre_callback(spi_transaction_t *transaction)
{
    int dc=(int)transaction->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void ili9341_wait(spi_device_handle_t spi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;

    /* TODO: This should be all transactions. */
    for (uint8_t i = 0; i <= 5; i++) {
        ESP_ERROR_CHECK(spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY));
        /* Do something with the result. */
    }
}

void ili9341_init(spi_device_handle_t *spi)
{
    uint8_t cmd = 0;

    /* Init non spi gpio. */
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    /* Reset the display. */
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    ESP_LOGD(TAG, "Initialize the display.");

    /* Send all the commands. */
    while (lcd_init_cmds[cmd].databytes != 0xff) { /* 0xff is the end marker. */
        ili9341_command(*spi, lcd_init_cmds[cmd].cmd);
        ili9341_data(*spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes & 0x1F);
        if (lcd_init_cmds[cmd].databytes & 0x80) {
            vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    /* Enable backlight. */
    gpio_set_level(PIN_NUM_BCKL, 1);
}

void ili9431_bitmap(spi_device_handle_t spi, uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    int x;

    int32_t x2 = x1 + w - 1;
    int32_t y2 = y1 + h - 1;

    static spi_transaction_t trans[6];
    uint32_t size = w * h;

    /* In theory, it's better to initialize trans and data only once and hang */
    /* on to the initialized variables. We allocate them on the stack, so we need */
    /* to re-init them each call. */
    for (x = 0; x < 6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if (0 == (x&1)) {
            /* Even transfers are commands. */
            trans[x].length = 8;
            trans[x].user = (void*)0;
        } else {
            /* Odd transfers are data. */
            trans[x].length = 8 * 4;
            trans[x].user = (void*)1;
        }
        trans[x].flags = SPI_TRANS_USE_TXDATA;
    }

    trans[0].tx_data[0] = 0x2A;           //Column Address Set
    trans[1].tx_data[0] = x1 >> 8;              //Start Col High
    trans[1].tx_data[1] = x1 & 0xff;              //Start Col Low
    trans[1].tx_data[2] = x2 >> 8;       //End Col High
    trans[1].tx_data[3] = x2 &0xff;     //End Col Low
    trans[2].tx_data[0] = 0x2B;           //Page address set
    trans[3].tx_data[0] = y1 >> 8;        //Start page high
    trans[3].tx_data[1] = y1 & 0xff;      //start page low
    trans[3].tx_data[2] = y2 >> 8;    //end page high
    trans[3].tx_data[3] = y2 & 0xff;  //end page low
    trans[4].tx_data[0] = 0x2C;           //memory write
    trans[5].tx_buffer = bitmap;        //finally send the line data
    trans[5].length = size * 2 * 8; //320*2*8*PARALLEL_LINES;          //Data length, in bits
    trans[5].flags = 0; //undo SPI_TRANS_USE_TXDATA flag

    for (x = 0; x <= 5; x++) {
        ESP_ERROR_CHECK(spi_device_queue_trans(spi, &trans[x], portMAX_DELAY));
    }

    /* Could do stuff here... */

    ili9341_wait(spi);
}

void ili9431_putpixel(spi_device_handle_t spi, uint16_t x1, uint16_t y1, uint16_t colour)
{
    ili9431_bitmap(spi, x1, y1, 1, 1, &colour);
}

