#include "dispenser.h"
#include <unity.h>



void setUp() {}

void tearDown() {}

void testDispenserStructChanged(void) {
    // If Fails, struct was changed and tests need to be adjusted
    TEST_ASSERT_EQUAL(40, sizeof(dispenser_t));
}

void test_Dispenser_should_sleep(void) {
    dispenser_t dispenser[1];
    dispenserCreate(&dispenser[0], 0, 4);
    dispenserStateCode_t state = getDispenserState(&dispenser[0]);
    TEST_ASSERT_EQUAL(DISPENSER_STATE_SLEEP, state);
}

void test_Dispenser_Cycle(void) {
    dispenser_t dispenser[1];
    dispenserCreate(&dispenser[0], 0, 4);
    dispenserSetHaltTime(&dispenser[0], 10);
    do {
        dispenserExecuteNextState(&dispenser[0]);
    } while(getDispenserState(&dispenser[0]) != DISPENSER_STATE_SLEEP);
    dispenserStateCode_t state = getDispenserState(&dispenser[0]);
    TEST_ASSERT_EQUAL(DISPENSER_STATE_SLEEP, state);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(testDispenserStructChanged);
    RUN_TEST(test_Dispenser_should_sleep);
    RUN_TEST(test_Dispenser_Cycle);

    return UNITY_END();
}
