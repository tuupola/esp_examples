## M5Stack fire effect

![M5Stack fullscreen fire](https://appelsiini.net/img/m5stack-fire1-1400.jpg)

Some old school effect done while learning to program C. Better explanation in the [blog post](https://appelsiini.net/2018/esp32-fire-effect/). To compile make sure the submodules are initialized.

```
$ git clone git@github.com:tuupola/esp-examples.git
$ cd esp-examples
$ git submodule update --init
$ cd 010-m5stack-fire
$ make menuconfig
$ make flash
```
