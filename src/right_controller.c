// Project
#include "common.h"
#include "dispenser.h"

// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pico SDK
#include <hardware/adc.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

/* region VARIABLES/DEFINES */

// maximum count of allowed input length
#define INPUT_BUFFER_LEN 255

// The uart Pins to be used
#define SERIAL_UART SERIAL2

const char *allowedCharacters = "0123456789i;\nn"; /// sequence of allowed character
dispenser_t dispenser[NUMBER_OF_DISPENSERS];       /// Array containing the dispenser

/* endregion VARIABLES/DEFINES */

/* region HELPER FUNCTION PROTOTYPES */

static void initPico(void);

static void initSide(void);

/*! initializes the ADC of the MCU
 *
 * @param gpio  GPIO of the Tiny2040 for the ADC input
 */
static void initialize_adc(uint8_t gpio);

/*! This function's purpose is to establish synchronization between the pico
 * and the pi. The pi sends `i\\n` to the pico corresponding and to confirm
 * that the string has been received the pico sends back `[POSITION]\\n`.
 * This function imprisons the program flow in the while loop until
 * synchronization has been established.
 */
static void establishConnectionToMaster(void);

/* region handle message */

static void resetMessageBuffer(char *buffer, size_t bufferSize, size_t *receivedCharacterCount);

static bool isAllowedCharacter(uint32_t charToCheck);

static bool isLineEnd(uint32_t charToCheck);

static bool isMessageToLong(uint16_t numberOfCharacters);

/*! parse a String to fetch the hopper halt timings
 *
 * @param message The received message containing the halt timing
 * @return        The parsed halt timing as an integer or on failure, a Zero
 */
static uint32_t parseInputString(char **message);

static void processMessage(char *message, size_t *messageLength);

static void handleMessage(char *buffer, size_t maxBufferSize, size_t *receivedCharacterCount);

static void storeCharacter(char *buffer, size_t *bufferIndex, char newCharacter);

/* endregion Handle message */

/* endregion HELPER FUNCTION PROTOTYPES*/

int main() {
    initPico();
    establishConnectionToMaster();

#ifdef RONDELL
    initRondell();
#else
    initSide();
#endif
    PRINT_COMMAND("CALIBRATED")

    /* region init message buffer*/
    char *input_buf = malloc(INPUT_BUFFER_LEN);
    size_t characterCounter;
    resetMessageBuffer(input_buf, INPUT_BUFFER_LEN, &characterCounter);
    /* endregion init message buffer*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
    while (true) {
        // watchdog update needs to be performed frequent, otherwise the device will crash
        watchdog_update();

        /* region Handle received character */

        uint32_t input = getchar_timeout_us(10000000);

        if (input == PICO_ERROR_TIMEOUT) {
            PRINT_DEBUG("No command received! Timeout reached.")
            continue;
        }

        if (!isAllowedCharacter(input)) {
            PRINT_DEBUG("Received '%c' which is not allowed. It will be ignored", input)
            continue;
        }

        if (isMessageToLong(characterCounter)) {
            resetMessageBuffer(input_buf, INPUT_BUFFER_LEN, &characterCounter);
            PRINT_DEBUG("Input too long! Flushed buffer.")
            continue;
        }

        if (isLineEnd(input)) {
            handleMessage(input_buf, INPUT_BUFFER_LEN, &characterCounter);
            PRINT_COMMAND("READY")
            continue;
        }

        storeCharacter(input_buf, &characterCounter, input);

        /* endregion handle received character */
    }
#pragma clang diagnostic pop
}

/* region HELPER FUNCTIONS */

static void initPico(void) {
    if (watchdog_enable_caused_reboot()) {
        reset_usb_boot(0, 0);
    }

    stdio_init_all();

    // Take a break to make sure everything is ready
    sleep_ms(2500);

    while ((!stdio_usb_connected())) {
        // waits for usb connection
    }

    // enables watchdog timer (15s)
    watchdog_enable(15000, 1);
}

static void initSide(void) {
    // create the dispenser with their address and save them in an array
    for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
        dispenser[i] = dispenserCreate(i, SERIAL_UART);
    }
}

static void initialize_adc(uint8_t gpio) {
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

static void establishConnectionToMaster(void) {
    uint32_t input_identifier;
    volatile bool identified = false;

    while (!identified) {
        input_identifier = getchar_timeout_us(10000000);
        if (input_identifier == 'i') {
            input_identifier = getchar_timeout_us(10000000);
            if (input_identifier == '\n' || input_identifier == 'n') {
                PRINT_COMMAND("RIGHT")
                identified = true;
            }
        } else {
            // Did not receive proper string; await new string.
            PRINT_COMMAND("F")
        }
    }
}

/* region handle message */

static void resetMessageBuffer(char *buffer, size_t bufferSize, size_t *receivedCharacterCount) {
    memset(buffer, '\0', bufferSize);
    *receivedCharacterCount = 0;
}

static bool isAllowedCharacter(uint32_t charToCheck) {
    for (uint32_t i = 0; i < strlen(allowedCharacters); ++i) {
        if (charToCheck == allowedCharacters[i]) {
            return true;
        }
    }
    return false;
}

static bool isLineEnd(uint32_t charToCheck) {
    return charToCheck == 'n' || charToCheck == '\n';
}

static bool isMessageToLong(uint16_t numberOfCharacters) {
    return numberOfCharacters >= INPUT_BUFFER_LEN - 1;
}

static uint32_t parseInputString(char **message) {
    // every halt timing command has to end with a ';'
    // the function will search for this char and save its position
    char *semicolonPosition = strchr(*message, ';');
    if (semicolonPosition == NULL) {
        return 0; // No Semicolon found
    }
    // the string will be cast, from the beginning of the string to the
    // ';'-Position, into an integer
    uint32_t delay = strtol(*message, &semicolonPosition, 10);
    *message = semicolonPosition + 1;
    return delay;
}

static void processMessage(char *message, size_t *messageLength) {
    uint16_t dispensersTrigger = 0;

    PRINT_DEBUG("Process message len: %d", *messageLength)
    for (uint8_t i = 0; i < 4; ++i) {
        uint32_t dispenserHaltTimes = parseInputString(&message);
#ifdef RONDELL
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
#else
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
        sleep_until(time);
        time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
        // Checks for each dispenser if their next state is reached and perform the
        // according action
        for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
            dispenserDoStep(&dispenser[i]);
        }
        // When all dispensers are finished, they are in the state sleep
    } while (!dispenserSetAllToSleepState(dispenser, NUMBER_OF_DISPENSERS));
#endif
}

static void handleMessage(char *buffer, size_t maxBufferSize, size_t *receivedCharacterCount) {
    processMessage(buffer, receivedCharacterCount);
    resetMessageBuffer(buffer, maxBufferSize, receivedCharacterCount);
}

static void storeCharacter(char *buffer, size_t *bufferIndex, char newCharacter) {
    PRINT_DEBUG("Received: %c (counter: %d)", buffer, bufferIndex)
    buffer[*bufferIndex] = newCharacter;
    *bufferIndex = *bufferIndex + 1;
}

/* endregion Handle message */

/* endregion HELPER FUNCTIONS */
