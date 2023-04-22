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

typedef struct Dispenser Dispenser_t;

typedef struct dispenserState {
    struct dispenserState (*function)(struct Dispenser *);
} DispenserState_t;

/// Dispenser struct holding all important values to control the stepper driver
typedef struct Dispenser {
    SerialAddress_t address;
    uint8_t othersTriggered;
    uint16_t stepsDone;
    uint16_t stepsUp;
    uint16_t haltSteps;
    DispenserState_t state;
    Motor_t motor;
    limitSwitch_t limitSwitch;
    serialUart_t uart;
} Dispenser_t;

/* endregion DEFINES */

/* region FUNCTION PROTOTYPES */

/*! initialize a new Dispenser with all of its components (Uart, Limit Switch)
 *
 * @param address the address for the UART connection (1, 2, 3, 4)
 * @param uart    which UART pins will be used
 * @return        a initialized Dispenser
 */
Dispenser_t createDispenser(SerialAddress_t address, serialUart_t uart);

/*! Dispenser cycles to the next state
 *
 * @param dispenser the Dispenser to take action on
 */
void dispenserDoStep(Dispenser_t *dispenser);

/*! Check if all Dispenser are sleeping
 *
 * @param dispenser           an array holding a reference to all dispenser
 * @param number_of_dispenser the amount of initialized dispenser
 * @return true if all Dispenser are in a sleeping state
 */
bool allDispenserInSleepState(Dispenser_t *dispenser, uint8_t number_of_dispenser);

/*! Set the halt time for the dispenser to wait at the "top"
 *
 * @param dispenser dispenser to be set
 * @param haltTime  the time in ms to halt
 * @return void
 */
void setDispenserHaltTime(Dispenser_t *dispenser, uint32_t haltTime);

/*! Change dispenser state to the sleep state (wait for new command)
 *
 * @param dispenser dispenser to be set
 * @return          new state (sleep state) of the dispenser
 */
static DispenserState_t sleepState(Dispenser_t *dispenser);

/*! Change dispenser state to the up state (drive upwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (up state) of the dispenser
 */
static DispenserState_t upState(Dispenser_t *dispenser);

/*! Change dispenser state to the top state (stay in the up position)
 *
 * @param dispenser dispenser to be set
 * @return          new state (top state) of the dispenser
 */
static DispenserState_t topState(Dispenser_t *dispenser);

/*! Change dispenser state to the down state (drive downwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (down state) of the dispenser
 */
static DispenserState_t downState(Dispenser_t *dispenser);

/*! Change dispenser state to the error state (no connection to the tmc -> try again)
 *
 * @param dispenser dispenser to be set
 * @return          new state (error state) of the dispenser
 */
static DispenserState_t errorState(Dispenser_t *dispenser);

/* endregion FUNCTION PROTOTYPES */

#endif // SIEGMA_DISPENSER_H
