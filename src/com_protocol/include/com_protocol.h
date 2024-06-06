#ifndef SIEGMA_COM_PROTOCOL_HEADER
#define SIEGMA_COM_PROTOCOL_HEADER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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

#endif /* SIEGMA_COM_PROTOCOL_HEADER */
