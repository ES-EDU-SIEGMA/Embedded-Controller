#include "common.h"
#include "dispenser.h"
#include "helper.h"
#include "rondell.h"
#include <hardware/adc.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>
#include <stdio.h>

/* region VARIABLES/DEFINES */

#define SERIAL_UART SERIAL2 /// The uart Pins to be used
#define NUMBER_OF_DISPENSERS 1
#define MS_DISPENSERS_ARE_MOVING_UP_0 7500
#define DISPENSER_SEARCH_TIMEOUT 750
dispenser_t dispenser[NUMBER_OF_DISPENSERS]; /// Array containing the dispenser

#define INPUT_BUFFER_LEN 255 /// maximum count of allowed input length
size_t characterCounter;
char *inputBuffer;

/* endregion VARIABLES/DEFINES */

int main() {
    initHardware(false, 30);
    establishConnectionWithController("LEFT");
    initDispenser();
    initializeMessageHandler(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        // watchdog update needs to be performed frequent, otherwise the device will crash
        resetWatchdogTimer();

        /* region Handle received character */

        char input = getchar_timeout_us(10000000);

        if ((int)input == PICO_ERROR_TIMEOUT) {
            PRINT_DEBUG("No command received! Timeout reached.")
            continue;
        }

        if (!isAllowedCharacter(input)) {
            PRINT_DEBUG("Received '%c' which is not allowed. It will be ignored", input)
            continue;
        }

        if (isMessageToLong(characterCounter, INPUT_BUFFER_LEN)) {
            resetMessageBuffer(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
            PRINT_DEBUG("Input too long! Flushed buffer.")
            continue;
        }

        if (isLineEnd(input)) {
            handleMessage(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
            PRINT_COMMAND("READY")
            continue;
        }

        storeCharacter(inputBuffer, &characterCounter, input);

        /* endregion handle received character */
    }
#pragma clang diagnostic pop
}

/* region HELPER FUNCTIONS */

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
        PRINT_DEBUG("Invalid ADC GPIO")
        return;
    }

    adc_init();
    adc_gpio_init(gpio);
    adc_select_input(adcInputPin);
}

void initDispenser(void) {
    initialize_adc(28);
    dispenserCreate(&dispenser[0], 0, SERIAL_UART, MS_DISPENSERS_ARE_MOVING_UP_0,
                    DISPENSER_SEARCH_TIMEOUT);
    setUpRondell(2, SERIAL2);
}

void processMessage(char *message, size_t *messageLength) {
    PRINT_DEBUG("Process message len: %d", *messageLength)
    for (uint8_t i = 0; i < 4; ++i) {
        uint32_t dispenserHaltTimes = parseInputString(&message);
        dispenserSetHaltTime(&dispenser[0], dispenserHaltTimes);
        if (dispenserHaltTimes > 0) {
            moveToDispenserWithId(i);
            absolute_time_t time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
            do {
                sleep_until(time);
                time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
                dispenserDoStep(&dispenser[0]);
            } while (!dispenserSetAllToSleepState(dispenser, 1));
        }
    }
}

/* endregion HELPER FUNCTIONS */
