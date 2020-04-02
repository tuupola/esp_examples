/*

SPDX-License-Identifier: MIT

MIT License

Copyright (c) 2018-2020 Mika Tuupola

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
#include <esp_log.h>

esp_err_t sim8xx_init();
esp_err_t sim8xx_register_network();
esp_err_t sim8xx_enable_gprs();
esp_err_t sim8xx_tcp_connect();
esp_err_t sim8xx_tcp_write_bytes(const char *bytes, size_t length);


void sim8xx_reset();
void sim8xx_time();
void sim8xx_imei();
void sim8xx_power_off();
void sim8xx_power_on();
void sim8xx_power_rf_off();
void sim8xx_power_rf_on();
void sim8xx_disable_gprs();
void sim8xx_tcp_disconnect();

void sim8xx_udp_connect();
void sim8xx_udp_disconnect();
void sim8xx_udp_write_bytes();

