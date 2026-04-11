#include "batt.h"

#include <hardware/adc.h>


// 12-bit conversion, and ADC_VREF of 3V3 should equal 13V, or 1 << 12 should be 13.0
// measured:
// 12.40V  == 3837
const float batt_conversion_factor = 12.40f / 3837; // multiply by this to get float voltage


int batt_init(uint gpio, uint adc)
{
    adc_init();

    adc_gpio_init(gpio);
    adc_select_input(adc);
    return 0;
}
float batt_voltage()
{
    uint16_t batt_reading = adc_read();
    return batt_reading * batt_conversion_factor;
}
