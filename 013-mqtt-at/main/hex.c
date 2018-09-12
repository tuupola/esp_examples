//#include <stdlib.h>
//#include <stddef.h>
// #include <stdbool.h>
// #include <string.h>
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