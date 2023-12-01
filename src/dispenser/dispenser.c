#define SOURCE_FILE "DISPENSER"

#include "dispenser.h"
#include "common.h"
#include "dispenser_internal.h"
#include <pico/time.h>

// Test

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
    // dispenser->stepsUp = dispenserUpTime(dispenserCL) / DISPENSER_STEP_TIME_MS;
    dispenser->searchTimeout = searchTimeout;

    resetDispenserPosition(dispenser);
    disableMotorByPin(&(dispenser->motor));
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
    stopMotor(&dispenser->motor);
    disableMotorByPin(&dispenser->motor);
    dispenser->haltSteps = 0;
    dispenser->state = (dispenserState_t){.function = &sleepState};
}
//we will delete it in the public version

/* endregion HEADER FUNCTIONS */

/* region STATIC FUNCTIONS */

static void resetDispenserPosition(dispenser_t *dispenser) {
    moveMotorDown(&dispenser->motor);
    while (!limitSwitchIsClosed(dispenser->limitSwitch))
        ;
    stopMotor(&dispenser->motor);
}

void dispenserSetHaltTime(dispenser_t *dispenser, uint32_t haltTime) {
    dispenser->haltSteps = haltTime / DISPENSER_STEP_TIME_MS;
    dispenser->stepsDone = 0;
    PRINT_DEBUG("Dispenser %u will stop after %hu steps", dispenser->address, dispenser->haltSteps)
}

/* endregion STATIC FUNCTIONS */
