#include "helper.h"
#include "unity.h"
#include <string.h>

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

void test_is_LineEnd() {
    bool result = isLineEnd('\n');
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_NotLineEnd() {
    bool result = isLineEnd('1');
    TEST_ASSERT_EQUAL(false, result);
}

char *receivedCharacters = "1000;1000;1000;1000;\n";
void test_is_MessageToLong() {
    bool result = isMessageToLong(strlen(receivedCharacters), 10);
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_NotMessageToLong() {
    bool result = isMessageToLong(strlen(receivedCharacters), 30);
    TEST_ASSERT_EQUAL(false, result);
}

void test_is_EqualMessageToLong() {
    bool result = isMessageToLong(strlen(receivedCharacters), 22);
    TEST_ASSERT_EQUAL(true, result);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_is_AllowedCharacter);
    RUN_TEST(test_is_NotAllowedCharacter);
    RUN_TEST(test_is_LineEnd);
    RUN_TEST(test_is_NotLineEnd);
    RUN_TEST(test_is_MessageToLong);
    RUN_TEST(test_is_NotMessageToLong);
    RUN_TEST(test_is_EqualMessageToLong);

    return UNITY_END();
}