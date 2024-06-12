#define SOURCE_FILE "SIDE-CONTROLLER"

#include "com_protocol.h"
#include "common.h"
#include "dispenser.h"

#include "hardware/watchdog.h"
#include "pico/stdlib.h" /// must be included -> sets clocks required for watchdog-timer!!

#include <stdio.h>
#include <string.h>

/* region VARIABLES/DEFINES */

#ifndef CONTROLLER_ID
#error "Controller ID must be defined!"
#endif

#define NUMBER_OF_DISPENSERS 4

dispenser_t dispenser[NUMBER_OF_DISPENSERS]; //!< Array containing the dispenser

#define INPUT_BUFFER_LEN 255 //!< maximum count of allowed input length
size_t characterCounter;
char inputBuffer[INPUT_BUFFER_LEN];

bool dispenserInitialized = false;

/* endregion VARIABLES/DEFINES */

/* region FUNCTIONS */

void initDispenser(void) {
    if (dispenserInitialized) {
        return;
    }

    PRINT("init motor");
    initializeAndActivateMotorsEnablePin(); // Enable TMC2209 drivers

    PRINT("init dispenser");
    dispenserCreate(&dispenser[0], 0, 4, false);
    dispenserCreate(&dispenser[1], 1, 4, false);
    dispenserCreate(&dispenser[2], 2, 4, false);
    dispenserCreate(&dispenser[3], 3, 4, false);

    dispenserInitialized = true;
}

void processMessage(char *message, size_t messageLength) {
    bool activeDispenser[NUMBER_OF_DISPENSERS] = {false};

    PRINT("Process message len: %u", messageLength);
    PRINT("Message: %s", message);

    if (strstr("i", message) != NULL) {
        PRINT_COMMAND("%s", CONTROLLER_ID);
        if (dispenserInitialized) {
            PRINT_COMMAND("CALIBRATED");
        }
        return;
    }

    for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
        uint32_t haltTime = parseInputString(&message);
        PRINT("Halt Time: %lu", haltTime);
        if (haltTime > 0) {
            activeDispenser[i] = true;
        }
        dispenserSetHaltTime(&dispenser[i], haltTime);
    }

    for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
        dispenserErrorStateCheck(&dispenser[i]);
    }

    do {
        watchdog_update();

        for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
            if (activeDispenser[i] == true) {
                dispenserExecuteNextState(&dispenser[i]);
                if (getDispenserState(&dispenser[i]) == DISPENSER_STATE_SLEEP) {
                    activeDispenser[i] = false;
                }
            }
        }
    } while (!dispenserAllInSleepState(dispenser, NUMBER_OF_DISPENSERS));
}

_Noreturn void run(void) {
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

/* endregion FUNCTIONS */

int main() {
    initIO(false);
    initDispenser();

    run();
}
