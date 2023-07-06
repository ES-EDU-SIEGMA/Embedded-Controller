#define SOURCE_FILE "DISPENSER"

#include "dispenser.h"
#include "common.h"
#include "dispenser_internal.h"
#include <pico/time.h>

//Test

static dispenserState_t sleepState_t = (dispenserState_t){.function = &sleepState};
static dispenserState_t upState_t = (dispenserState_t){.function = &upState};
static dispenserState_t topState_t = (dispenserState_t){.function = &topState};
static dispenserState_t downState_t = (dispenserState_t){.function = &downState};
static dispenserState_t errorState_t = (dispenserState_t){.function = &errorState};

static uint32_t fCLK = 12000000;
static uint32_t timeVACTUAL = 1<<24;
static uint32_t motorUpSpeedSlow = 150000;
static uint32_t motorUpSpeedFast = 180000;
static uint32_t timeForSlowSpeed;


/* region HEADER FUNCTIONS */

void dispenserCreate(dispenser_t *dispenser, SerialAddress_t address, serialUart_t uart,
                     uint8_t dispenserCL, uint16_t searchTimeout) {
    dispenser->address = address;
    dispenser->uart = uart;
    dispenser->haltSteps = 0;
    dispenser->stepsDone = 0;
    dispenser->state = (dispenserState_t){.function = &sleepState};
    dispenser->motor = createMotor(address, uart);
    dispenser->limitSwitch = createLimitSwitch(address);
    dispenser->othersTriggered = 0;
    dispenser->stepsUp = dispenserUpTimeMS(dispenserCL) / DISPENSER_STEP_TIME_MS;
    dispenser->searchTimeout = searchTimeout;

    findDirection(dispenser, 250);
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

/* endregion HEADER FUNCTIONS */

/* region STATIC FUNCTIONS */

static void resetDispenserPosition(dispenser_t *dispenser) {
    moveMotorUp(&dispenser->motor, motorUpSpeedSlow);
    while (limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    moveMotorDown(&dispenser->motor);
    while (!limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    stopMotor(&dispenser->motor);
}

// TODO: Why driving upwards, when the limitswitch is closed?
static void findDirection(dispenser_t *dispenser, uint32_t time) {
    time = time + dispenser->searchTimeout;
    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        PRINT_DEBUG("limitswitch closed")
        moveMotorUp(&dispenser->motor, motorUpSpeedSlow);
        sleep_ms(time);
        if (!limitSwitchIsClosed(dispenser->limitSwitch)) {
            stopMotor(&dispenser->motor);
            return;
        } else {
            moveMotorDown(&dispenser->motor);
            sleep_ms(time + dispenser->searchTimeout);
            if (!limitSwitchIsClosed(dispenser->limitSwitch)) {
                stopMotor(&dispenser->motor);
                dispenser->motor.direction = DIRECTION_DOWN;
                return;
            } else {
                findDirection(dispenser, time + dispenser->searchTimeout);
            }
        }
    } else {
        PRINT_DEBUG("limitswitch open")
        moveMotorDown(&dispenser->motor);
        // Every cycle the time is higher, until the limitswitch is reached
        sleep_ms(time);
        if (limitSwitchIsClosed(dispenser->limitSwitch)) {
            stopMotor(&dispenser->motor);
            dispenser->motor.direction = DIRECTION_UP;
            return;
        } else {
            moveMotorUp(&dispenser->motor, motorUpSpeedSlow);
            sleep_ms(time + dispenser->searchTimeout);
            if (limitSwitchIsClosed(dispenser->limitSwitch)) {
                stopMotor(&dispenser->motor);
                return;
            } else {
                findDirection(dispenser, time + dispenser->searchTimeout);
            }
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
        moveMotorUp(&dispenser->motor, motorUpSpeedSlow);
        return upState_t;
    }
    return sleepState_t;
}

static dispenserState_t upState(dispenser_t *dispenser) {
    PRINT_DEBUG("upState")
    PRINT_DEBUG("%i", dispenser->stepsDone)
    PRINT_DEBUG("%i", dispenser->stepsUp + 2 * dispenser->othersTriggered)
    if (dispenser->stepsDone > dispenser->stepsUp + 2 * dispenser->othersTriggered) {
        stopMotor(&dispenser->motor);
        return topState_t;
    }
    if (!limitSwitchIsClosed(dispenser->limitSwitch)) {
        dispenser->stepsDone++;
        if(dispenser->stepsDone == (timeForSlowSpeed / DISPENSER_STEP_TIME_MS)){
            PRINT_DEBUG("FastMode")
            moveMotorUp(&dispenser->motor, motorUpSpeedFast);
        }
        else PRINT_DEBUG("SlowMode")
    }
    return upState_t;
}

static dispenserState_t topState(dispenser_t *dispenser) {
    PRINT_DEBUG("topState")
    if (dispenser->stepsDone >
        dispenser->stepsUp + 2 * dispenser->othersTriggered + dispenser->haltSteps) {
        moveMotorDown(&dispenser->motor);
        return downState_t;
    }
    dispenser->stepsDone++;
    return topState_t;
}

static dispenserState_t downState(dispenser_t *dispenser) {
    PRINT_DEBUG("downState")
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
    if (!motorIsCommunicating(&(dispenser->motor))) {
        dispenser->state = errorState_t;
    }
    dispenser->state = dispenser->state.function(dispenser);
}

void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime) {
    dispenser->haltSteps = haltTime / DISPENSER_STEP_TIME_MS;
    dispenser->stepsDone = 0;
    PRINT_DEBUG("Dispenser %u will stop after %hu steps", dispenser->address, dispenser->haltSteps)
}

bool dispenserSetAllToSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser) {
    for (uint8_t i = 0; i < number_of_dispenser; ++i) {
        if (dispenser[i].state.function != sleepState_t.function) {
            return false;
        }
    }
    return true;
}

static uint16_t dispenserUpTimeMS(uint8_t dispenserCL){
    uint32_t stepsPerSecondSlow = (uint64_t)motorUpSpeedSlow * (uint64_t)fCLK / (uint64_t)timeVACTUAL;
    uint32_t stepsPerSecondFast = (uint64_t)motorUpSpeedFast * (uint64_t)fCLK / (uint64_t)timeVACTUAL;
    //! If only one speed is required, set stepsFastSpeed = 0
    uint32_t stepsSlowSpeed = 286104;
    uint32_t stepsFastSpeed = 0; // 140000 - stepsSlowSpeed;
    uint32_t stepsToReachTopState2cl = 0; // TODO: 2 cl is different to 4 cl because smaller
    timeForSlowSpeed = 1000 * stepsSlowSpeed / stepsPerSecondSlow;

    if (dispenserCL == 2){
        // TODO: return for 2 cl
    }
    else if (dispenserCL == 4){
        return 1000 * ((stepsSlowSpeed / stepsPerSecondSlow) + (stepsFastSpeed / stepsPerSecondFast));
    }
}

/* endregion STATIC FUNCTIONS */
