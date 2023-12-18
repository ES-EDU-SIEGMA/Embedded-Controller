#include <string.h>
#include <stdlib.h>
#include "helper.h"

const char *allowedCharacters = "0123456789i;\n";

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

void initializeMessageHandler(char **buffer, size_t bufferLength, size_t *characterCounter) {
    *buffer = malloc(bufferLength);
    resetMessageBuffer(*buffer, bufferLength, characterCounter);
}

void resetMessageBuffer(char *buffer, size_t bufferSize, size_t *receivedCharacterCount) {
    memset(buffer, '\0', bufferSize);
    *receivedCharacterCount = 0;
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