#ifndef SIEGMA_RONDELL_H
#define SIEGMA_RONDELL_H

#include "motor.h"
#include <stdint.h>

/// A main concept of the rondell is reading out light dependent resistor (=:LDR) values.
typedef uint16_t LDR_VALUE;
/// all possible rondell positions
typedef uint8_t rondellPosition_t;
enum rondellPosition {
    RONDELL_POSITION_0,
    RONDELL_POSITION_1,
    RONDELL_POSITION_2,
    RONDELL_POSITION_3,
    UNDEFINED
};

/// possible Rondell states.
typedef uint8_t rondellState_t;
enum rondellState {
    RONDELL_SLEEP,
    RONDELL_MOVING_COUNTER_CLOCKWISE,
    RONDELL_IN_KEY_POS,
    RONDELL_MOVING_CLOCKWISE
};

/// general often needed information of the rondell.

struct Rondell {
    motorAddress_t address;
    rondellState_t state;
    rondellPosition_t position;
    rondellPosition_t positionToDriveTo;
    LDR_VALUE max_ldr_value;
    LDR_VALUE min_ldr_value;
};
typedef struct Rondell Rondell_t;

/*! Sets up the rondell by initializing corresponding struct and setting extreme values of ldr.
 *
 * @param address[in] Address of the motor
 */
void createRondell(motorAddress_t address);

/*! This is the core-function of the rondell; it moves the rondell to the desired dispenser.
 *
 * @param positionToDriveTo rondellPosition Position that should be driven to.
 */
void moveToDispenserWithId(rondellPosition_t positionToDriveTo);

#endif // SIEGMA_RONDELL_H
