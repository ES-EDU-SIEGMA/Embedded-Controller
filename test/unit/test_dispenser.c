#include "dispenser.h"
#include "serialUART.h"
#include <unity.h>

Dispenser_t dispenser;

void setUp() {
    dispenser = dispenserCreate(0, SERIAL);
}

void tearDown() {}

void testDispenserStructChanged(void) {
    // If Fails, struct was changed and tests need to be adjusted
    TEST_ASSERT_EQUAL(32, sizeof(Dispenser_t));
}

void testDispenserCycle(void) {
    //    dispenserSetHaltTime(&dispenser, 10);
    //
    //    TEST_ASSERT_EQUAL(sleepState, dispenser.state.function);
    //
    //    for (int i = 0; i < STEPS_DISPENSERS_ARE_MOVING_UP; ++i) {
    //        dispenserDoStep(&dispenser);
    //    }
    //
    //    TEST_ASSERT_EQUAL(sleepState, dispenser.state.function);
    //    Dispenser_t expected = (Dispenser_t) {.address=ID,
    //    .state=DISPENSER_SLEEP, .haltTime=0,
    //            .motor=createMotor(ID, SERIAL),
    //            .limitSwitch=createLimitSwitch(ID)};
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(testDispenserStructChanged);
    RUN_TEST(testDispenserCycle);

    return UNITY_END();
}
