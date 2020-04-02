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

#ifndef _I2C_H
#define _I2C_H

#define SPEKTRUM_DATA_LENGTH    16
#define TELE_DEVICE_AIRSPEED    0x11
#define TELE_DEVICE_ALTITUDE    0x12

#define SLAVE_1_DATA_LENGTH     1 /* One byte. */
#define SLAVE_2_DATA_LENGTH     2 /* Two bytes. */
#define I2C_BUFFER_LENGTH       2 /* Two bytes. */

#define I2C_MASTER_SCL         19 /* Use yellow wire. */
#define I2C_MASTER_SDA         18 /* Use green wire. */
#define I2C_MASTER_NUM         I2C_NUM_1
#define I2C_MASTER_TX_BUF_LEN  0
#define I2C_MASTER_RX_BUF_LEN  0
#define I2C_MASTER_FREQ_HZ     100000

#define ACK_CHECK_ENABLE        0x1 /* Master will require ack from slave */
#define ACK_CHECK_DISABLE       0x0
#define ACK_VAL                 0x0
#define NACK_VAL                0x1

void i2c_init();
void i2c_scan();

esp_err_t i2c_probe(uint8_t address);
esp_err_t i2c_read(uint8_t address, uint8_t *buffer, uint16_t length);

#endif

