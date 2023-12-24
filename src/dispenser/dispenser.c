#define SOURCE_FILE "DISPENSER"

#include "dispenser.h"
#include "common.h"
#include "dispenser_internal.h"
#include <pico/time.h>

static uint8_t counterTorque = 0;
static uint16_t torque = 0;

static dispenserState_t sleepState_t = (dispenserState_t){.function = &sleepState};
static dispenserState_t upState_t = (dispenserState_t){.function = &upState};
static dispenserState_t topState_t = (dispenserState_t){.function = &topState};
static dispenserState_t downState_t = (dispenserState_t){.function = &downState};
static dispenserState_t errorState_t = (dispenserState_t){.function = &errorState};
// Test
/* region HEADER FUNCTIONS */

void dispenserCreate(dispenser_t *dispenser, motorAddress_t address, uint8_t dispenserCL, uint16_t searchTimeout) {
    dispenser->address = address;
    dispenser->state = (dispenserState_t){.function = &sleepState};
    dispenser->limitSwitch = createLimitSwitch(address);
    // dispenser->stepsUp = dispenserUpTime(dispenserCL) / DISPENSER_STEP_TIME_MS;
    //dispenser->searchTimeout = searchTimeout;
    dispenser->haltTime = 0;
    createMotor(dispenser->address);
    resetDispenserPosition(dispenser);
}

dispenserStateCode_t getDispenserState(dispenser_t *dispenser) {
    if (dispenser->state.function == sleepState) {
        return DISPENSER_STATE_SLEEP;
    }
    if (dispenser->state.function == upState) {
        return DISPENSER_STATE_UP;
    }
    if (dispenser->state.function == topState) {
        return DISPENSER_STATE_TOP;
    }
    if (dispenser->state.function == downState) {
        return DISPENSER_STATE_DOWN;
    }
    if (dispenser->state.function == errorState) {
        return DISPENSER_STATE_ERROR;
    }
    return DISPENSER_STATE_INVALID;
}

void dispenserEmergencyStop(dispenser_t *dispenser) {
    stopMotor(dispenser->address);
    disableMotorByPin(dispenser->address);
    dispenserSetHaltTime(dispenser, 0);
    dispenser->state = (dispenserState_t){.function = &sleepState};
}
//we will delete it in the public version

/* endregion HEADER FUNCTIONS */

/* region STATIC FUNCTIONS */
void dispenserChangeStates(dispenser_t *dispenser) {
    if (!motorIsCommunicating(dispenser->address)) {
        dispenser->state = errorState_t;
    }
    dispenser->state = dispenser->state.function(dispenser);
}

static dispenserState_t errorState(dispenser_t *dispenser) {
    Motor_t *motor = getMotor(dispenser->address);
    setUpMotor(motor, (int)(dispenser->address));
    if (motorIsCommunicating(dispenser->address)) {
        disableMotorByPin(dispenser->address);
        dispenserSetHaltTime(dispenser, 0);
        return sleepState_t;
    }
    return errorState_t;
}

static dispenserState_t sleepState(dispenser_t *dispenser) {
    if (dispenser->haltTime > 0) {
        enableMotorByPin(dispenser->address);
        moveMotorUp(dispenser->address);
        return upState_t;
    }
    return sleepState_t;
}

static dispenserState_t upState(dispenser_t *dispenser) {
    torque = motorGetTorque(dispenser->address);
    PRINT_DEBUG("upState")
    PRINT_DEBUG("Torque: %i", torque)

    // If the torque is below 10 twice in a row, stop
    if (torque < 10){
        counterTorque++;
        if (counterTorque == 2){
            PRINT_DEBUG("detect Top Position")
            stopMotor(dispenser->address);
            counterTorque = 0;
            return topState_t;
        }
    }
    else counterTorque = 0;

    return upState_t;
}

static dispenserState_t topState(dispenser_t *dispenser) {
    PRINT_DEBUG("topState")
    if (dispenser->haltTime > 0) {
        sleep_ms(TOP_TIME_SLOT);
        dispenser->haltTime = dispenser->haltTime - TOP_TIME_SLOT;
        return topState_t;
    }
    // reset to 0 when change to down state.
    dispenserSetHaltTime(dispenser, 0);
    moveMotorDown(dispenser->address);
    return downState_t;
}

static dispenserState_t downState(dispenser_t *dispenser) {
    PRINT_DEBUG("downState")
    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        stopMotor(dispenser->address);
        disableMotorByPin(dispenser->address);
        dispenserSetHaltTime(dispenser, 0);
        return sleepState_t;
    }
    return downState_t;
}

bool dispenserAllInSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser) {
    for (uint8_t i = 0; i < number_of_dispenser; ++i) {
        if (dispenser[i].state.function != sleepState_t.function) {
            return false;
        }
    }
    return true;
}



static void resetDispenserPosition(dispenser_t *dispenser) {
    moveMotorDown(dispenser->address);
    while (!limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    stopMotor(dispenser->address);
}

void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime) {
     dispenser->haltTime = haltTime;
}

/* endregion STATIC FUNCTIONS */
