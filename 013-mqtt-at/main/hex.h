// #include <string.h>
// #include <stdio.h>

void byte2hex(unsigned char byte, char *hex);
char char2nibble(char chr);
char nibble2char(unsigned char nibble);
unsigned char hex2byte(const char *hex);
void string2hex(const char *string, char *hex);
void hex2string(char *hex, char *string);
void hex2bytes(char *hex, uint16_t hexlen, unsigned char *bytes, uint16_t strlen);
void bytes2hex(unsigned char *bytes, uint16_t byteslen, char *hex, uint16_t hexlen);
