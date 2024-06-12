#ifndef SIEGMA_DISPENSER_H
#define SIEGMA_DISPENSER_H

#include "limitSwitch.h"
#include "motor.h"
#include <stdint.h>

/* region DEFINES */

#define DISPENSER_STEP_TIME_MS 100
#define DISPENSER_MS_TO_TOP_POSITION 10000

typedef enum dispenserStateCode {
    DISPENSER_STATE_SLEEP,
    DISPENSER_STATE_UP,
    DISPENSER_STATE_TOP,
    DISPENSER_STATE_DOWN,
    DISPENSER_CONTACT_STATE,
    DISPENSER_STATE_ERROR,
    DISPENSER_STATE_INVALID,
} dispenserStateCode_t;

typedef void (*state)(void *);

typedef struct dispenser {
    motorAddress_t address;
    limitSwitch_t limitSwitch;
    bool isRondell;
    state nextState;
    uint32_t timeToTopPosition;
    uint32_t haltTime; //!< milliseconds to stay in top state
    uint32_t limitTopState;
    uint8_t torqueCounter;
    uint32_t minimum;
    uint8_t counterMinimum;
    uint16_t downCounter;
    uint16_t torque;
} dispenser_t;

/* endregion DEFINES */

/*!
 * @param dispenser[inout] dispenser to initialize
 * @param address[in] motor used for the dispenser
 * @param dispenserCL[in] size of the applied dispenser
 * @param isRondell[in] true if the dispenser is used for an rondell
 */
void dispenserCreate(dispenser_t *dispenser, motorAddress_t address, uint8_t dispenserCL,
                     bool isRondell);

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

/*! Check if Dispenser is sleeping
 *
 * @param dispenser           an array holding a reference to all dispenser
 * @return true if Dispenser is in a sleeping state
 */
bool dispenserInSleepState(dispenser_t *dispenser);
/*! Check if all Dispenser are sleeping
 *
 * @param dispenser           an array holding a reference to all dispenser
 * @param numberOfDispenser the amount of initialized dispenser
 * @return true if all Dispenser are in a sleeping state
 */
bool dispenserAllInSleepState(dispenser_t *dispenser, size_t numberOfDispenser);

/*! Enable the motor
 *
 * @param motor motor to be enabled
 */

/*! Dispenser cycles to the next state
 *
 * @param dispenser the Dispenser to take action on
 */
void dispenserExecuteNextState(dispenser_t *dispenser);

void dispenserErrorStateCheck(dispenser_t *dispenser);

#endif // SIEGMA_DISPENSER_H
