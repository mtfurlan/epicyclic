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
#define MOTOR_L 4 // slice 2A
#define MOTOR_R 5 // slice 2B
#define WEAPON  6 // slice 3A
//#define CHAN4 7 // slice 3B

#define CRSF_UART_TX 0
#define CRSF_UART_RX 1

#define SCREEN_CLEAR          "\033[H\033[J"
#define SCREEN_SET_LINE0_COL0 "\033[0;0H"


static absolute_time_t connected_timeout;

void disarm()
{
    pwm_set_both_levels(2, 0, 0); // chan1, chan2
    pwm_set_both_levels(3, 0, 0); // 3,4
    gpio_put(LED_B, 1);
}

// https://xiaoxiae.github.io/Robotics-Simplified-Website/drivetrain-control/arcade-drive/
void doDriving(uint16_t forward_us, uint16_t turn_us, bool flip)
{
    // need to have zero centerd shit for this algorithm
    // we have rc μs
    int16_t drive = forward_us - 1500;
    // TODO: inverted?
    int16_t rotate = -1 * (turn_us - 1500);


    uint16_t maximum = std::max(abs(drive), abs(rotate));
    int16_t total = drive + rotate;
    int16_t difference = drive - rotate;

    int16_t left = 0;
    int16_t right = 0;

    if (drive >= 0) {
        if (rotate >= 0) {
            left = maximum;
            right = difference;
        } else {
            left = total;
            right = maximum;
        }
    } else {
        if (rotate >= 0) {
            left = total;
            right = -maximum;
        } else {
            left = -maximum;
            right = difference;
        }
    }

    // TODO flip maybe invert both inputs?
    //if(flip) {
    //    left *= -1;
    //    right *= -1;
    //}

    // left is wired backwards
    left *= -1;

    // convert back to us
    left += 1500;
    right += 1500;


    //printf("rc input drive: %d, rotat %d\n", forward_us, turn_us);
    //printf("steering input drive: %d, rotat %d, max %d, total %d, diff %d\n", drive, rotate, maximum, total, difference);
    //printf("setting motors to L: %d, R: %d\n", left, right);

    pwm_set_both_levels(2, left, right); // chan1 left, chan2 right
}
void cb(const crsf_packet_t* data)
{
    bool armed;
    bool flipped;
    uint16_t forward;
    uint16_t turn;
    uint16_t weapon;

    switch (data->header.type) {
        case CSRF_FRAMETYPE_RC_CHANNELS_PACKED_PAYLOAD:
            connected_timeout = make_timeout_time_ms(100);
            gpio_put(LED_G, 0);
            armed = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_5) > 1500;
            flipped = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_6) > 1500;
            if (armed) {
                gpio_put(LED_B, 0);
                forward = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_2);
                weapon = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_3);
                turn = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_4);
                doDriving(forward, turn, flipped);

                pwm_set_gpio_level(WEAPON, weapon);
            } else {
                disarm();
            }

            break;
        case CSRF_FRAMETYPE_LINK_STATISTICS:
            //print_packet(data, false);
            break;
        default:
            break;
    }
}

crsf_t c = CRSF_DEFINE();

int my_pwm_init()
{
    gpio_set_function(MOTOR_L, GPIO_FUNC_PWM);
    gpio_set_function(MOTOR_R, GPIO_FUNC_PWM);
    gpio_set_function(WEAPON, GPIO_FUNC_PWM);

    uint32_t clock = clock_get_hz(clk_sys);

    // set period to 20ms, or 50Hz
    // clock(Hz) / wrap = 50(Hz)
    // clock = wrap * 50
    // wrap = clock/50

    pwm_set_gpio_level(MOTOR_L, 0);
    pwm_set_gpio_level(MOTOR_R, 0);
    pwm_set_gpio_level(WEAPON, 0);


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
        if (time_reached(connected_timeout)) {
            // NO LONGER CONNECTED
            disarm();
            gpio_put(LED_G, 1);
        }
        if (uart_is_readable(uart0)) {
            uint8_t currentByte = uart_getc(uart0);
            crsf_process(&c, &currentByte, 1, cb);
        }
    }
}
