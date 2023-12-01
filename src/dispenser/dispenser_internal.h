#ifndef SIEGMA_DISPENSER_INTERNAL_H
#define SIEGMA_DISPENSER_INTERNAL_H

#include "dispenser.h"
#include <stdint.h>

static void resetDispenserPosition(dispenser_t *dispenser);


/*! Calculate the time a dispenser needs to reach TopState
 *
 * @param dispenserCL 2Cl or 4Cl Dispenser
 * @return        Time that a dispenser needs to reach TopState
 */
static uint32_t dispenserUpTime(uint8_t dispenserCL);

#endif // SIEGMA_DISPENSER_INTERNAL_H
