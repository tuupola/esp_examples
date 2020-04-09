## M5StickC BM8563 RTC demo

Demonstrates M5StickC RTC usage. Uses the [hardware agnostic I2C driver for BM8563 RTC](https://github.com/tuupola/bm8563).

```
$ make menuconfig
$ make flash
$ make monitor

...
I (294) cpu_start: Starting scheduler on PRO CPU.
I (0) cpu_start: Starting scheduler on APP CPU.
I (305) main: SDK version: v4.1-dev-281-g96b96ae24
I (315) main: Heap when starting: 298004
I (315) esp_i2c_hal: Starting I2C master at port 1.
I (325) main: Heap after init: 298820
I (325) main: RTC: 2020-12-31 23:59:45
I (1325) main: RTC: 2020-12-31 23:59:46
I (2325) main: RTC: 2020-12-31 23:59:47
I (3325) main: RTC: 2020-12-31 23:59:48
I (4325) main: RTC: 2020-12-31 23:59:49
I (5325) main: RTC: 2020-12-31 23:59:50
I (6325) main: RTC: 2020-12-31 23:59:51
I (7325) main: RTC: 2020-12-31 23:59:52
I (8325) main: RTC: 2020-12-31 23:59:53
I (9325) main: RTC: 2020-12-31 23:59:54
```