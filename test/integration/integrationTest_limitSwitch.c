#define SOURCE_FILE "LIMIT_SWITCH_TEST"

#include "common.h"
#include "limitSwitch.h"
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#define NUMBER_OF_DISPENSERS 4

void initPico(bool waitForUSBConnection) {
    if (watchdog_enable_caused_reboot()) {
        reset_usb_boot(0, 0);
    }

    stdio_init_all();

    // Give components time for boot up
    sleep_ms(2500);

    if (waitForUSBConnection) {
        while ((!stdio_usb_connected())) {
            // waits for usb connection
        }
    }
}

int main() {
    initPico(true);

    limitSwitch_t limitSwitch[NUMBER_OF_DISPENSERS];

    for (int i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
        limitSwitch[i] = createLimitSwitch(i);
    }

    PRINT("##########\n# Start Test")
    while (true) {
        for (int i = 0; i < NUMBER_OF_DISPENSERS; ++i) {
            if (limitSwitchIsClosed(limitSwitch[i])) {
                PRINT("Switch %i is closed", i)
            } else {
                PRINT("Switch %i is open", i)
            }
        }
        PRINT("#####");
        sleep_ms(1000);
    }
}
