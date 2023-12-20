#ifndef SIEGMA_RONDELL_INTERNAL_H
#define SIEGMA_RONDELL_INTERNAL_H

/// This function allows to adapt to various lighting situation since the threshold is not a fixed
/// number.
#define MEAN_OF_LDR_VALUES ((rondell.max_ldr_value + rondell.min_ldr_value) / 2)

static void setUpRondell(SerialAddress_t address, serialUart_t uart);

/*!
 * "setExtrema" is called at startup and its purpose is to set the maximum and minimum ldr value
 * in order to correctly calculate the threshold for "passDarkPeriod/passBrightPeriod".
 * The threshold formula is visible in the macro definition of "MEAN_OF_LDR_VALUES".
 */
static void setExtrema(void);

static void moveRondellClockwise(void);

static void moveRondellCounterClockwise(void);

static void stopRondell(void);

/*!
 * Being at position 0 or 3 and wanting to get to 0 or 3 is special, because applied to the set
 * {0,1,2,3} some rules of arithmetics, specifically the absolute value function, are different and
 * therefore need special consideration. Returns a bool.
 */
static uint8_t specialPositionGiven(void);

static int8_t subtractPositions(void);

static uint8_t calculatePositionDifference(void);

/*!
 * Depending on the difference between the rondell's current position and its desired position a
 * decision is being made whether to move clockwise or counter-clockwise
 */
static void startRondellAndDecideDirection(void);

static void passBrightPeriod(void);

static void passDarkPeriod(uint32_t *counter);

/*!
 * The idea of "findLongHole" is to increment a counter until a certain value, during a period in
 * which there is light, is reached; if the counter reaches the value it means that there was light
 * for so long that the passed area qualifies as a long hole. "adc_read() < MEAN_OF_LDR_VALUES"
 * might be unintuitive; since we're checking for light "adc_read() > MEAN_OF_LDR_VALUES" may be
 * expected, however this is due to a restriction of the PCB.
 */
static void findLongHole(bool *longHoleFound);

static void passLongHole(void);

static void findLongHoleAndPassIt(void);

/*!
 * The following two functions check whether the ADC reads above/below a certain value. So long as
 * that value is read, the rondell keeps moving. The usage of ">" and "<" may seem confusing; the
 * reader might think that "adc_read() > threshold" in "passDARKPeriod" does not make sense, because
 * intuitively you would rather except "adc_read < threshold", but this is necessary because of a
 * hardware restriction on the PCB that could not be changed anymore.
 *
 * The decision to not generalize the sleep-duration is based on the need for consistent behaviour.
 * The sleep period should always be the same; therefore the usage of a constant.
 */

/*
The idea for the algorithm of "identify position" is to determine time differences between certain
areas on the rondell. Tests have shown these values to be quite stable. There is a tolerance of
about ±100 for each critical value, though tests have shown that a lesser tolerance probably would
have worked too.
*/
static void identifyPosition(void);

/*!
 * This function moves the dispenser in alignment with the hopper.
 * After passBrightPeriod/passDarkPeriod there might be some extra sleep time to ensure a smooth
 * transition. Some values/instruction may seem arbitrary/inconsistent; this is because of some
 * slight inaccuracies of the rondell-pattern.
 */
static void moveRondellToKeyPosition(void);

#endif // SIEGMA_RONDELL_INTERNAL_H
