//
// Created by Baran Demir on 09.01.23.
//

#ifndef SIEGMA_RONDELL_H
#define SIEGMA_RONDELL_H

#endif //SIEGMA_RONDELL_H

#include <stdint.h>
#include "motor.h"

enum RondellPos {
    Pos0 = 0,
    Pos1,
    Pos2,
    Pos3,
    UNDEFINED
};

enum RondellState {
    RONDELL_SLEEP,
    RONDELL_MOVING_COUNTER_CLOCKWISE,
    RONDELL_IN_KEY_POS,
    RONDELL_MOVING_CLOCKWISE
};

typedef struct Rondell Rondell_t;

typedef struct Rondell {
    SerialAddress_t address;
    enum RondellState state;
    enum RondellPos position;
    enum RondellPos positionToDriveTo;
    Motor_t motor;
    SerialUART_t uart;
} Rondell_t;

void setUpRondell(SerialAddress_t address, SerialUART_t uart);

void moveToDispenserWithId(enum RondellPos positionToDriveTo);


