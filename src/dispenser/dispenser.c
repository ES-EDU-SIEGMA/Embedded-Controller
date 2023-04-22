#include "dispenser.h"
#include "dispenser_internal.h"
#include <pico/time.h>
#include <stdio.h>

static dispenserState_t sleepState_t = (dispenserState_t){.function = &sleepState};
static dispenserState_t upState_t = (dispenserState_t){.function = &upState};
static dispenserState_t topState_t = (dispenserState_t){.function = &topState};
static dispenserState_t downState_t = (dispenserState_t){.function = &downState};
static dispenserState_t errorState_t = (dispenserState_t){.function = &errorState};

/* region HEADER FUNCTIONS */
dispenser_t dispenserCreate(SerialAddress_t address, serialUart_t uart) {
    dispenser_t dispenser;
    dispenser.address = address;
    dispenser.uart = uart;
    dispenser.haltSteps = 0;
    dispenser.stepsDone = 0;
    dispenser.state = (dispenserState_t){.function = &sleepState};
    dispenser.motor = createMotor(address, uart);
    dispenser.limitSwitch = createLimitSwitch(address);
    dispenser.othersTriggered = 0;

    switch (address) {
    case SERIAL_ADDRESS_0:
        dispenser.stepsUp = MS_DISPENSERS_ARE_MOVING_UP_0 / DISPENSER_STEP_TIME_MS;
        break;
#ifndef RONDELL
    case SERIAL_ADDRESS_1:
        dispenser.stepsUp = MS_DISPENSERS_ARE_MOVING_UP_1 / DISPENSER_STEP_TIME_MS;
        break;
    case SERIAL_ADDRESS_2:
        dispenser.stepsUp = MS_DISPENSERS_ARE_MOVING_UP_2 / DISPENSER_STEP_TIME_MS;
        break;
    case SERIAL_ADDRESS_3:
        dispenser.stepsUp = MS_DISPENSERS_ARE_MOVING_UP_3 / DISPENSER_STEP_TIME_MS;
        break;
#endif
    }

    findDirection(&dispenser, 250);

    // Reset Dispenser position
    resetDispenserPosition(&dispenser);

    disableMotorByPin(&dispenser.motor);

    return dispenser;
}

dispenserStateCode_t dispenserGetStateCode(dispenser_t *dispenser) {
    if (dispenser->state.function == sleepState) {
        return DISPENSER_STATE_SLEEP;
    } else if (dispenser->state.function == upState) {
        return DISPENSER_STATE_UP;
    } else if (dispenser->state.function == topState) {
        return DISPENSER_STATE_TOP;
    } else if (dispenser->state.function == downState) {
        return DISPENSER_STATE_DOWN;
    } else if (dispenser->state.function == errorState) {
        return DISPENSER_STATE_ERROR;
    } else {
        return DISPENSER_STATE_INVALID;
    }
}

/* endregion HEADER FUNCTIONS */

/* region STATIC FUNCTIONS */

static void resetDispenserPosition(dispenser_t *dispenser) {
    moveMotorUp(&dispenser->motor);
    while (limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    moveMotorDown(&dispenser->motor);
    while (!limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    stopMotor(&dispenser->motor);
}

static void findDirection(dispenser_t *dispenser, uint32_t time) {
    time = time + FIND_TIME;
    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        moveMotorUp(&dispenser->motor);
        sleep_ms(time);
        if (!limitSwitchIsClosed(dispenser->limitSwitch)) {
            stopMotor(&dispenser->motor);
            return;
        } else {
            moveMotorDown(&dispenser->motor);
            sleep_ms(time + FIND_TIME);
            if (!limitSwitchIsClosed(dispenser->limitSwitch)) {
                stopMotor(&dispenser->motor);
                dispenser->motor.direction = DIRECTION_DOWN;
                return;
            } else
                findDirection(dispenser, time + FIND_TIME);
        }
    } else {
        moveMotorDown(&dispenser->motor);
        sleep_ms(time);
        if (limitSwitchIsClosed(dispenser->limitSwitch)) {
            stopMotor(&dispenser->motor);
            dispenser->motor.direction = DIRECTION_UP;
            return;
        } else {
            moveMotorUp(&dispenser->motor);
            sleep_ms(time + FIND_TIME);
            if (limitSwitchIsClosed(dispenser->limitSwitch)) {
                stopMotor(&dispenser->motor);
                return;
            } else
                findDirection(dispenser, time + FIND_TIME);
        }
    }
}

static dispenserState_t errorState(dispenser_t *dispenser) {
    setUpMotor(&dispenser->motor, dispenser->address, dispenser->uart);
    if (motorIsCommunicating(&dispenser->motor)) {
        disableMotorByPin(&dispenser->motor);
        dispenser->haltSteps = 0;
        return sleepState_t;
    }
    return errorState_t;
}

static dispenserState_t sleepState(dispenser_t *dispenser) {
    if (dispenser->haltSteps > 0) {
        enableMotorByPin(&dispenser->motor);
        moveMotorUp(&dispenser->motor);
        return upState_t;
    }
    return sleepState_t;
}

static dispenserState_t upState(dispenser_t *dispenser) {
    printf("upState\n");
    printf("%i\n", dispenser->stepsDone);
    printf("%i\n", dispenser->stepsUp + 2 * dispenser->othersTriggered);
    if (dispenser->stepsDone > dispenser->stepsUp + 2 * dispenser->othersTriggered) {
        stopMotor(&dispenser->motor);
        return topState_t;
    }
    if (!limitSwitchIsClosed(dispenser->limitSwitch))
        dispenser->stepsDone++;
    return upState_t;
}

static dispenserState_t topState(dispenser_t *dispenser) {
    printf("topState\n");
    if (dispenser->stepsDone >
        dispenser->stepsUp + 2 * dispenser->othersTriggered + dispenser->haltSteps) {
        moveMotorDown(&dispenser->motor);
        return downState_t;
    }
    dispenser->stepsDone++;
    return topState_t;
}

static dispenserState_t downState(dispenser_t *dispenser) {
    printf("downState\n");
    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        stopMotor(&dispenser->motor);
        disableMotorByPin(&dispenser->motor);
        dispenser->haltSteps = 0;
        return sleepState_t;
    }
    if (dispenser->stepsDone >
        2 * dispenser->stepsUp + 2 * dispenser->othersTriggered + dispenser->haltSteps + 10) {
        stopMotor(&dispenser->motor);
        disableMotorByPin(&dispenser->motor);
        dispenser->haltSteps = 0;
        return sleepState_t;
    }
    dispenser->stepsDone++;
    return downState_t;
}

void dispenserDoStep(dispenser_t *dispenser) {
    if (!motorIsCommunicating(&dispenser->motor)) {
        dispenser->state = errorState_t;
    }
    dispenser->state = dispenser->state.function(dispenser);
}

void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime) {
    dispenser->haltSteps = haltTime / DISPENSER_STEP_TIME_MS;
    dispenser->stepsDone = 0;
#ifdef DEBUG
    printf("Dispenser %i will stop %hu steps\n", dispenser->address, dispenser->haltSteps);
#endif
}

bool dispenserSetAllToSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser) {
    for (uint8_t i = 0; i < number_of_dispenser; ++i) {
        if (dispenser[i].state.function != sleepState_t.function)
            return false;
    }
    return true;
}

/* endregion STATIC FUNCTIONS */
