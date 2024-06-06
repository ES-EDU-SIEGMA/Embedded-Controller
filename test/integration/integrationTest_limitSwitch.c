#define SOURCE_FILE "LIMIT_SWITCH_TEST"

#include <stdlib.h>

#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#include "common.h"
#include "limitSwitch.h"

#define NUMBER_OF_DISPENSERS 4

static limitSwitch_t limitSwitch[NUMBER_OF_DISPENSERS];

static void initPico(void) {
    if (watchdog_enable_caused_reboot()) {
        reset_usb_boot(0, 0);
    }

    stdio_init_all();

    // Give components time for boot up
    sleep_ms(2500);

    while (!stdio_usb_connected()) {
        // waits for usb connection
    }
}

static void setupLimitSwitchtes() {
    for (int i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
        limitSwitch[i] = createLimitSwitch(i);
    }
}

_Noreturn static void runTest() {
    PRINT("##########\n# Start Test");
    while (true) {
        for (int index = 0; index < NUMBER_OF_DISPENSERS; ++index) {
            if (limitSwitchIsClosed(limitSwitch[index])) {
                PRINT("Switch %i is closed", index);
            } else {
                PRINT("Switch %i is open", index);
            }
        }
        PRINT("#####");
        sleep_ms(2000);
    }
}

int main() {
    initPico();
    setupLimitSwitchtes();
    runTest();
}
