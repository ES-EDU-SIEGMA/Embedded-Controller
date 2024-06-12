#define SOURCE_FILE "RONDELL"

#include "rondell.h"
#include "common.h"
#include "motor.h"
#include "rondell_internal.h"
#include <hardware/adc.h>
#include <pico/time.h>

/* region VARIABLES */

#define MEASUREMENT_CYCLE_IN_SECONDS 15

static Rondell_t rondell;

/* endregion VARIABLES */

/* region HEADER FUNCTIONS */

void createRondell(motorAddress_t address) {
    setUpRondell(address);
    setExtrema();
}

void handleSpecialPosition(void) {
    PRINT("ENTERED handleSpecialPosition\n");
    if (rondell.positionToDriveTo == 3 && rondell.position == 0) {
        moveRondellCounterClockwise();
    } else {
        moveRondellClockwise();
    }
}

/*!
 * "OrdinaryPosition" means that (position, positionToDriveTo) have a difference of 1 but are
 * neither (3,0) nor (0,3).
 */
void handleOrdinaryPosition(void) {
    PRINT("ENTERED handleOrdinaryPosition");
    PRINT("rondell.position = %d, rondell.positionToDriveTo: %d", rondell.position,
          rondell.positionToDriveTo);
    // The if-condition may seem arbitrary, but it is not; it results from the corresponding
    // dispenser IDs.
    if (rondell.positionToDriveTo > rondell.position) {
        PRINT("positionToDriveTo > rondell.position");
        moveRondellClockwise();
    } else {
        PRINT("positionToDriveTo <= rondell.position");
        moveRondellCounterClockwise();
    }
}

void moveToDispenserWithId(rondellPosition_t positionToDriveTo) {
    rondell.positionToDriveTo = positionToDriveTo;

    if (rondell.positionToDriveTo == rondell.position) {
        return;
    }

    while (true) {
        moveRondellToKeyPosition();
        if (rondell.position == rondell.positionToDriveTo) {
            stopRondell();
            rondell.state = RONDELL_IN_KEY_POS;
            PRINT("reached desired position: %d, while position variable is: %d", positionToDriveTo,
                  rondell.position);
            return;
        }
    }
}

/* endregion HEADER FUNCTIONS */

/* region STATIC FUNCTION IMPLEMENTATIONS */

static void setUpRondell(motorAddress_t address) {
    rondell.address = address;
    rondell.position = UNDEFINED;
    rondell.state = RONDELL_SLEEP;
    rondell.positionToDriveTo = UNDEFINED;
    rondell.max_ldr_value = 0;
    rondell.min_ldr_value = 4095;
    createMotor(rondell.address);
}

static void moveRondellCounterClockwise(void) {
    moveMotorUp(rondell.address);
    rondell.state = RONDELL_MOVING_COUNTER_CLOCKWISE;
    sleep_ms(200);
}

static void moveRondellClockwise(void) {
    PRINT("moveRondellClockwise");
    moveMotorDown(rondell.address);
    rondell.state = RONDELL_MOVING_CLOCKWISE;
    sleep_ms(200);
}

static void stopRondell(void) {
    stopMotor(rondell.address);
}

static uint8_t specialPositionGiven(void) {
    uint8_t specialPosition;
    (rondell.position == 0 && rondell.positionToDriveTo == 3) ||
            (rondell.position == 3 && rondell.positionToDriveTo == 0)
        ? (specialPosition = 1)
        : (specialPosition = 0);
    return specialPosition;
}

static int8_t subtractPositions(void) {
    int8_t current_pos = (int8_t)rondell.position;
    int8_t positionToDriveTo = (int8_t)rondell.positionToDriveTo;
    return (current_pos - positionToDriveTo);
}

static uint8_t calculatePositionDifference(void) {
    if (specialPositionGiven()) {
        return 1;
    }
    uint8_t positionDifference;
    ((subtractPositions()) >= 0) ? (positionDifference = subtractPositions())
                                 : (positionDifference = -(subtractPositions()));
    return positionDifference;
}

static void setExtrema(void) {
    PRINT("ENTERED setExtrema");

    moveRondellClockwise();

    uint32_t collectionLimit = time_us_32() + MEASUREMENT_CYCLE_IN_SECONDS * 1000000;
    while (collectionLimit >= time_us_32()) {
        uint16_t current_val = adc_read();
        //PRINT("ADC: %i", current_val);
        if (current_val > rondell.max_ldr_value) {
            rondell.max_ldr_value = current_val;
        }
        if (current_val < rondell.min_ldr_value) {
            rondell.min_ldr_value = current_val;
        }
        sleep_ms(1);
    }

    stopRondell();

    rondell.state = RONDELL_SLEEP;
    sleep_ms(1000);

    moveToDispenserWithId(RONDELL_POSITION_0);
    rondell.state = RONDELL_SLEEP;

    PRINT("LEAVING SET EXTREMA, MAX LDR: %d, MIN LDR: %d", rondell.max_ldr_value,
          rondell.min_ldr_value);
}

static void startRondellAndDecideDirection(void) {
    PRINT("started rondell and deciding direction");
    if (rondell.position != UNDEFINED) {
        uint8_t positionDifference = calculatePositionDifference();
        PRINT("POSITION DIFFERENCE: %u", positionDifference);
        if (positionDifference == 1) {
            if (specialPositionGiven()) {
                handleSpecialPosition();
                return;
            } else {
                handleOrdinaryPosition();
                return;
            }
        }
    }
    moveRondellClockwise();
}

static void findLongHole(bool *longHoleFound) {
    PRINT("entered FINDLONGHOLE");
    int high_counter = 0;
    passDarkPeriod(0);
    while (adc_read() < MEAN_OF_LDR_VALUES) {
        if (high_counter >= 500) {
            break;
        }
        high_counter += 10;
        sleep_ms(10);
    }
    if (high_counter >= 500) {
        *longHoleFound = true;
        return;
    }
}

static void passLongHole(void) {
    passBrightPeriod();
}

static void findLongHoleAndPassIt(void) {
    if (rondell.state != RONDELL_MOVING_COUNTER_CLOCKWISE &&
        rondell.state != RONDELL_MOVING_CLOCKWISE) {
        startRondellAndDecideDirection();
    }
    bool longHoleFound = false;

    while (!longHoleFound) {
        findLongHole(&longHoleFound);
    }
    PRINT("LONG HOLE FOUND");

    passLongHole();
}

static void passDarkPeriod(uint32_t *counter) {
    while (adc_read() > MEAN_OF_LDR_VALUES) {
        if (counter) {
            *counter += 5;
        }
        sleep_ms(5);
    }
}

static void passBrightPeriod(void) {
    while (adc_read() < MEAN_OF_LDR_VALUES) {
        sleep_ms(5);
    }
}

static void identifyPosition(void) {
    // The next two lines ensure a proper transition from the long hole and counts the time for that
    // period.
    uint32_t counterLongHoleToFirstHole = 50;
    sleep_ms(50);

    passDarkPeriod(&counterLongHoleToFirstHole);
    sleep_ms(25);

    PRINT("LH TO FH: %u", counterLongHoleToFirstHole);
    // If one of the first two if statements evaluates to true the position can be determined
    // immediately due to the rondell's shape. Tests have shown that the time difference for
    // RONDELL_POSITION_2 needs wider range of tolerance.
    if (counterLongHoleToFirstHole >= 700 && counterLongHoleToFirstHole <= 1000) {
        PRINT("RONDELL POS2");
        rondell.position = RONDELL_POSITION_2;
        return;
    }
    if (counterLongHoleToFirstHole >= 400 && counterLongHoleToFirstHole <= 600) {
        PRINT("RONDELL POS3");
        rondell.position = RONDELL_POSITION_3;
        return;
    }

    // If the first two if statements were not evaluated to true, another two if statements are
    // necessary because there are two areas on the rondell with the same value for
    // "counterLongHoleToFirstHole"
    if (counterLongHoleToFirstHole >= 100 && counterLongHoleToFirstHole <= 300) {
        passBrightPeriod();

        // ensure hole has really been left and count time for that period
        uint32_t counterFirstHoleToSecondHole = 50;
        sleep_ms(50);

        passDarkPeriod(&counterFirstHoleToSecondHole);
        PRINT("FH TO 2ndH: %u", counterFirstHoleToSecondHole);
        if (counterFirstHoleToSecondHole >= 400 && counterFirstHoleToSecondHole <= 600) {
            PRINT("RONDELL POS0");
            rondell.position = RONDELL_POSITION_0;
            return;
        }
        if (counterFirstHoleToSecondHole >= 100 && counterFirstHoleToSecondHole <= 300) {
            PRINT("RONDELL POS1");
            rondell.position = RONDELL_POSITION_1;
            return;
        }
    }
}

static void moveRondellToKeyPosition(void) {
    findLongHoleAndPassIt();
    identifyPosition();
    switch (rondell.position) {
    case RONDELL_POSITION_0:
        sleep_ms(50);
        passBrightPeriod();
        sleep_ms(100);
        break;
    case RONDELL_POSITION_1:
        passBrightPeriod();
        sleep_ms(100);
        passDarkPeriod(0);
        sleep_ms(50);
        passBrightPeriod();
        sleep_ms(200);
        break;
    case RONDELL_POSITION_2:
        passBrightPeriod();
        break;
    case RONDELL_POSITION_3:
        passBrightPeriod();
        sleep_ms(100);
        passDarkPeriod(0);
        passBrightPeriod();
        break;
    default:
        break;
    }
}

/* endregion STATIC FUNCTION IMPLEMENTATIONS */
