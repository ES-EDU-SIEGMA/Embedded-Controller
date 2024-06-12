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

#define NUMBER_OF_DISPENSER 4
dispenser_t dispenser; /// Array containing the dispenser

#define INPUT_BUFFER_LEN 255 /// maximum count of allowed input length
size_t characterCounter;
char inputBuffer[INPUT_BUFFER_LEN];

bool dispenserInitialized = false;

/* endregion VARIABLES/DEFINES */

/* region HELPER FUNCTIONS */

/*
 * @param gpio[in] GPIO of the ADC (26-29)
 */
void initializeAdc(uint8_t gpio) {
    adc_init();
    adc_gpio_init(gpio);
    adc_select_input(gpio - 26);

    PRINT("Initialized ADC %u (GPIO %u)", gpio - 26, gpio);
}

void initDispenser(void) {
    PRINT("activate motor");
    initializeAndActivateMotorsEnablePin(); // Enable TMC2209 drivers

    PRINT("init rondell");
    createRondell(MOTOR_ADDRESS_2);

    PRINT("init dispenser");
    dispenserCreate(&dispenser, MOTOR_ADDRESS_0, 4, true);

    dispenserInitialized = true;
}

void processMessage(char *message, size_t messageLength) {
    PRINT("Process message len: %u", messageLength);
    PRINT("Message: %s", message);

    if (strstr("i", message) != NULL) {
        PRINT_COMMAND("%s", CONTROLLER_ID);
        if (dispenserInitialized) {
            PRINT_COMMAND("CALIBRATED");
        }
        return;
    }

    for (uint8_t i = 0; i < NUMBER_OF_DISPENSER; ++i) {
        uint32_t dispenserHaltTime = parseInputString(&message);
        if (dispenserHaltTime > 0) {
            dispenser.haltTime = dispenserHaltTime;
            watchdog_update();
            moveToDispenserWithId(i);
            do {
                watchdog_update();
                dispenserExecuteNextState(&dispenser);
            } while (!dispenserInSleepState(&dispenser));
        }
    }
}

_Noreturn void run() {
    resetMessageBuffer(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);

//    watchdog_enable(60 * 1000, true);

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
    initializeAdc(27);
    initDispenser();

    run();
}
