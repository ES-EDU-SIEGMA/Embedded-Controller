#include "dispenser.h"
#include "serialUART.h"
#include <unity.h>

dispenser_t dispenser;

void setUp() {
    dispenserCreate(&dispenser, 0, SERIAL, 4, 100);
}

void tearDown() {}

void testDispenserStructChanged(void) {
    // If Fails, struct was changed and tests need to be adjusted
    TEST_ASSERT_EQUAL(32, sizeof(dispenser_t));
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
//    dispenser_t expected = (dispenser_t) {.address=ID,
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
