// Project
#include "common.h"
#include "dispenser.h"
#include "rondell.h"

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
Dispenser_t dispenser[NUMBER_OF_DISPENSERS];       /// Array containing the dispenser

/* endregion VARIABLES/DEFINES */

/* region HELPER FUCTIONS */

/*! initialize the usb connection to the pico
 *
 * @param waitForUSBConnection Bool if the pico should wait for an usb connection
 */
void initPico(bool waitForUSBConnection) {
    if (watchdog_enable_caused_reboot())
        reset_usb_boot(0, 0);

    stdio_init_all(); // init usb
    sleep_ms(2500);   // Time to make sure everything is ready

    if (waitForUSBConnection)
        while ((!stdio_usb_connected()))
            ; // waits for usb connection
}

bool isAllowedCharacter(uint32_t charToCheck) {
    for (uint32_t i = 0; i < strlen(allowedCharacters); ++i) {
        if (charToCheck == allowedCharacters[i]) {
            return true;
        }
    }
    return false;
}

bool isLineEnd(uint32_t charToCheck) {
    return charToCheck == 'n' || charToCheck == '\n';
}

bool isMessageToLong(uint16_t numberOfCharacters) {
    return numberOfCharacters >= INPUT_BUFFER_LEN - 1;
}

/*! parse a String to fetch the hopper halt timings
 *
 * @param message The received message containing the halt timing
 * @return        The parsed halt timing as an integer or on failure, a Zero
 */
uint32_t parseInputString(char **message) {
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

/*! process the received Message (received over Serial)
 *
 * @param message char buffer containing the received Message
 */
void processMessage(char *message) {
    uint16_t dispensersTrigger = 0;

    for (uint8_t i = 0; i < 4; ++i) {
        uint32_t dispenserHaltTimes = parseInputString(&message);
#ifdef RONDELL
        setDispenserHaltTime(&dispenser[0], dispenserHaltTimes);
        if (dispenserHaltTimes > 0) {
            moveToDispenserWithId(i);
            absolute_time_t time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
            do {
                sleep_until(time);
                time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
                dispenserDoStep(&dispenser[0]);
            } while (!allDispenserInSleepState(dispenser, 1));
        }
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
    } while (!allDispenserInSleepState(dispenser, NUMBER_OF_DISPENSERS));
#else
        if (dispenserHaltTimes > 0) {
            dispensersTrigger++;
        }
        setDispenserHaltTime(&dispenser[i], dispenserHaltTimes);
    }
#endif
}

/*! This function's purpose is to establish synchronization between the pico
 * and the pi. The pi sends `i\\n` to the pico corresponding and to confirm
 * that the string has been received the pico sends back `[POSITION]\\n`.
 * This function imprisons the program flow in the while loop until
 * synchronization has been established.
 */
void establishConnectionToMaster(void) {
    uint32_t input_identifier;
    volatile bool identified = false;

    while (!(identified)) {
        input_identifier = getchar_timeout_us(10000000); // 10 seconds wait
        if (input_identifier == 'i') {
            input_identifier = getchar_timeout_us(10000000);
            if (input_identifier == '\n' || input_identifier == 'n') {
                identified = true;
#ifdef RONDELL
                PRINT_DEBUG("RONDELL")
#endif
#ifdef LEFT
                PRINT_DEBUG("LEFT")
#endif
#ifdef RIGHT
                PRINT_DEBUG("RIGHT")
#endif
            }
        } else {
            // Did not receive proper string; await new string.
            input_identifier = 0;
            printf("F\n");
        }
    }
}

/*! initializes the ADC of the MCU for usage with the light sensor
 *
 * @param gpio  GPIO of the Tiny2040 for the ADC input
 */
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
        adcInputPin = 0xFF;
    }

    adc_init();
    adc_gpio_init(gpio);
    adc_select_input(adcInputPin);
}

/* endregion HELPER FUNCTIONS */

int main() {
    initPico(false);
    establishConnectionToMaster();

#ifdef RONDELL
    initialize_adc(28);
    dispenser[0] = createDispenser(0, SERIAL2);
    setUpRondell(2, SERIAL2);
#else
    // create the dispenser with their address and save them in an array
    for (uint8_t i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
        dispenser[i] = createDispenser(i, SERIAL_UART);
    }
#endif

    PRINT("CALIBRATED")
    // Buffer for received Messages
    char *input_buf = malloc(INPUT_BUFFER_LEN);
    memset(input_buf, '\0', INPUT_BUFFER_LEN);
    uint16_t characterCounter = 0;

    // Waits for an input in the form ([0..9];)+[\n|n], each number standing for the wait time of
    // the corresponding dispenser
    while (true) {

        uint32_t input = getchar_timeout_us(10000000);

        if (input == PICO_ERROR_TIMEOUT) {
            continue;
        }

        if (!isAllowedCharacter(input)) {
            // ignore the received character if it is not an allowed one
            PRINT_DEBUG("Received '%c' which is not allowed", input)
            continue;
        }

        if (isLineEnd(input)) {
            // received end character, message should be complete, start with processing
            PRINT_DEBUG("Process message len: %d", characterCounter)
            processMessage(input_buf);
            memset(input_buf, '\0', INPUT_BUFFER_LEN);
            characterCounter = 0;
            printf("READY\n");
        } else if (isMessageToLong(characterCounter)) {
            // received to many characters -> flushing the uart connection and start over
            PRINT_DEBUG("Input too long, flushing...")
            memset(input_buf, '\0', INPUT_BUFFER_LEN);
            characterCounter = 0;
        } else {
            // character is allowed and we did not reach the end
            PRINT_DEBUG("Received: %c (counter: %d)", input, characterCounter)
            input_buf[characterCounter] = input;
            ++characterCounter;
        }
    }
}
