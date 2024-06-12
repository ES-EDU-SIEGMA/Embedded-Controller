#ifndef SIEGMA_COMMON_HEADER
#define SIEGMA_COMMON_HEADER

#include <stdio.h>

#ifndef SOURCE_FILE
#define SOURCE_FILE "NOT DEFINED"
#endif

#ifdef DEBUG
#define DEBUG_MODE true
#else
#define DEBUG_MODE false
#endif

/*!
 * @brief macro to simplify printing commands for the communication with the RPi
 * @param str command to print
 * @param ... replacements for str
 *
 * @IMPORTANT based on printf(...)
 */
#define PRINT_COMMAND(str, ...)                                                                    \
    do {                                                                                           \
        printf(str, ##__VA_ARGS__);                                                                \
        printf("\n");                                                                              \
    } while (false)

/*!
 * @brief macro to simplify printing output for debug purposes
 * @param str information to print
 * @param ... replacements for str
 *
 * @IMPORTANT based on printf(...)
 *
 * @CAUTION only visible if DEBUG mode is disabled
 */
#define PRINT(str, ...)                                                                            \
    do {                                                                                           \
        if (DEBUG_MODE) {                                                                          \
            printf("[%s: %s] ", SOURCE_FILE, __FUNCTION__);                                        \
            printf(str, ##__VA_ARGS__);                                                            \
            printf("\n");                                                                          \
        }                                                                                          \
    } while (false)

#endif /* SIEGMA_COMMON_HEADER */
