/*

This code is based on Espressif provided SPI Master example which was
released to Public Domain: https://goo.gl/ksC2Ln


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

#include <stdint.h>
#include <driver/spi_master.h>

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   14

#define PIN_NUM_DC   27
#define PIN_NUM_RST  33
#define PIN_NUM_BCKL 32

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

void ili9431_bitmap(spi_device_handle_t spi, uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t *bitmap);
void ili9431_putpixel(spi_device_handle_t spi, uint16_t x1, uint16_t y1, uint16_t colour);
void ili9341_wait(spi_device_handle_t spi);
void ili9341_init(spi_device_handle_t *spi);
void ili9341_pre_callback(spi_transaction_t *transaction);
