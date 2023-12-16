#ifndef SIEGMA_DISPENSER_H
#define SIEGMA_DISPENSER_H

#include "limitSwitch.h"
#include "motor.h"
#include "tmc2209.h"
#include "serialUART.h"
#include <stdint.h>

/* region DEFINES */

#define TOP_TIME_SLOT 1

typedef struct dispenserState dispenserState_t;
typedef struct dispenser dispenser_t;
typedef uint8_t dispenserStateCode_t;

enum {
    DISPENSER_STATE_SLEEP,
    DISPENSER_STATE_UP,
    DISPENSER_STATE_TOP,
    DISPENSER_STATE_DOWN,
    DISPENSER_STATE_ERROR,
    DISPENSER_STATE_INVALID,
};

struct dispenserState {
    struct dispenserState (*function)(dispenser_t *);
};
/// Dispenser struct holding all important values to control the stepper driver
struct dispenser {
    SerialAddress_t address;
    uint16_t stepsUp;
    dispenserState_t state;
    Motor_t motor;
    limitSwitch_t limitSwitch;
    serialUart_t uart;
    uint32_t haltTime;
};

/* endregion DEFINES */

/* region FUNCTION PROTOTYPES */

/*! initialize a new Dispenser with all of its components (Uart, Limit Switch)
 *
 * @param address the address for the UART connection (1, 2, 3, 4)
 * @param uart    which UART pins will be used
 * @return        a initialized Dispenser
 */
void dispenserCreate(dispenser_t *dispenser, SerialAddress_t address, serialUart_t uart,
                     uint8_t dispenserCL);

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

/*! Halt the dispenser motor immedeatly
 *
 * @param dispenser dispenser to stop
 */
void dispenserEmergencyStop(dispenser_t *dispenser);

/* endregion FUNCTION PROTOTYPES */

#endif // SIEGMA_DISPENSER_H
