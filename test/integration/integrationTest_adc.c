#define SOURCE_FILE "ADC_TEST"

#include <stdlib.h>

#include <hardware/adc.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#include "common.h"

#define GPIO_PIN 28

static void initPico(void) {
    if (watchdog_enable_caused_reboot()) {
        reset_usb_boot(0, 0);
    }

    stdio_init_all();

    // Give components time for boot up
    sleep_ms(2500);

    while (!stdio_usb_connected()) {
        // waits for usb connection
    }
}

void initialize_adc(uint8_t gpio) {
    uint8_t adcInputPin;
    switch (gpio) {
    case 29:
        adcInputPin = 3;
        break;
    case 28:
        adcInputPin = 2;
        break;
    case 27:
        adcInputPin = 1;
        break;
    case 26:
        adcInputPin = 0;
        break;
    default:
        PRINT("Invalid ADC GPIO");
        return;
    }

    adc_init();
    adc_gpio_init(gpio);
    adc_select_input(adcInputPin);
}

_Noreturn static void runTest(uint8_t gpio) {
    PRINT("##########\n# Start Test");
    while (true) {
        PRINT("OUTPUT: %hu", adc_read());
        PRINT("#####");
        sleep_ms(2000);
    }
}

int main() {
    initPico();
    initialize_adc(GPIO_PIN);
    runTest(GPIO_PIN);
}
