#ifndef SIEGMA_DISPENSER_INTERNAL_H
#define SIEGMA_DISPENSER_INTERNAL_H

#include "dispenser.h"

/*! Change dispenser state to the sleep state (wait for new command)
 *
 * @param dispenser dispenser to be set
 * @return          new state (sleep state) of the dispenser
 */
static DispenserState_t sleepState(Dispenser_t *dispenser);

/*! Change dispenser state to the up state (drive upwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (up state) of the dispenser
 */
static DispenserState_t upState(Dispenser_t *dispenser);

/*! Change dispenser state to the top state (stay in the up position)
 *
 * @param dispenser dispenser to be set
 * @return          new state (top state) of the dispenser
 */
static DispenserState_t topState(Dispenser_t *dispenser);

/*! Change dispenser state to the down state (drive downwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (down state) of the dispenser
 */
static DispenserState_t downState(Dispenser_t *dispenser);

/*! Change dispenser state to the error state (no connection to the tmc -> try again)
 *
 * @param dispenser dispenser to be set
 * @return          new state (error state) of the dispenser
 */
static DispenserState_t errorState(Dispenser_t *dispenser);

#endif // SIEGMA_DISPENSER_INTERNAL_H
