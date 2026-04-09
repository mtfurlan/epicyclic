#pragma once
/**
 * wrap the crsf lib with things using the pico sdk like the uart
 **/

#include <crsf.h>

#include <hardware/uart.h>
#include <pico/types.h>

int pico_crsf_init(crsf_callback_t cb,
                   uint uart_tx,
                   uint uart_rx,
                   uart_inst_t* uart,
                   size_t telemetry_size);
int pico_crsf_process();
