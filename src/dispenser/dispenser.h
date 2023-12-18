#ifndef SIEGMA_DISPENSER_H
#define SIEGMA_DISPENSER_H

#include "limitSwitch.h"
#include "motor.h"
#include <stdint.h>

/* region DEFINES */

#define DISPENSER_STEP_TIME_MS 100

enum {
    DISPENSER_STATE_SLEEP,
    DISPENSER_STATE_UP,
    DISPENSER_STATE_TOP,
    DISPENSER_STATE_DOWN,
    DISPENSER_STATE_ERROR,
    DISPENSER_STATE_INVALID,
};

typedef struct dispenserState dispenserState_t;
typedef struct dispenser dispenser_t;
typedef uint8_t dispenserStateCode_t;

struct dispenserState {
    struct dispenserState (*function)(dispenser_t *);
};

/// Dispenser struct holding all important values to control the stepper driver
struct dispenser {
    motorAddress_t address;
    uint8_t othersTriggered; //! How many Dispenser have halt-times > 0
    uint16_t stepsDone;
    uint16_t stepsUp;
    uint16_t haltSteps;
    dispenserState_t state;
    limitSwitch_t limitSwitch;
    uint16_t searchTimeout;//never accessed
};

/* endregion DEFINES */

/* region FUNCTION PROTOTYPES */

/*! initialize a new Dispenser with all of its components (Uart, Limit Switch)
 *
 * @param address the address for the UART connection (1, 2, 3, 4)
 * @param uart    which UART pins will be used
 * @return        a initialized Dispenser
 */
/*void dispenserCreate(dispenser_t *dispenser, SerialAddress_t address, serialUart_t uart,
                     uint8_t dispenserCL, uint16_t searchTimeout);*/

void dispenserCreate(dispenser_t *dispenser, motorAddress_t address, uint8_t dispenserCL, uint16_t searchTimeout);
/*! Set the halt time for the dispenser to wait at the "top"
 *
 * @param dispenser dispenser to be set
 * @param haltTime  the time in ms to halt
 * @return void
 */
void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime);

dispenserStateCode_t getDispenserState(dispenser_t *dispenser);

/*! Halt the dispenser motor immedeatly
 *
 * @param dispenser dispenser to stop
 */
void dispenserEmergencyStop(dispenser_t *dispenser);
/*! Check if all Dispenser are sleeping
 *
 * @param dispenser           an array holding a reference to all dispenser
 * @param number_of_dispenser the amount of initialized dispenser
 * @return true if all Dispenser are in a sleeping state
 */
bool dispenserAllInSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser);

/*! Enable the motor
 *
 * @param motor motor to be enabled
 */

/*! Dispenser cycles to the next state
 *
 * @param dispenser the Dispenser to take action on
 */
void dispenserChangeStates(dispenser_t *dispenser);
/*! Change dispenser state to the sleep state (wait for new command)
 *
 * @param dispenser dispenser to be set
 * @return          new state (sleep state) of the dispenser
 */
static dispenserState_t sleepState(dispenser_t *dispenser);

/*! Change dispenser state to the up state (drive upwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (up state) of the dispenser
 */
static dispenserState_t upState(dispenser_t *dispenser);

/*! Change dispenser state to the top state (stay in the up position)
 *
 * @param dispenser dispenser to be set
 * @return          new state (top state) of the dispenser
 */
static dispenserState_t topState(dispenser_t *dispenser);

/*! Change dispenser state to the down state (drive downwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (down state) of the dispenser
 */
static dispenserState_t downState(dispenser_t *dispenser);

/*! Change dispenser state to the error state (no connection to the tmc -> try again)
 *
 * @param dispenser dispenser to be set
 * @return          new state (error state) of the dispenser
 */
static dispenserState_t errorState(dispenser_t *dispenser);
/* endregion FUNCTION PROTOTYPES */

#endif // SIEGMA_DISPENSER_H
