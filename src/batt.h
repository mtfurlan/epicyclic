#pragma once
#include <stdint.h>

#include <pico/types.h>

int batt_init(uint gpio, uint adc);

/*
 * return battery voltage in V
 */
float batt_voltage();
