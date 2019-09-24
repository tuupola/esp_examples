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

#ifndef _FPS2_H
#define _FPS2_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdbool.h>

static inline float fps2(bool reset)
{
    static uint32_t ticks; /* TickType_t */
    static uint32_t start; /* TickType_t */
    static uint32_t frames = 1;
    static float current = 0;
    static bool firstrun = true;

    float smoothing = 0.9; /* Larger value is more smoothing. */
    float measured = 0;

    if (reset) {
        start = xTaskGetTickCount();
        frames = 1;
        current = 0;
        firstrun = false;
    }

    if (firstrun) {
        start = xTaskGetTickCount();
        firstrun = false;
    }
    frames++;

    ticks = xTaskGetTickCount() - start;
    measured = frames / (float) ticks * pdMS_TO_TICKS(1000);
    measured = (measured * smoothing) + (current * (1.0 - smoothing));

    return measured;
}

#endif