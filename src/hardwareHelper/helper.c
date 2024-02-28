#define SOURCE_FILE "HELPER"

#include "helper.h"
#include "common.h"
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdlib.h>
#include <stdlib.h>
#include <string.h>

/* region HARDWARE */

void initHardware(bool waitForConnection) {
#ifdef DEBUG
    if (watchdog_enable_caused_reboot()) {
        reset_usb_boot(0, 0);
    }
#endif

    stdio_init_all();

    // Take a break to make sure everything is ready
    sleep_ms(2500);

    if (waitForConnection) {
        while ((!stdio_usb_connected())) {
            // waits for usb connection
        }
    }
    PRINT_DEBUG("Hardware Initialized!")
}

__force_inline void setUpWatchdog(int timeoutInSeconds) {
    if (timeoutInSeconds > 0) {
        watchdog_enable(timeoutInSeconds * 1000, true);
        PRINT_DEBUG("Watchdog enabled with %i seconds", timeoutInSeconds)
    }
}

__force_inline void resetWatchdogTimer(void) {
    watchdog_update();
    PRINT_DEBUG("Watchdog timer performed restart!")
}

/* endregion HARDWARE */

/* region MESSAGE */

const char *allowedCharacters = "0123456789i;\n"; /// sequence of allowed character

void establishConnectionWithController(char *identifier) {
    char receivedCharacter;
    bool identified = false;

    while (!identified) {
        receivedCharacter = getchar_timeout_us(10000000);
        if (receivedCharacter == 'i') {
            receivedCharacter = getchar_timeout_us(10000000);
            if (receivedCharacter == '\n') {
                PRINT_COMMAND("%s", identifier)
                identified = true;
            }
        } else {
            // Did not receive proper string; await new string.
            PRINT_COMMAND("F")
        }
    }
}

void initAndConnect(bool waitForConnection, char *identifier) {
    initHardware(waitForConnection);
    establishConnectionWithController(identifier);
}

void initializeMessageHandler(char **buffer, size_t bufferLength, size_t *characterCounter) {
    *buffer = malloc(bufferLength);
    resetMessageBuffer(*buffer, bufferLength, characterCounter);
}

void resetMessageBuffer(char *buffer, size_t bufferSize, size_t *receivedCharacterCount) {
    memset(buffer, '\0', bufferSize);
    *receivedCharacterCount = 0;
}

bool isAllowedCharacter(char charToCheck) {
    for (uint32_t i = 0; i < strlen(allowedCharacters); ++i) {
        if (charToCheck == allowedCharacters[i]) {
            return true;
        }
    }
    return false;
}

bool isLineEnd(char charToCheck) {
    return charToCheck == '\n';
}

bool isMessageToLong(size_t numberOfCharacters, size_t bufferLength) {
    return numberOfCharacters >= bufferLength - 1;
}

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

void handleMessage(char *buffer, size_t maxBufferSize, size_t *receivedCharacterCount) {
    processMessage(buffer, *receivedCharacterCount);
    resetMessageBuffer(buffer, maxBufferSize, receivedCharacterCount);
}

void storeCharacter(char *buffer, size_t *bufferIndex, char newCharacter) {
    PRINT_DEBUG("Received: %c (counter: %u)", newCharacter, *bufferIndex)
    buffer[*bufferIndex] = newCharacter;
    *bufferIndex = *bufferIndex + 1;
}

/* endregion */