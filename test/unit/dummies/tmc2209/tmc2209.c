/** ----------------------------------------------------------------------------
 * Adapted from:
 *  https://github.com/peterpolidoro/TMC2209
 *  Peter Polidoro peter@polidoro.io
 *** ---------------------------------------------------------------------------- */

#include "tmc2209.h"
uint64_t CURRENT_VELOCITY;

void TMC2209_setup(TMC2209_t *tmc2209, serialUart_t serial, long serial_baud_rate,
                   SerialAddress_t serial_address) {
    tmc2209->serial_address = serial_address;
    CURRENT_VELOCITY = 0;
}

void TMC2209_setupByMotor(TMC2209_t *tmc2209,SerialAddress_t serial_address){}

bool TMC2209_isSetupAndCommunicating(TMC2209_t *tmc2209) {
    return true;
}

void TMC2209_enable(TMC2209_t *tmc2209) {}

void TMC2209_disable(TMC2209_t *tmc2209) {}

bool TMC2209_disabledByInputPin(TMC2209_t *tmc2209) {
    return false;
}

void TMC2209_setRunCurrent(TMC2209_t *tmc2209, uint8_t percent) {}

void TMC2209_setHoldCurrent(TMC2209_t *tmc2209, uint8_t percent) {}

uint16_t TMC2209_getStallGuardResult(TMC2209_t *tmc2209) {
    return 0;
}

void TMC2209_moveAtVelocity(TMC2209_t *tmc2209, int32_t microsteps_per_period) {
    CURRENT_VELOCITY = microsteps_per_period;
}
