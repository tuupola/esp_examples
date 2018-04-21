## Software I2C master

Uses ESP32 software I2C master to read from two slave devices at addresses `0x11` and `0x12`. See [005-i2c-slave](https://github.com/tuupola/esp32-examples/tree/master/005-i2c-slave) for slave device code.

```
...
I (0) cpu_start: Starting scheduler on APP CPU.
I (247) cpu_start: Starting scheduler on PRO CPU.
D (317) software_i2c: Initializing software i2c with clock pin 19.
D (317) i2cscanner: Scanning I2C bus.
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- 11 12 -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
I (260) main: Reading from 0x11.
I (260) main: 0x3ffafff4   00                                                |.|
I (270) main: Reading from 0x12.
I (270) main: 0x3ffb3418   00 00                                             |..|
...
```