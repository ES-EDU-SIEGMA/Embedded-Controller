#ifndef SIEGMA_DISPENSER_INTERNAL_H
#define SIEGMA_DISPENSER_INTERNAL_H

#include "include/dispenser.h"

#include <stdint.h>

static void resetDispenserPosition(dispenser_t *dispenser);

/*! Calculate the time a dispenser needs to reach TopState
 *
 * @param dispenserCL 2Cl or 4Cl Dispenser
 * @return        Time that a dispenser needs to reach TopState
 */
static uint32_t dispenserUpTime(uint8_t dispenserCL);

/* region STATES */

/*! Change dispenser state to the sleep state (wait for new command)
 *
 * @param dispenser dispenser to be set
 */
static void sleepState(dispenser_t *dispenser);

/*! Change dispenser void to the up-state (drive upwards)
 *
 * @param dispenser dispenser to be set
 */
static void upState(dispenser_t *dispenser);

/*! Change dispenser void to the top-state (stay in the up position)
 *
 * @param dispenser dispenser to be set
 */
static void topState(dispenser_t *dispenser);

/*! Change dispenser void to the down-state (drive downwards)
 *
 * @param dispenser dispenser to be set
 */
static void downState(dispenser_t *dispenser);

static void contactState(dispenser_t *dispenser);

/*! Change dispenser void to the error-state (no connection to the tmc -> try again)
 *
 * @param dispenser dispenser to be set
 */
static void errorState(dispenser_t *dispenser);

/* endregion STATES */

#endif // SIEGMA_DISPENSER_H
