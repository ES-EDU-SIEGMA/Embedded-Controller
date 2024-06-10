#define SOURCE_FILE "RONDELL"

#include "com_protocol.h"
#include "common.h"
#include "dispenser.h"
#include "rondell.h"

#include "hardware/adc.h"
#include "hardware/watchdog.h"
#include "pico/stdlib.h" /// must be included -> sets clocks required for watchdog-timer!!

#include <stdio.h>
#include <string.h>

/* region VARIABLES/DEFINES */

#ifndef CONTROLLER_ID
#error "Controller ID must be defined!"
#endif

#define NUMBER_OF_DISPENSERS 1
dispenser_t dispenser[NUMBER_OF_DISPENSERS]; /// Array containing the dispenser

#define INPUT_BUFFER_LEN 255 /// maximum count of allowed input length
size_t characterCounter;
char inputBuffer[INPUT_BUFFER_LEN];

bool dispenserInitialized = false;

/* endregion VARIABLES/DEFINES */

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
        PRINT("Invalid ADC GPIO");
        return;
    }

    adc_init();
    adc_gpio_init(gpio);
    adc_select_input(adcInputPin);
}

void initDispenser(void) {
    if (dispenserInitialized) {
        return;
    }

    initialize_adc(27);
    createRondell(2);
    dispenserCreate(dispenser, 0, 4, true);
    PRINT_COMMAND("CALIBRATED");
}

void processMessage(char *message, size_t messageLength) {
    PRINT("Process message len: %u", messageLength);
    PRINT("Message: %s", message);

    if (strcmp("i\n", message) == 0) {
        PRINT_COMMAND("%s", CONTROLLER_ID);
        initDispenser();
        PRINT_COMMAND("CALIBRATED");
        return;
    }

    for (uint8_t i = 0; i < 4; ++i) {
        uint32_t dispenserHaltTimes = parseInputString(&message);
        dispenserSetHaltTime(&dispenser[0], dispenserHaltTimes);
        if (dispenserHaltTimes > 0) {
            watchdog_update();
            moveToDispenserWithId(i);
            do {
                watchdog_update();
                dispenserChangeStates(&dispenser[i]);
            } while (!dispenserAllInSleepState(dispenser, 1));
        }
    }
}

_Noreturn void run() {
    resetMessageBuffer(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);

    watchdog_enable(60 * 1000, true);

    while (true) {
        // watchdog update needs to be performed frequent, otherwise the device will crash
        watchdog_update();

        /* region Handle received character */
        int input = getchar_timeout_us(3 * 1000000);

        PRINT("Start Processing Input!");
        if (input == PICO_ERROR_TIMEOUT) {
            PRINT("No command received! Timeout reached.");
            continue;
        }
        PRINT("No timeout reached!");

        if (!isAllowedCharacter(input)) {
            PRINT("Received '%c' which is not allowed. It will be ignored", input);
            continue;
        }
        PRINT("Received valid character");

        if (isMessageToLong(characterCounter, INPUT_BUFFER_LEN)) {
            resetMessageBuffer(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
            PRINT("Input too long! Flushed buffer.");
            continue;
        }
        PRINT("Message Buffer not full!");

        if (isLineEnd(input)) {
            handleMessage(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
            PRINT_COMMAND("READY");
            continue;
        }
        PRINT("Message end not reached!");

        storeCharacter(inputBuffer, &characterCounter, input);

        /* endregion handle received character */
    }
}

/* endregion HELPER FUNCTIONS */

int main() {
    initIO(false);

    run();
}
