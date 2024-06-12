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
    dispenser->timeToTopPosition = 0;
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
void dispenserExecuteNextState(dispenser_t *dispenser) {
    void (*next)(dispenser_t *dispenser) = (void (*)(dispenser_t *))dispenser->nextState;
    next(dispenser);
}

bool dispenserInSleepState(dispenser_t *dispenser) {
    return getDispenserState(dispenser) == DISPENSER_STATE_SLEEP;
}
bool dispenserAllInSleepState(dispenser_t *dispenser, size_t numberOfDispenser) {
    for (uint8_t index = 0; index < numberOfDispenser; index++) {
        if (!dispenserInSleepState(&dispenser[index])) {
            return false;
        }
    }
    return true;
}

/* endregion PUBLIC FUNCTIONS */

/* region INTERNAL FUNCTIONS */

static void resetDispenserPosition(dispenser_t *dispenser) {
    PRINT("Initial State");

    dispenser->minimum = 1000;
    dispenser->counterMinimum = 0;
    dispenser->downCounter = 0;
    dispenser->torqueCounter = 0;

    /* region move to limit-switch */
    moveMotorDown(dispenser->address);
    while (!limitSwitchIsClosed(dispenser->limitSwitch)) {}
    stopMotor(dispenser->address);
    /* endregion move to limit-switch */

    if (dispenser->isRondell) {
        PRINT("Dispenser Position detected");
        return;
    }

    //! use torque detection
    /* region move motor up until contact with dispenser */
    moveMotorUpSlowSpeed(dispenser->address);
    while (dispenser->torqueCounter < 50) {
        dispenser->torque = motorGetTorque(dispenser->address);
        if (dispenser->torque < dispenser->minimum) {
            dispenser->minimum = dispenser->torque;
        }
        dispenser->torqueCounter += 1;
    }
    PRINT("Torque Min: %lu", dispenser->minimum);

    while (true) {
        dispenser->torque = motorGetTorque(dispenser->address);
        if (dispenser->torque < (dispenser->minimum - 10)) {
            break;
        }
    }
    /* endregion move motor up until contact with dispenser */

    /* region reduce pressure to dispenser */
    moveMotorDown(dispenser->address);
    sleep_ms(100);
    stopMotor(dispenser->address);
    /* endregion reduce pressure to dispenser */

    dispenser->torqueCounter = 0;

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
    PRINT("ht: %lu", dispenser->haltTime);

    if (dispenser->haltTime > 0) {
        if (dispenser->isRondell) {
            dispenser->timeToTopPosition = time_us_32() + DISPENSER_MS_TO_TOP_POSITION * 1000;
        }
        moveMotorUp(dispenser->address);
        dispenser->nextState = (state)upState;
        return;
    }
    dispenser->nextState = (state)sleepState;
}

static void upState(dispenser_t *dispenser) {
    PRINT("upState");

    if (dispenser->isRondell) {
        if (dispenser->timeToTopPosition <= time_us_32()) {
            stopMotor(dispenser->address);
            dispenser->timeToTopPosition = 0;
            dispenser->nextState = (state)topState;
            return;
        }
        dispenser->nextState = (state)upState;
        return;
    }

    if (motorGetTorque(dispenser->address) < 5) {
        dispenser->torqueCounter++;
    } else {
        dispenser->torqueCounter = 0;
    }

    if (dispenser->torqueCounter == 8) {
        PRINT("top position reached");
        stopMotor(dispenser->address);
        dispenser->torqueCounter = 0;
        dispenser->nextState = (state)topState;
        return;
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
        return;
    }

    dispenser->nextState = (state)topState;
}

static void downState(dispenser_t *dispenser) {
    PRINT("downState");

    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        PRINT("limitswitch reached");
        stopMotor(dispenser->address);

        if (dispenser->isRondell) {
            dispenser->nextState = (state)sleepState;
            return;
        } else {
            dispenser->minimum = 1000;
            dispenser->torqueCounter = 0;
            dispenser->counterMinimum = 0;
            dispenser->downCounter = 0;
            moveMotorUpSlowSpeed(dispenser->address);
            dispenser->nextState = (state)contactState;
            return;
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
        PRINT("Torque: %u", dispenser->torque);
        dispenser->torqueCounter += 1;
        dispenser->nextState = (state)contactState;
        return;
    }

    if (dispenser->counterMinimum < 1) {
        PRINT("Torque Min: %lu", dispenser->minimum);
        dispenser->torque = motorGetTorque(dispenser->address);
        if (dispenser->torque < (dispenser->minimum - 10)) {
            dispenser->counterMinimum += 1;
        }
        dispenser->nextState = (state)contactState;
        return;
    }

    moveMotorDown(dispenser->address);
    sleep_ms(100);
    dispenser->nextState = (state)contactState;
    stopMotor(dispenser->address);

    PRINT("Dispenser Position detected");

    dispenser->torqueCounter = 0;
    dispenserSetHaltTime(dispenser, 0);
    dispenser->nextState = (state)sleepState;
}

/* endregion STATES */

/* endregion INTERNAL FUNCTIONS */
