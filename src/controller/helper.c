#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "pico/time.h"

#include "common.h"
#include "helper.h"
#include "motor.h"

void initHardware(bool waitForConnection) {
#ifdef DEBUG
    if (watchdog_enable_caused_reboot()) {
        reset_usb_boot(0, 0);
    }
#endif

    stdio_init_all();

    // Take a break to make sure everything is ready
    sleep_ms(2500);

    if (waitForConnection) {
        while ((!stdio_usb_connected())) {
            // waits for usb connection
        }
    }

    // Enable TMC2209 drivers
    initializeAndActivateMotorsEnablePin();

    PRINT("Hardware Initialized!");
}
