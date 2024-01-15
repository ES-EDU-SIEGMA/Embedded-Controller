#define SOURCE_FILE "DISPENSER"

#include "dispenser.h"
#include "common.h"
#include "dispenser_internal.h"
#include <pico/time.h>

static uint16_t torque = 0;

static dispenserState_t sleepState_t = (dispenserState_t){.function = &sleepState};
static dispenserState_t upState_t = (dispenserState_t){.function = &upState};
static dispenserState_t topState_t = (dispenserState_t){.function = &topState};
static dispenserState_t downState_t = (dispenserState_t){.function = &downState};
static dispenserState_t errorState_t = (dispenserState_t){.function = &errorState};

/* region HEADER FUNCTIONS */
void dispenserCreate(dispenser_t *dispenser, motorAddress_t address, uint8_t dispenserCL) {
    dispenser->address = address;
    dispenser->state = (dispenserState_t){.function = &sleepState};
    dispenser->limitSwitch = createLimitSwitch(address);
    // dispenser->stepsUp = dispenserUpTime(dispenserCL) / DISPENSER_STEP_TIME_MS;
    dispenser->haltTime = 0;
    createMotor(dispenser->address);
    resetDispenserPosition(dispenser);
    dispenser->switchClosed = 0;
    dispenser->counterTorque = 0;
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

void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime) {
    dispenser->haltTime = haltTime;
}

void dispenserChangeStates(dispenser_t *dispenser) {
    if (!motorIsCommunicating(dispenser->address)) {
        dispenser->state = errorState_t;
    }
    dispenser->state = dispenser->state.function(dispenser);
}

bool dispenserAllInSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser) {
    for (uint8_t i = 0; i < number_of_dispenser; ++i) {
        if (dispenser[i].state.function != sleepState_t.function) {
            return false;
        }
    }
    return true;
}
/* endregion HEADER FUNCTIONS */

/* region STATIC FUNCTIONS */
static void resetDispenserPosition(dispenser_t *dispenser) {
    int minimum = 1000;
    int counterMinimum = 0;
    dispenser->counterTorque = 0;

    PRINT_DEBUG("resetPosition")
    moveMotorDown(dispenser->address);
    PRINT_DEBUG("moveDown")
    while (!limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    stopMotor(dispenser->address);
    moveMotorUpSlowSpeed(dispenser->address);

    while (dispenser->counterTorque < 50){
        torque = motorGetTorque(dispenser->address);
        if (torque < minimum){
            minimum = torque;
        }
        dispenser->counterTorque++;
    }
    //PRINT_DEBUG("Torque Min: %i", minimum)
    while (counterMinimum < 1){
        torque = motorGetTorque(dispenser->address);
        if(torque < (minimum-10)){
            //minimum = torque;
            counterMinimum++;
        }
    }
    stopMotor(dispenser->address);
    PRINT_DEBUG("Dipsenser Position detected")
    dispenser->counterTorque = 0;
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
        dispenser->counterTorque++;
        if (dispenser->counterTorque == 2){
            PRINT_DEBUG("detect Top Position")
            stopMotor(dispenser->address);
            dispenser->counterTorque = 0;
            return topState_t;
        }
    }
    else dispenser->counterTorque = 0;

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
    dispenser->minimum = 1000;
    dispenser->counterTorque = 0;
    dispenser->counterMinimum = 0;
    return downState_t;
}

static dispenserState_t downState(dispenser_t *dispenser) {
    int minimum, counterMinimum = 0;

    PRINT_DEBUG("downState")
    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        stopMotor(dispenser->address);
        moveMotorUpSlowSpeed(dispenser->address);
        dispenser->switchClosed = 1;
        PRINT_DEBUG("isClosed")
    }
    if (dispenser->switchClosed){
        if (dispenser->counterTorque < 50){
            torque = motorGetTorque(dispenser->address);
            if (torque < dispenser->minimum){
                dispenser->minimum = torque;
            }
            dispenser->counterTorque++;
        }
        else{
            if(dispenser->counterMinimum < 1) {
                //PRINT_DEBUG("Torque Min: %i", dispenser->minimum)
                torque = motorGetTorque(dispenser->address);
                if (torque < (dispenser->minimum - 10)) {
                    // dispenser->minimum = torque;
                    dispenser->counterMinimum++;
                }
            }
            else{
                stopMotor(dispenser->address);
                PRINT_DEBUG("Dipsenser Position detected")
                dispenser->counterTorque = 0;
                disableMotorByPin(dispenser->address);
                dispenser->switchClosed = 0;
                dispenserSetHaltTime(dispenser, 0);
                return sleepState_t;
            }
        }
    }
    return downState_t;
}
/* endregion STATIC FUNCTIONS */