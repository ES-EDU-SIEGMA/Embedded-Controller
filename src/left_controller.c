#define SOURCE_FILE "MAIN-LEFT"

#include "common.h"
#include "dispenser.h"
#include "helper.h"
#include <pico/stdlib.h> /// must be included -> sets clocks required for watchdog-timer!!
#include <stdio.h>
#include <stdlib.h>

/* region VARIABLES/DEFINES */

#define NUMBER_OF_DISPENSERS 4
dispenser_t dispenser[NUMBER_OF_DISPENSERS]; /// Array containing the dispenser

#define INPUT_BUFFER_LEN 255 /// maximum count of allowed input length
size_t characterCounter;
char *inputBuffer;

/* endregion VARIABLES/DEFINES */

/* region HELPER FUNCTIONS */

void initDispenser(void) {
    dispenserCreate(&dispenser[0], 0, 4);
    dispenserCreate(&dispenser[1], 1, 4);
    dispenserCreate(&dispenser[2], 2, 4);
    dispenserCreate(&dispenser[3], 3, 4);
    PRINT_COMMAND("CALIBRATED")
}

void processMessage(char *message, size_t messageLength) {
    uint8_t dispensersTrigger = 0;
    bool triggeredDispensers[NUMBER_OF_DISPENSERS]={false};

    PRINT_DEBUG("Process message len: %u", messageLength)
    PRINT_DEBUG("Message: %s", message)
    for (uint8_t i = 0; i < 4; ++i) {
        uint32_t dispenserHaltTimes = parseInputString(&message);
        if (dispenserHaltTimes > 0) {
            dispensersTrigger++;
            triggeredDispensers[i] = true;
        }
        dispenserSetHaltTime(&dispenser[i], dispenserHaltTimes);
    }

    do {
        resetWatchdogTimer();
        uint8_t topStateCounter = 0;
        for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
            if (getDispenserState(&dispenser[i]) == DISPENSER_STATE_TOP) {
                topStateCounter++;
            }
        }
        for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
            if (getDispenserState(&dispenser[i]) == DISPENSER_STATE_TOP) {
                dispenser[i].dispensersInTopState = topStateCounter;
            }
        }
        for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
            if (triggeredDispensers[i] == true) {
                dispenserChangeStates(&dispenser[i]);
                if (getDispenserState(&dispenser[i]) == DISPENSER_STATE_SLEEP) {
                    triggeredDispensers[i] = false;
                }
            }
        }
        // When all dispensers are finished, they are in the state sleep
    } while (!dispenserAllInSleepState(dispenser, NUMBER_OF_DISPENSERS));
}

_Noreturn void run(void) {
    while (true) {
        // watchdog update needs to be performed frequent, otherwise the device will crash
        resetWatchdogTimer();

        /* region Handle received character */
        int input = getchar_timeout_us(3 * 1000000);

        PRINT_DEBUG("Start Processing Input!")
        if (input == PICO_ERROR_TIMEOUT) {
            PRINT_DEBUG("No command received! Timeout reached.")
            continue;
        }
        PRINT_DEBUG("No timeout reached!")

        if (!isAllowedCharacter(input)) {
            PRINT_DEBUG("Received '%c' which is not allowed. It will be ignored", input)
            continue;
        }
        PRINT_DEBUG("Received valid character")

        if (isMessageToLong(characterCounter, INPUT_BUFFER_LEN)) {
            resetMessageBuffer(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
            PRINT_DEBUG("Input too long! Flushed buffer.")
            continue;
        }
        PRINT_DEBUG("Message Buffer not full!")

        if (isLineEnd(input)) {
            handleMessage(inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
            PRINT_COMMAND("READY")
            continue;
        }
        PRINT_DEBUG("Message end not reached!")

        storeCharacter(inputBuffer, &characterCounter, input);

        /* endregion handle received character */
    }
}

/* endregion HELPER FUNCTIONS */

int main() {
    initHardware(false);
    establishConnectionWithController("LEFT");
    initDispenser();
    initializeMessageHandler(&inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
    setUpWatchdog(60);
    run();
    return EXIT_FAILURE;
}

