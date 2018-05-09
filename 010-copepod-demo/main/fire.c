/*

Adapted from Lode's classic fire tutorial: http://lodev.org/cgtutor/fire.html

Copyright (c) 2004-2007, Lode Vandevenne
Copyright (c) 2018, Mika Tuupola

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdbool.h>
#include <stdint.h>

#include <string.h>
#include <stdio.h>

#include <blit.h>
#include <bitmap.h>
#include <esp_log.h>

#include "color.h"
#include "byteswap.h"
#include "bitmap.h"
#include "fire.h"

static const char *TAG = "fire";
static uint16_t g_palette[256];
static uint8_t g_fire[FIRE_HEIGHT][FIRE_WIDTH];

void fire_init(void)
{
    uint16_t color;
    uint8_t r, g, b;
    hsl_t hsl;
    rgb_t rgb;

    /* Generate palette. */
    for (uint16_t x = 0; x < 256; x++) {
        /* Hue goes from 0 to 85: red to yellow. */
        /* Saturation is always the maximum: 255. */
        /* Lightness is 0..255 for x=0..128, and 255 for x=128..255. */
        hsl.h = x / 3;
        hsl.s = 255;
        hsl.l = min(255, x * 2);

        hsl2rgb(&hsl, &rgb);

        color = RGB565(rgb.r, rgb.g, rgb.b);
        g_palette[x] = bswap_16(color);
    }

    /* Clear the fire. */
    memset(g_fire, 0x00, FIRE_HEIGHT * FIRE_WIDTH);
}

void fire_effect(bitmap_t *dst)
{
    uint16_t color;

    /* Random bright line in the bottom. */
    // for (uint16_t x = 0; x < FIRE_WIDTH; x++) {
    //     color = abs(32768 + rand());
    //     g_fire[FIRE_HEIGHT - 1][x] = color;
    // }

    /* The fire effect itself. */
    for(uint16_t y = 0; y < FIRE_HEIGHT - 1; y++) {
        for(uint16_t x = 0; x < FIRE_WIDTH; x++) {
            g_fire[y][x] =
                ((g_fire[(y + 1) % FIRE_HEIGHT][(x - 1 + FIRE_WIDTH) % FIRE_WIDTH]
                + g_fire[(y + 1) % FIRE_HEIGHT][(x) % FIRE_WIDTH]
                + g_fire[(y + 1) % FIRE_HEIGHT][(x + 1) % FIRE_WIDTH]
                + g_fire[(y + 2) % FIRE_HEIGHT][(x) % FIRE_WIDTH])
                * 30) / 130;
        }
    }

    /* Copy everything to the destination bitmap. */
    for(uint16_t y = 0; y < FIRE_HEIGHT - 1; y++) {
        for(uint16_t x = 0; x < FIRE_WIDTH; x++) {
            uint16_t *fireptr = dst->buffer + dst->pitch * y + dst->bpp * x;
            *fireptr = g_palette[g_fire[y][x]];
        }
    }
}


void fire_putchar(char ascii, uint16_t x0, uint16_t y0, uint8_t color, char font[128][8])
{
    uint8_t *ptr = g_fire;
    ptr += (FIRE_WIDTH * y0) + x0;

    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
            ptr += 1;

            bool set = font[(uint8_t)ascii][x] & 1 << y;
            if (set) {
                *(ptr) = color;
            }
        }
        ptr += FIRE_WIDTH - 8;
    }
}