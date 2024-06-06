#define SOURCE_FILE "MOTOR_TEST"

#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#include "common.h"
#include "motor.h"
#include "serialUART.h"

#define SERIAL_UART SERIAL2
#define NUMBER_OF_DISPENSERS 4

static Motor_t motor[4];

void initPico() {
    if (watchdog_enable_caused_reboot()) {
        reset_usb_boot(0, 0);
    }

    stdio_init_all();

    // Give components time for boot up
    sleep_ms(2500);

    initializeAndActivateMotorsEnablePin();

    while (!stdio_usb_connected()) {
        // waits for usb connection
    }
}

_Noreturn static void runTest() {
    PRINT("##########\n# Start Test");
    while (true) {
        PRINT("Enter the Stepper ID (0-%i)", NUMBER_OF_DISPENSERS - 1);
        uint32_t id = getchar_timeout_us(5000000);
        if (id == PICO_ERROR_TIMEOUT) {
            PRINT("Enter the Stepper ID (0-%i)", NUMBER_OF_DISPENSERS - 1);
            continue;
        } else if (id - 48 > NUMBER_OF_DISPENSERS - 1) {
            PRINT("ID out of scope!");
            continue;
        } else {
            // convert ascii code to integer number
            id = id - 48;
        }

        PRINT("Run a Command: Setup (s), Up (u), Down (d), Halt(h)");
        uint32_t command = getchar_timeout_us(30000000);
        if (command == PICO_ERROR_TIMEOUT) {
            PRINT("No command received");
            continue;
        }
        switch (command) {
        case 's':
            PRINT("Setup dispenser: %lu", id);
            motor[id] = createMotor(id);
            break;
        case 'u':
            PRINT("Move dispenser up: %lu", id);
            moveMotorUp(id);
            break;
        case 'd':
            PRINT("Move dispenser down: %lu", id);
            moveMotorDown(id);
            break;
        case 'h':
            PRINT("Stop dispenser: %lu", id);
            stopMotor(id);
            break;
        default:
            PRINT("Invalid command received!");
        }
    }
}

int main() {
    initPico();
    runTest();
}
