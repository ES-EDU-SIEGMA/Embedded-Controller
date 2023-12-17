#include "limitSwitch.h"
#include "unity.h"

limitSwitch_t limitSwitch;

void setUp() {}

void tearDown() {}

void testLimitSwitchStruct(void) {
    TEST_ASSERT_EQUAL(sizeof(uint8_t), sizeof(limitSwitch_t));
}

void test_LimitSwitchId0_should_Valid(void) {
    limitSwitch = createLimitSwitch(0);
    TEST_ASSERT_NOT_EQUAL_UINT8(-1, limitSwitch.pin);
}

void test_LimitSwitchId4_should_NotValid(void) {
    limitSwitch = createLimitSwitch(4);
    TEST_ASSERT_EQUAL_UINT8(-1, limitSwitch.pin);
}

void test_LimitSwitch_should_NotClose(void) {
    limitSwitch = createLimitSwitch(0);
    TEST_ASSERT_EQUAL(false, limitSwitchIsClosed(limitSwitch));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(testLimitSwitchStruct);
    RUN_TEST(test_LimitSwitchId0_should_Valid);
    RUN_TEST(test_LimitSwitchId4_should_NotValid);
    RUN_TEST(test_LimitSwitch_should_NotClose);

    return UNITY_END();
}