# RP2040 rc robot

## Parts
* seed xaio-RP2040
    * [schematic](https://files.seeedstudio.com/wiki/XIAO-RP2040/res/Seeed-Studio-XIAO-RP2040-v1.3.pdf)
    * [diagram](https://media-cdn.seeedstudio.com/media/wysiwyg/rp2040_pinout.png)
        * NOTE: D$n isn't real, look at P$n
* [radiomaster RP2 V2 ELRS radio](https://radiomasterrc.com/products/rp2-expresslrs-2-4ghz-nano-receiver)
* [Repeat Robotics Scalar kit(old version)](https://repeat-robotics.com/collections/scalar-robot-kit)

## Wiring
Copied from main.cpp
```
#define CRSF_UART_TX 0
#define CRSF_UART_RX 1
#define MOTOR_L 4
#define MOTOR_R 5
#define WEAPON  6
```
