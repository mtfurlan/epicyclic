#include "batt.h"
#include "pico_crsf.h"


#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <hardware/watchdog.h>
#include <pico/binary_info.h>
#include <pico/stdlib.h>

#include <stdio.h>

#define LED_R 17
#define LED_G 16
#define LED_B 25


// battery divider of 15kΩ to 5.1kΩ
// ADC0 is on pin 26
#define BATT_DIVIDER_GPIO 26
#define BATT_DIVIDER_ADC  0

float battery_min = 9.9;

// each pwm slice drives 2 pwm channels
// only B can be input
// alterante, gpio0 is 0A, gpio1 is 0B, gpio2 is 1A, etc up to 15
// code assumes motors are on the same slice, left is A right is B
#define MOTOR_L 2 // slice 1A
#define MOTOR_R 3 // slice 1B
#define WEAPON  4 // slice 2A
//#define CHAN4 7 // slice 3B

#define CRSF_UART_TX 0
#define CRSF_UART_RX 1

#define SCREEN_CLEAR          "\033[H\033[J"
#define SCREEN_SET_LINE0_COL0 "\033[0;0H"


static absolute_time_t connected_timeout;
static absolute_time_t battery_read_time;

void disarm()
{
    pwm_set_gpio_level(WEAPON, 0);
    pwm_set_both_levels(PWM_GPIO_SLICE_NUM(MOTOR_L), 0, 0);
    gpio_put(LED_B, 1);
}

// https://xiaoxiae.github.io/Robotics-Simplified-Website/drivetrain-control/arcade-drive/
void doSingleStick(int16_t* left, int16_t* right, int16_t drive, int16_t rotate)
{
    //printf("drive: %d, rotate: %d \n", drive, rotate);
    uint16_t maximum = std::max(abs(drive), abs(rotate));
    int16_t total = drive + rotate;
    int16_t difference = drive - rotate;

    if (drive >= 0) {
        if (rotate >= 0) {
            *left = maximum;
            *right = difference;
        } else {
            *left = total;
            *right = maximum;
        }
    } else {
        if (rotate >= 0) {
            *left = total;
            *right = -maximum;
        } else {
            *left = -maximum;
            *right = difference;
        }
    }
}

void handleDriving(uint16_t l, uint16_t r, bool flipped)
{
    int16_t left = l - 1500;
    int16_t right = r - 1500;

    // comment out for tank drive
    doSingleStick(&left, &right, left, right);

    // right is reversed
    right *= -1;
    if (flipped) {
        l = right + 1500;
        r = left + 1500;
    } else {
        l = left + 1500;
        r = right + 1500;
    }
    //printf("setting motors to L: %d, R: %d\n", left, right);
    pwm_set_both_levels(pwm_gpio_to_slice_num(MOTOR_L), l, r);
}



typedef enum { WEAPON_OFF = 0, WEAPON_TRIM, WEAPON_FULL } weapon_mode_e;

void handleWeapon(weapon_mode_e weapon_mode, uint16_t weapon_trim)
{
    switch (weapon_mode) {
        case WEAPON_TRIM:
            pwm_set_gpio_level(WEAPON, weapon_trim);
            break;
        case WEAPON_FULL:
            pwm_set_gpio_level(WEAPON, 2000);
            break;
        default:
            pwm_set_gpio_level(WEAPON, 1000);
            break;
    }

}
int crsf2enum(uint16_t input)
{
    // 1000 is 0
    // 1500 is 1
    // 2000 is 2
    if (input < 1250) {
        return 0;
    } else if (input < 1750) {
        return 1;
    } else {
        return 2;
    }
}
void cb(const crsf_packet_t* data)
{
    bool armed;
    bool flipped;
    weapon_mode_e weapon_mode;
    uint16_t driveL;
    uint16_t driveR;
    uint16_t weapon_trim;

    switch (data->header.type) {
        case CSRF_FRAMETYPE_RC_CHANNELS_PACKED_PAYLOAD:
            connected_timeout = make_timeout_time_ms(100);
            gpio_put(LED_G, 0);
            armed = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_5) > 1500;
            if (armed) {
                gpio_put(LED_B, 0);
                driveL = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_1);
                driveR = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_2);
                weapon_trim = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_3);
                flipped = CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_6) > 1500;
                weapon_mode = (weapon_mode_e)crsf2enum(
                        CRSF_TICKS_TO_US(data->rc_channels_packed_payload.channel_7));

                handleDriving(driveL, driveR, flipped);
                handleWeapon(weapon_mode, weapon_trim);
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

    disarm();


    // scale pwm clock to 1MHz
    // wrap is max 65535, so we can't do unscaled
    // also, if we do it this way, you pass the pwm level as μs
    uint32_t pwmClk = 1000000;
    float divider = clock / pwmClk;
    uint32_t top = pwmClk / 50 - 1;

    uint sliceDrive = PWM_GPIO_SLICE_NUM(MOTOR_L);
    uint sliceWeapon = PWM_GPIO_SLICE_NUM(WEAPON);

    pwm_set_clkdiv(sliceDrive, divider);
    pwm_set_wrap(sliceDrive, top);
    pwm_set_enabled(sliceDrive, true);

    pwm_set_clkdiv(sliceWeapon, divider);
    pwm_set_wrap(sliceWeapon, top);
    pwm_set_enabled(sliceWeapon, true);


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

    my_pwm_init();

    pico_crsf_init(cb, CRSF_UART_TX, CRSF_UART_RX, uart0, 5);

    batt_init(BATT_DIVIDER_GPIO, BATT_DIVIDER_ADC);

    if (watchdog_enable_caused_reboot()) {
        gpio_put(LED_R, 0);
    }

    watchdog_enable(100, 1);


    while (1) {
        watchdog_update();
        if (time_reached(connected_timeout)) {
            // NO LONGER CONNECTED
            disarm();
            gpio_put(LED_G, 1);
        }
        pico_crsf_process();

        static uint8_t batt_min_count = 0;
        if (time_reached(battery_read_time)) {
            battery_read_time = make_timeout_time_ms(1000);
            float batt_reading = batt_voltage();
            printf("battery at %fV\n", batt_reading);

            if (batt_reading <= battery_min) {
                if(batt_min_count < 3) {
                    batt_min_count++;
                } else {
                    watchdog_disable();
                    while (batt_reading <= battery_min) {
                        printf("battery low at %fV, no more movement\n", batt_reading);
                        disarm();
                        gpio_put(LED_R, !gpio_get_out_level(LED_R));
                        sleep_ms(1000);
                        batt_reading = batt_voltage();
                    }
                    printf("battery recovered, at %fV\n", batt_reading);
                    watchdog_enable(100, 1);
                    gpio_put(LED_R, 1);
                    batt_min_count=0;
                }
            } else {
                batt_min_count=0;
            }
        }
    }
}
