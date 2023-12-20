#define SOURCE_FILE "DISPENSER"

#include "dispenser.h"
#include "common.h"
#include "dispenser_internal.h"
#include <pico/time.h>

// Test

static dispenserState_t sleepState_t = (dispenserState_t){.function = &sleepState};
static dispenserState_t upState_t = (dispenserState_t){.function = &upState};
static dispenserState_t topState_t = (dispenserState_t){.function = &topState};
static dispenserState_t downState_t = (dispenserState_t){.function = &downState};
static dispenserState_t errorState_t = (dispenserState_t){.function = &errorState};

static uint32_t fCLK = 12000000;
static uint32_t timeVACTUAL = 1 << 24;

static uint8_t counterTorque = 0;
static uint16_t torque = 0;

/* region HEADER FUNCTIONS */

void dispenserCreate(dispenser_t *dispenser, SerialAddress_t address, serialUart_t uart,
                     uint8_t dispenserCL, uint16_t searchTimeout) {
    dispenser->address = address;
    dispenser->uart = uart;
    dispenser->state = (dispenserState_t){.function = &sleepState};
    dispenser->motor = createMotor(address, uart);
    dispenser->limitSwitch = createLimitSwitch(address);
    // dispenser->stepsUp = dispenserUpTime(dispenserCL) / DISPENSER_STEP_TIME_MS;
    dispenser->haltTime = 0;

    resetDispenserPosition(dispenser);
    disableMotorByPin(&(dispenser->motor));
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

void dispenserEmergencyStop(dispenser_t *dispenser) {
    stopMotor(&dispenser->motor);
    disableMotorByPin(&dispenser->motor);
    dispenserSetHaltTime(dispenser, 0);
    dispenser->state = (dispenserState_t){.function = &sleepState};
}
/* endregion HEADER FUNCTIONS */

/* region STATIC FUNCTIONS */

static void resetDispenserPosition(dispenser_t *dispenser) {
    moveMotorDown(&dispenser->motor);
    while (!limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    stopMotor(&dispenser->motor);
}

static dispenserState_t errorState(dispenser_t *dispenser) {
    setUpMotor(&dispenser->motor, dispenser->address, dispenser->uart);
    if (motorIsCommunicating(&dispenser->motor)) {
        disableMotorByPin(&dispenser->motor);
        dispenserSetHaltTime(dispenser, 0);
        return sleepState_t;
    }
    return errorState_t;
}

static dispenserState_t sleepState(dispenser_t *dispenser) {
    if (dispenser->haltTime > 0) {
        enableMotorByPin(&dispenser->motor);
        moveMotorUp(&dispenser->motor);
        return upState_t;
    }
    return sleepState_t;
}

static dispenserState_t upState(dispenser_t *dispenser) {
    torque = TMC2209_getStallGuardResult(&dispenser->motor.tmc2209);
    PRINT_DEBUG("upState")
    PRINT_DEBUG("Torque: %i", torque)

    // If the torque is below 10 twice in a row, stop
    if (torque < 10){
        counterTorque++;
        if (counterTorque == 2){
            PRINT_DEBUG("detect Top Position")
            stopMotor(&dispenser->motor);
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
    moveMotorDown(&dispenser->motor);
    return downState_t;
}

static dispenserState_t downState(dispenser_t *dispenser) {
    PRINT_DEBUG("downState")
    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        stopMotor(&dispenser->motor);
        disableMotorByPin(&dispenser->motor);
        dispenserSetHaltTime(dispenser, 0);
        return sleepState_t;
    }
    return downState_t;
}

void dispenserDoStep(dispenser_t *dispenser) {
    if (!motorIsCommunicating(&(dispenser->motor))) {
        dispenser->state = errorState_t;
    }
    dispenser->state = dispenser->state.function(dispenser);
}

void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime) {
     dispenser->haltTime = haltTime;
}

bool dispenserSetAllToSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser) {
    for (uint8_t i = 0; i < number_of_dispenser; ++i) {
        if (dispenser[i].state.function != sleepState_t.function) {
            return false;
        }
    }
    return true;
}

/* endregion STATIC FUNCTIONS */
