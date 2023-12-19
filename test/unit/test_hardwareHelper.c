#include "helper.h"
#include "unity.h"
#include <string.h>

void setUp() {}

void tearDown() {}

/* region TEST isAllowedCharacter */
void test_is_AllowedCharacter() {
    bool result = isAllowedCharacter('1');
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_NotAllowedCharacter() {
    bool result = isAllowedCharacter('a');
    TEST_ASSERT_EQUAL(false, result);
}
/* endregion TEST isAllowedCharacter */

/* region TEST isLineEnd */
void test_is_LineEnd() {
    bool result = isLineEnd('\n');
    TEST_ASSERT_EQUAL(true, result);
}

void test_is_NotLineEnd() {
    bool result = isLineEnd('1');
    TEST_ASSERT_EQUAL(false, result);
}
/* endregion TEST isLineEnd */

/* region TEST isMessageToLong */
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
/* endregion TEST isMessageToLong */

/* region TEST initializeMessageHandler */
#define INPUT_BUFFER_LEN 255 /// maximum count of allowed input length
size_t characterCounter;
char *inputBuffer;

void test_initBuffer_Empty() {
    initializeMessageHandler(&inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
    TEST_ASSERT_EQUAL(0, strlen(inputBuffer));
}

void test_initCounter_Zero() {
    initializeMessageHandler(&inputBuffer, INPUT_BUFFER_LEN, &characterCounter);
    TEST_ASSERT_EQUAL(0, characterCounter);
}
/* endregion TEST initializeMessageHandler */

/* region TEST parseInputString */
void test_ParseInputString() {
    char *message = "1000;1000;1000;1000;\n";
    uint32_t parsed[4];
    for (int i = 0; i < 4; ++i) {
        parsed[i] = parseInputString(&message);
    }
    TEST_ASSERT_EQUAL(1000, parsed[0]);
    TEST_ASSERT_EQUAL(1000, parsed[1]);
    TEST_ASSERT_EQUAL(1000, parsed[2]);
    TEST_ASSERT_EQUAL(1000, parsed[3]);
}
/* endregion TEST parseInputString */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_is_AllowedCharacter);
    RUN_TEST(test_is_NotAllowedCharacter);
    RUN_TEST(test_is_LineEnd);
    RUN_TEST(test_is_NotLineEnd);
    RUN_TEST(test_is_MessageToLong);
    RUN_TEST(test_is_NotMessageToLong);
    RUN_TEST(test_is_EqualMessageToLong);
    RUN_TEST(test_initBuffer_Empty);
    RUN_TEST(test_initCounter_Zero);
    RUN_TEST(test_ParseInputString);

    return UNITY_END();
}