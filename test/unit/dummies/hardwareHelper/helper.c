#include <string.h>
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