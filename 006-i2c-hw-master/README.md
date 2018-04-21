## SSD1306 and U8G2

Uses [U8G2](https://github.com/olikraus/u8g2) to output text to a SSD1306 OLED.
Also uses Neil Kolban's [ESP32 HAL layer for U8G2](https://github.com/nkolban/esp32-snippets/tree/master/hardware/displays/U8G2).

```
I (247) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- 11 12 -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
I (3440) main: Allocated 16 bytes for reading from 0x11.
I (3440) main: 0x3ffb34d4   11 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
I (4440) main: 0x3ffb34d4   11 00 00 01 00 64 00 00  00 00 00 00 00 00 00 00  |.....d..........|
I (5440) main: 0x3ffb34d4   11 00 00 02 00 64 00 00  00 00 00 00 00 00 00 00  |.....d..........|
I (6440) main: 0x3ffb34d4   11 00 00 03 00 64 00 00  00 00 00 00 00 00 00 00  |.....d..........|
I (7440) main: 0x3ffb34d4   11 00 00 04 00 64 00 00  00 00 00 00 00 00 00 00  |.....d..........|
I (8440) main: 0x3ffb34d4   11 00 00 05 00 64 00 00  00 00 00 00 00 00 00 00  |.....d..........|
I (9440) main: 0x3ffb34d4   11 00 00 06 00 64 00 00  00 00 00 00 00 00 00 00  |.....d..........|
```