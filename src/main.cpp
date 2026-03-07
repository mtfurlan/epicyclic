#include <crsf.h>

#include <hardware/gpio.h>
#include <pico/binary_info.h>
#include <pico/stdlib.h>

#include <stdio.h>

#define LED_R 18
#define LED_G 19
#define LED_B 20

#define CRSF_UART_TX 0
#define CRSF_UART_RX 1

#define SCREEN_CLEAR          "\033[H\033[J"
#define SCREEN_SET_LINE0_COL0 "\033[0;0H"


void cb(const crsf_packet_t* data)
{
    switch (data->header.type) {
        case CSRF_FRAMETYPE_RC_CHANNELS_PACKED_PAYLOAD:
            break;
        case CSRF_FRAMETYPE_LINK_STATISTICS:
            print_packet(data, false);
            break;
        default:
            break;
    }
}


crsf_t c = CRSF_DEFINE();


int main()
{
    bi_decl(bi_program_description("This is a test binary."));

    stdio_init_all();

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);


    printf("hello asdf\n");
    gpio_set_function(CRSF_UART_TX, UART_FUNCSEL_NUM(uart0, CRSF_UART_TX));
    gpio_set_function(CRSF_UART_RX, UART_FUNCSEL_NUM(uart0, CRSF_UART_RX));
    uart_init(uart0, 416666);


    while (1) {
        uint8_t currentByte = uart_getc(uart0);
        crsf_process(&c, &currentByte, 1, cb);
        if (currentByte == 0xC8) {
            //    printf("\r\n%02X ", currentByte);
            gpio_put(LED_R, 0);
            gpio_put(LED_G, 0);
            gpio_put(LED_B, 0);

        } else {
            gpio_put(LED_R, 1);
            gpio_put(LED_G, 1);
            gpio_put(LED_B, 1);
            //    printf("%02X ", currentByte);
        }
    }
}
