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

#include <string.h>

#include "esp_log.h"

#include "plankton.h"
#include "plankton_ll.h"

static const char *TAG = "plankton_ll";

#define swap(x,y) do \
    { unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
        memcpy(swap_temp,&y,sizeof(x)); \
        memcpy(&y,&x,       sizeof(x)); \
        memcpy(&x,swap_temp,sizeof(x)); \
    } while (0)

void pln_ll_put_pixel(uint16_t x1, uint16_t y1, uint16_t color)
{
	ili9431_putpixel(g_spi, x1, y1, color);
}

void pln_ll_put_bitmap(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    ili9431_bitmap(g_spi, x1, y1, w, h, bitmap);
}

void pln_ll_draw_hline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour)
{
    if (x1 > x2) {
        swap(x1, x2);
    }

    if (y1 > y2) {
        swap(y1, y2);
    }

    uint16_t size = x2 - x1;
    uint16_t bitmap[size];

    for (uint16_t i = 0; i < size; i++) {
        ((uint16_t *)bitmap)[i] = colour;
    }

    ili9431_bitmap(g_spi, x1, y1, size, 1, &bitmap);
}

void pln_ll_draw_vline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t colour)
{
    if (x1 > x2) {
        swap(x1, x2);
    }

    if (y1 > y2) {
        swap(y1, y2);
    }

    uint16_t size = y2 - y1;
    uint16_t bitmap[size];

    for (uint16_t i = 0; size < size; i++) {
        ((uint16_t *)bitmap)[i] = colour;
    }

    ili9431_bitmap(g_spi, x1, y1, 1, size, &bitmap);
}