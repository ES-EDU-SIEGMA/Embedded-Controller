#define SOURCE_FILE "MAIN-RIGHT"

#include "common.h"
#include "dispenser.h"
#include "helper.h"
#include <pico/stdlib.h> /// must be included -> sets clocks required for watchdog-timer!!

/* region VARIABLES/DEFINES */

#define SERIAL_UART SERIAL2 /// The uart Pins to be used
#define NUMBER_OF_DISPENSERS 4

#define DISPENSER_SEARCH_TIMEOUT 250
dispenser_t dispenser[NUMBER_OF_DISPENSERS]; /// Array containing the dispenser

#define INPUT_BUFFER_LEN 255 /// maximum count of allowed input length
size_t characterCounter;
char *inputBuffer;

/* endregion VARIABLES/DEFINES */

int main() {
    initHardware(false);
    establishConnectionWithController("RIGHT");
    initDispenser();
    initializeMessageHandler(&inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
    setUpWatchdog(60);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
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
#pragma clang diagnostic pop
}

/* region HELPER FUNCTIONS */

void initDispenser(void) {
    dispenserCreate(&dispenser[0], 0, SERIAL_UART, 4,
                    DISPENSER_SEARCH_TIMEOUT);
    dispenserCreate(&dispenser[1], 1, SERIAL_UART, 4,
                    DISPENSER_SEARCH_TIMEOUT);
    dispenserCreate(&dispenser[2], 2, SERIAL_UART, 4,
                    DISPENSER_SEARCH_TIMEOUT);
    dispenserCreate(&dispenser[3], 3, SERIAL_UART, 4,
                    DISPENSER_SEARCH_TIMEOUT);

    PRINT_COMMAND("CALIBRATED")
}

void processMessage(char *message, size_t messageLength) {
    uint8_t dispensersTrigger = 0;

    PRINT_DEBUG("Process message len: %u", messageLength)
    PRINT_DEBUG("Message: %s", message)
    for (uint8_t i = 0; i < 4; ++i) {
        uint32_t dispenserHaltTimes = parseInputString(&message);
        if (dispenserHaltTimes > 0) {
            dispensersTrigger++;
        }
        dispenserSetHaltTime(&dispenser[i], dispenserHaltTimes);
    }
    for (int i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
        dispenser[i].othersTriggered = dispensersTrigger;
    }
    absolute_time_t time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
    do {
        resetWatchdogTimer();
        sleep_until(time);
        time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
        // Checks for each dispenser if their next state is reached and perform the according action
        for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
            motorDoStep(&dispenser[i]);
        }
        // When all dispensers are finished, they are in the state sleep
    } while (!dispenserAllInSleepState(dispenser, NUMBER_OF_DISPENSERS));
}

/* endregion HELPER FUNCTIONS */
