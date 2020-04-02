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
#include <string.h>
#include <stdio.h>

static char hexlut[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

void byte2hex(unsigned char byte, char *hex)
{
    *hex++ = hexlut[byte >> 4];
    *hex++ = hexlut[byte & 0xf];
}

char char2nibble(char chr)
{
    return chr - (chr <= '9' ? '0' : ('a' - 10));
}

char nibble2char(unsigned char nibble)
{
    return hexlut[nibble];
}

unsigned char hex2byte(const char *hex)
{
    unsigned char byte = char2nibble(*hex++);
    byte <<= 4;
    byte += char2nibble(*hex++);
    return byte;
}

void string2hex(const char *string, char *hex)
{
    size_t len = strlen(string);
    //printf("len:%zu\n", len);

    while (*string != 0x00) {
        unsigned char chr = *string++;
        *hex++ = hexlut[chr >> 4];
        *hex++ = hexlut[chr & 0xf];
    }
    *hex = 0x00;
}

void hex2string(char *hex, char *string)
{
    size_t len = strlen(hex);
    //printf("len:%zu\n", len);

    while (*hex != 0x00) {
        unsigned char byte = char2nibble(*hex++);
        byte <<= 4;
        byte += char2nibble(*hex++);
        *string++ = byte;
    }
    *string++ = 0x00;
}


void hex2bytes(char *hex, uint16_t hexlen, unsigned char *bytes, uint16_t strlen)
{
    while (hexlen--) {
        unsigned char byte = char2nibble(*hex++);
        byte <<= 4;
        byte += char2nibble(*hex++);
        *bytes++ = byte;
    }
}

void bytes2hex(unsigned char *bytes, uint16_t byteslen, char *hex, uint16_t hexlen)
{
    while (byteslen--) {
        unsigned char chr = *bytes++;
        *hex++ = hexlut[chr >> 4];
        *hex++ = hexlut[chr & 0xf];
    }
}

// char* MQTT_CONNECT = "101800044d51545404020078000c45535033325f3466394433431a";

// int main(void)
// {
// //     // unsigned char byte = 'A';
// //     // printf("byte:%d\n", byte);

// //     char hex[32];
// //     // byte2hex(byte, hex);
// //     // printf("hex:%s\n", hex);

// //     // unsigned char byte2 = hex2byte(hex);
// //     // printf("byte2:%d\n", byte2);

// //    char string[255] = "ABC";
// //     // string2hex(string, hex);
// //     // printf("hex:%s\n", hex);

//     //hex2string(hex, string);

//     unsigned char bytes[255];
//     char string[255];

//     hex2bytes(MQTT_CONNECT, 54, bytes, 27);
//     fwrite(bytes, 1, 27, stdout);
//     printf("\n");
//     bytes2hex(bytes, 27, string, 54);
//     fwrite(string, 1, 54, stdout);

//     return 0;
// }