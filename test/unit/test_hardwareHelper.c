#include "helper.h"
#include "unity.h"

void setUp() {}

void tearDown() {}

void test_is_AllowedCharacter() {
    bool result = isAllowedCharacter('1');
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_NotAllowedCharacter() {
    bool result = isAllowedCharacter('a');
    TEST_ASSERT_EQUAL(false, result);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_is_AllowedCharacter);
    RUN_TEST(test_is_NotAllowedCharacter);

    return UNITY_END();
}