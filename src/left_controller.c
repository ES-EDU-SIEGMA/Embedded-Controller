#define SOURCE_FILE "MAIN-LEFT"

#include "common.h"
#include "dispenser.h"
#include "helper.h"
#include <pico/stdlib.h> /// must be included -> sets clocks required for watchdog-timer!!
#include <stdio.h>

/* region VARIABLES/DEFINES */

#define NUMBER_OF_DISPENSERS 4
dispenser_t dispenser[NUMBER_OF_DISPENSERS]; /// Array containing the dispenser

#define INPUT_BUFFER_LEN 255 /// maximum count of allowed input length
size_t characterCounter;
char *inputBuffer;
bool calibratedLeft = false;
/* endregion VARIABLES/DEFINES */
int main() {
    initHardware(false);
    calibratedLeft = initDispenser();

    if(!calibratedLeft){
        initDispenser();
    }
    initializeMessageHandler(&inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
    setUpWatchdog(60);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        // watchdog update needs to be performed frequent, otherwise the device will crash
        resetWatchdogTimer();

        /* region Handle received character */
        establishConnectionWithController("LEFT");
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
#pragma clang diagnostic pop
}

/* region HELPER FUNCTIONS */
bool initDispenser(void) {
    dispenserCreate(&dispenser[0], 0, 4);
    dispenserCreate(&dispenser[1], 1, 4);
    dispenserCreate(&dispenser[2], 2, 4);
    dispenserCreate(&dispenser[3], 3, 4);
    return true;
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

/* endregion HELPER FUNCTIONS */
