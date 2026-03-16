#include <crsf.h>

#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/watchdog.h"
#include <hardware/gpio.h>
#include <pico/binary_info.h>
#include <pico/stdlib.h>

#include <stdio.h>

#define LED_R 18
#define LED_G 19
#define LED_B 20


// each pwm slice drives 2 pwm channels
// only B can be input
// alterante, gpio0 is 0A, gpio1 is 0B, gpio2 is 1A, etc up to 15
// same slices re-used for 16-29
#define CHAN1 4 // slice 2A
#define CHAN2 5 // slice 2B
#define CHAN3 6 // slice 3A
#define CHAN4 7 // slice 3B

#define CRSF_UART_TX 0
#define CRSF_UART_RX 1

#define SCREEN_CLEAR          "\033[H\033[J"
#define SCREEN_SET_LINE0_COL0 "\033[0;0H"


void cb(const crsf_packet_t* data)
{
    switch (data->header.type) {
        case CSRF_FRAMETYPE_RC_CHANNELS_PACKED_PAYLOAD:
            printf("setting CHAN1 to %d\n",
                   CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_1));
            pwm_set_gpio_level(CHAN1,
                               CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_1));
            break;
        case CSRF_FRAMETYPE_LINK_STATISTICS:
            print_packet(data, false);
            break;
        default:
            break;
    }
}

crsf_t c = CRSF_DEFINE();

int my_pwm_init()
{
    gpio_set_function(CHAN1, GPIO_FUNC_PWM);
    gpio_set_function(CHAN2, GPIO_FUNC_PWM);
    gpio_set_function(CHAN3, GPIO_FUNC_PWM);
    gpio_set_function(CHAN4, GPIO_FUNC_PWM);

    uint32_t clock = clock_get_hz(clk_sys);

    // set period to 20ms, or 50Hz
    // clock(Hz) / wrap = 50(Hz)
    // clock = wrap * 50
    // wrap = clock/50

    pwm_set_gpio_level(CHAN1, 0);
    pwm_set_gpio_level(CHAN2, 0);
    pwm_set_gpio_level(CHAN3, 0);
    pwm_set_gpio_level(CHAN4, 0);


    // scale pwm clock to 1MHz
    // wrap is max 65535, so we can't do unscaled
    // also, if we do it this way, you pass the pwm level as μs
    uint32_t pwmClk = 1000000;
    float divider = clock / pwmClk;
    uint32_t top = pwmClk / 50 - 1;

    // we know that the inputs we picked are on slices 2 and 3
    for (size_t slice = 2; slice <= 3; ++slice) {
        pwm_set_clkdiv(slice, divider);
        pwm_set_wrap(slice, top);
        pwm_set_enabled(slice, true);
    }


    return 0;
}

int led_init()
{
    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);

    gpio_put(LED_R, 1);
    gpio_put(LED_G, 1);
    gpio_put(LED_B, 1);
    return 0;
}

int main()
{
    bi_decl(bi_program_description("This is a test binary."));

    stdio_init_all();
    printf("hello asdf\n");

    led_init();

    if (watchdog_enable_caused_reboot()) {
        gpio_put(LED_R, 0);
    }

    watchdog_enable(100, 1);

    gpio_set_function(CRSF_UART_TX, UART_FUNCSEL_NUM(uart0, CRSF_UART_TX));
    gpio_set_function(CRSF_UART_RX, UART_FUNCSEL_NUM(uart0, CRSF_UART_RX));
    uart_init(uart0, 416666);

    my_pwm_init();

    while (1) {
        watchdog_update();
        if (uart_is_readable(uart0)) {
            uint8_t currentByte = uart_getc(uart0);
            crsf_process(&c, &currentByte, 1, cb);
        }
    }
}
