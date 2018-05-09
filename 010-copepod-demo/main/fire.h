#include <stdint.h>
#include <bitmap.h>

#define FIRE_WIDTH          106 /* 320 div 3 */
#define FIRE_HEIGHT         80 /* 240 div 3 */

void fire_init();
void fire_effect(bitmap_t *dst);
void fire_putchar(char ascii, uint16_t x0, uint16_t y0, uint8_t color, char font[128][8]);
