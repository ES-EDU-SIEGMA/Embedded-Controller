#ifndef SIEGMA_DISPENSER_H
#define SIEGMA_DISPENSER_H

#include "limitSwitch.h"
#include "motor.h"
#include "serialUART.h"
#include <stdint.h>

/* region DEFINES */

#ifdef RONDELL
#define NUMBER_OF_DISPENSERS 1
#else
#define NUMBER_OF_DISPENSERS 4
#endif

#define DISPENSER_STEP_TIME_MS 100

#ifdef RONDELL
#define MS_DISPENSERS_ARE_MOVING_UP_0 7500
#elifdef LEFT
#define MS_DISPENSERS_ARE_MOVING_UP_0 8300
#define MS_DISPENSERS_ARE_MOVING_UP_1 8500
#define MS_DISPENSERS_ARE_MOVING_UP_2 7500
#define MS_DISPENSERS_ARE_MOVING_UP_3 7300
#elifdef RIGHT
#define MS_DISPENSERS_ARE_MOVING_UP_0 7700
#define MS_DISPENSERS_ARE_MOVING_UP_1 7500
#define MS_DISPENSERS_ARE_MOVING_UP_2 8200
#define MS_DISPENSERS_ARE_MOVING_UP_3 7900
#endif

#ifdef RONDELL
#define FIND_TIME 750
#else
#define FIND_TIME 250
#endif

#if NUMBER_OF_DISPENSERS > 4
#error ONLY 4 DISPENERS AVAILABLE
#endif

typedef struct dispenser dispenser_t;
typedef struct dispenserState dispenserState_t;

struct dispenserState {
    struct dispenserState (*function)(dispenser_t *);
};

/// Dispenser struct holding all important values to control the stepper driver
struct dispenser {
    SerialAddress_t address;
    uint8_t othersTriggered;
    uint16_t stepsDone;
    uint16_t stepsUp;
    uint16_t haltSteps;
    dispenserState_t state;
    Motor_t motor;
    limitSwitch_t limitSwitch;
    serialUart_t uart;
};

enum {
    DISPENSER_STATE_SLEEP,
    DISPENSER_STATE_UP,
    DISPENSER_STATE_TOP,
    DISPENSER_STATE_DOWN,
    DISPENSER_STATE_ERROR,
    DISPENSER_STATE_INVALID,
};
typedef uint8_t dispenserStateCode_t;

/* endregion DEFINES */

/* region FUNCTION PROTOTYPES */

/*! initialize a new Dispenser with all of its components (Uart, Limit Switch)
 *
 * @param address the address for the UART connection (1, 2, 3, 4)
 * @param uart    which UART pins will be used
 * @return        a initialized Dispenser
 */
dispenser_t dispenserCreate(SerialAddress_t address, serialUart_t uart);

/*! Dispenser cycles to the next state
 *
 * @param dispenser the Dispenser to take action on
 */
void dispenserDoStep(dispenser_t *dispenser);

/*! Check if all Dispenser are sleeping
 *
 * @param dispenser           an array holding a reference to all dispenser
 * @param number_of_dispenser the amount of initialized dispenser
 * @return true if all Dispenser are in a sleeping state
 */
bool dispenserSetAllToSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser);

/*! Set the halt time for the dispenser to wait at the "top"
 *
 * @param dispenser dispenser to be set
 * @param haltTime  the time in ms to halt
 * @return void
 */
void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime);

dispenserStateCode_t dispenserGetStateCode(dispenser_t *dispenser);

/* endregion FUNCTION PROTOTYPES */

#endif // SIEGMA_DISPENSER_H
