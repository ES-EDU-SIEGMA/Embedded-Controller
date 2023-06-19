#ifndef SIEGMA_DISPENSER_INTERNAL_H
#define SIEGMA_DISPENSER_INTERNAL_H

#include "dispenser.h"
#include <stdint.h>

static void resetDispenserPosition(dispenser_t *dispenser);

static void findDirection(dispenser_t *dispenser, uint32_t time);

/*! Change dispenser state to the sleep state (wait for new command)
 *
 * @param dispenser dispenser to be set
 * @return          new state (sleep state) of the dispenser
 */
static dispenserState_t sleepState(dispenser_t *dispenser);

/*! Change dispenser state to the up state (drive upwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (up state) of the dispenser
 */
static dispenserState_t upState(dispenser_t *dispenser);

/*! Change dispenser state to the top state (stay in the up position)
 *
 * @param dispenser dispenser to be set
 * @return          new state (top state) of the dispenser
 */
static dispenserState_t topState(dispenser_t *dispenser);

/*! Change dispenser state to the down state (drive downwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (down state) of the dispenser
 */
static dispenserState_t downState(dispenser_t *dispenser);

/*! Change dispenser state to the error state (no connection to the tmc -> try again)
 *
 * @param dispenser dispenser to be set
 * @return          new state (error state) of the dispenser
 */
static dispenserState_t errorState(dispenser_t *dispenser);

/*! Calculate the time a dispenser needs to reach TopState
 *
 * @param dispenserCL 2Cl or 4Cl Dispenser
 * @return        Time that a dispenser needs to reach TopState
 */
static uint16_t dispenserUpTime(uint8_t dispenserCL);

#endif // SIEGMA_DISPENSER_INTERNAL_H
