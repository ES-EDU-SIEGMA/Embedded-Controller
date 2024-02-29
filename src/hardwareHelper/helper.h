#ifndef SIEGMA_HELPER_H
#define SIEGMA_HELPER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* region HARDWARE */

/*! initialize the hardware
 *
 * @param waitForConnection defines if the pico waits for a USB connection
 */
void initHardware(bool waitForConnection);

void setUpWatchdog(int timeoutInSeconds);

void resetWatchdogTimer();

/*! initializes the dispenser
 *
 * \IMPORTANT needs to implemented!
 */
void initDispenser(void);

/* endregion HARDWARE */

/* region MESSAGE */

/*! This function's purpose is to establish synchronization between the pico
 * and the pi. The pi sends `i\\n` to the pico corresponding and to confirm
 * that the string has been received the pico sends back `[POSITION]\\n`.
 * This function imprisons the program flow in the while loop until
 * synchronization has been established.
 */
void establishConnectionWithController(char *identifier);

void initializeMessageHandler(char **buffer, size_t bufferLength, size_t *characterCounter);

void resetMessageBuffer(char *buffer, size_t bufferSize, size_t *receivedCharacterCount);

bool isAllowedCharacter(char charToCheck);

bool isLineEnd(char charToCheck);

bool isMessageToLong(size_t numberOfCharacters, size_t bufferLength);

void storeCharacter(char *buffer, size_t *bufferIndex, char newCharacter);

uint32_t parseInputString(char **message);

void handleMessage(char *buffer, size_t maxBufferSize, size_t *receivedCharacterCount);

/*! function thats processes the received message
 *
 * \IMPORTANT needs to be implemented
 *
 * @param message       message to be processed
 * @param messageLength length of the message
 */
void processMessage(char *message, size_t messageLength);
/* endregion MESSAGE */

#endif // SIEGMA_HELPER_H
