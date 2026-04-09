#include "pico_crsf.h"

#include <hardware/gpio.h>

static crsf_t c = CRSF_DEFINE();
static crsf_callback_t cb;

int pico_crsf_init(crsf_callback_t _cb,
                   uint uart_tx,
                   uint uart_rx,
                   uart_inst_t* uart,
                   size_t telemetry_size)
{
    cb = _cb;
    gpio_set_function(uart_rx, UART_FUNCSEL_NUM(uart, uart_tx));
    gpio_set_function(uart_rx, UART_FUNCSEL_NUM(uart, uart_rx));
    uart_init(uart, 416666);
    return 0;
}

int pico_crsf_process()
{
    if (uart_is_readable(uart0)) {
        uint8_t currentByte = uart_getc(uart0);
        crsf_process(&c, &currentByte, 1, cb);
    }
    return 0;
}
