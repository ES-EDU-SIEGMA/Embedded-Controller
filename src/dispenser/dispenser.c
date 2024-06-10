#define SOURCE_FILE "DISPENSER"

#include "include/dispenser.h"
#include "common.h"
#include "dispenser.h"

#include <pico/time.h>

/* region PUBLIC FUNCTIONS */

void dispenserCreate(dispenser_t *dispenser, motorAddress_t address, uint8_t dispenserCL,
                     bool isRondell) {
    dispenser->address = address;
    dispenser->limitSwitch = createLimitSwitch(address);
    dispenser->isRondell = isRondell;
    dispenser->nextState = (state)sleepState;
    // dispenser->stepsUp = dispenserUpTime(dispenserCL) / DISPENSER_STEP_TIME_MS;
    dispenser->haltTime = 0;
    dispenser->limitTopState = 0;
    dispenser->torqueCounter = 0;
    dispenser->minimum = 0;
    dispenser->counterMinimum = 0;
    dispenser->downCounter = 0;
    dispenser->torque = 0;
    createMotor(dispenser->address);
    resetDispenserPosition(dispenser);
}

dispenserStateCode_t getDispenserState(dispenser_t *dispenser) {
    if (dispenser->nextState == (state)sleepState) {
        return DISPENSER_STATE_SLEEP;
    }
    if (dispenser->nextState == (state)upState) {
        return DISPENSER_STATE_UP;
    }
    if (dispenser->nextState == (state)topState) {
        return DISPENSER_STATE_TOP;
    }
    if (dispenser->nextState == (state)downState) {
        return DISPENSER_STATE_DOWN;
    }
    if (dispenser->nextState == (state)contactState) {
        return DISPENSER_CONTACT_STATE;
    }
    if (dispenser->nextState == (state)errorState) {
        return DISPENSER_STATE_ERROR;
    }
    return DISPENSER_STATE_INVALID;
}

void dispenserEmergencyStop(dispenser_t *dispenser) {
    stopMotor(dispenser->address);
    dispenserSetHaltTime(dispenser, 0);
    dispenser->nextState = (state)sleepState;
}

void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime) {
    dispenser->haltTime = haltTime;
}
void dispenserErrorStateCheck(dispenser_t *dispenser) {
    if (!motorIsCommunicating(dispenser->address)) {
        dispenser->nextState = (state)errorState;
    }
}
void dispenserChangeStates(dispenser_t *dispenser) {
    void (*next)(dispenser_t *dispenser) = (void (*)(dispenser_t *))dispenser->nextState;
    next(dispenser);
}

bool dispenserAllInSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser) {
    for (uint8_t i = 0; i < number_of_dispenser; ++i) {
        if (dispenser[i].nextState != (state)sleepState) {
            return false;
        }
    }
    return true;
}

/* endregion PUBLIC FUNCTIONS */

/* region INTERNAL FUNCTIONS */

static void resetDispenserPosition(dispenser_t *dispenser) {
    PRINT("Initial State");

    int minimum = 1000;
    int counterMinimum = 0;
    int downCounter = 0;
    dispenser->torqueCounter = 0;

    /* region move to limit-switch */
    moveMotorDown(dispenser->address);
    while (!limitSwitchIsClosed(dispenser->limitSwitch)) {}
    stopMotor(dispenser->address);
    /* endregion move to limit-switch */

    //! use torque detection
    if (!dispenser->isRondell) {
        /* region move motor up until contact with dispenser */
        moveMotorUpSlowSpeed(dispenser->address);
        while (dispenser->torqueCounter < 50) {
            dispenser->torque = motorGetTorque(dispenser->address);
            if (dispenser->torque < minimum) {
                minimum = dispenser->torque;
            }
            dispenser->torqueCounter++;
        }
        PRINT("Torque Min: %i", minimum);

        while (counterMinimum < 1) {
            dispenser->torque = motorGetTorque(dispenser->address);
            if (dispenser->torque < (minimum - 10)) {
                counterMinimum++;
            }
        }
        /* endregion move motor up until contact with dispenser */

        /* region reduce pressure to dispenser */
        moveMotorDown(dispenser->address);
        while (downCounter < 100) {
            sleep_ms(1);
            downCounter++;
        }
        stopMotor(dispenser->address);
        /* endregion reduce pressure to dispenser */

        dispenser->torqueCounter = 0;
    }

    PRINT("Dispenser Position detected");
}

/* region STATES */

static void errorState(dispenser_t *dispenser) {
    Motor_t *motor = getMotor(dispenser->address);
    setUpMotor(motor, (int)(dispenser->address));
    if (motorIsCommunicating(dispenser->address)) {
        dispenserSetHaltTime(dispenser, 0);
        dispenser->nextState = (state)sleepState;
    }
    dispenser->nextState = (state)errorState;
}

static void sleepState(dispenser_t *dispenser) {
    PRINT("sleepState");

    if (dispenser->haltTime > 0) {
        moveMotorUp(dispenser->address);
        dispenser->nextState = (state)upState;
    }
    dispenser->nextState = (state)sleepState;
}

static void upState(dispenser_t *dispenser) {
    PRINT("upState");

    if (motorGetTorque(dispenser->address) < 10) {
        dispenser->torqueCounter++;
    } else {
        dispenser->torqueCounter = 0;
    }

    if (dispenser->torqueCounter == 2) {
        PRINT("top position reached");
        stopMotor(dispenser->address);
        dispenser->torqueCounter = 0;
        dispenser->nextState = (state)topState;
    }

    dispenser->nextState = (state)upState;
}

static void topState(dispenser_t *dispenser) {
    PRINT("topState");

    if (dispenser->haltTime > 0 && dispenser->limitTopState == 0) {
        dispenser->limitTopState = time_us_32() + dispenser->haltTime * 1000;
    }

    if (dispenser->limitTopState <= time_us_32()) {
        PRINT("limit reached");
        dispenser->haltTime = 0;
        dispenser->limitTopState = 0;

        moveMotorDown(dispenser->address);
        dispenser->nextState = (state)downState;
    }

    dispenser->nextState = (state)topState;
}

static void downState(dispenser_t *dispenser) {
    PRINT("downState");

    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        PRINT("limitswitch reached");
        stopMotor(dispenser->address);

        if (!dispenser->isRondell) {
            dispenser->nextState = (state)contactState;
        } else {
            dispenser->nextState = (state)sleepState;
        }
    }

    dispenser->nextState = (state)downState;
}

static void contactState(dispenser_t *dispenser) {
    // TODO: check state
    if (dispenser->torqueCounter < 50) {
        dispenser->torque = motorGetTorque(dispenser->address);
        if (dispenser->torque < dispenser->minimum) {
            dispenser->minimum = dispenser->torque;
        }
        dispenser->torqueCounter++;
    } else {
        if (dispenser->counterMinimum < 1) {
            PRINT("Torque Min: %lu", dispenser->minimum);
            dispenser->torque = motorGetTorque(dispenser->address);
            if (dispenser->torque < (dispenser->minimum - 10)) {
                // dispenser->minimum = torque;
                dispenser->counterMinimum++;
            }
        } else {
            moveMotorDown(dispenser->address);
            if (dispenser->downCounter < 10) {
                dispenser->downCounter++;
            } else {
                stopMotor(dispenser->address);
                PRINT("Dispenser Position detected");
                dispenser->torqueCounter = 0;
                dispenserSetHaltTime(dispenser, 0);
                dispenser->nextState = (state)sleepState;
            }
        }
    }

    dispenser->nextState = (state)contactState;
}

/* endregion STATES */

/* endregion INTERNAL FUNCTIONS */
