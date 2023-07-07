#define SOURCE_FILE "MOTOR_TEST"

#include "common.h"
#include "dispenser.h"
#include "motor.h"
#include "serialUART.h"
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#define SERIAL_UART SERIAL2
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

    Motor_t motor[4];

    PRINT("##########\n# Start Test")
    while (true) {
        PRINT("First write Stepper ID (0-%i) and then command:"
              "Setup (s), Up (u), Down (d), Halt(h)",
              NUMBER_OF_DISPENSERS - 1)

        uint32_t id = getchar_timeout_us(10000000);
        if (id == PICO_ERROR_TIMEOUT) {
            continue;
        } else if (id - 48 > NUMBER_OF_DISPENSERS - 1) {
            PRINT("Wrong ID")
            continue;
        } else {
            // convert ascii code to integer number
            id = id - 48;
        }

        uint32_t command = getchar_timeout_us(10000000);
        if (command == PICO_ERROR_TIMEOUT) {
            PRINT("No command received")
            continue;
        }
        switch (command) {
        case 's':
            PRINT("Setup dispenser: %lu", id)
            motor[id] = createMotor(id, SERIAL_UART);
            break;
        case 'u':
            PRINT("Move dispenser up: %lu", id)
            moveMotorUp(&motor[id], MOTOR_UP_SPEED_FAST);
            break;
        case 'd':
            PRINT("Move dispenser down: %lu", id)
            moveMotorDown(&motor[id]);
            break;
        case 'h':
            PRINT("Stop dispenser: %lu", id)
            stopMotor(&motor[id]);
            break;
        default:
            PRINT("Invalid command received!")
        }
        PRINT("#####")
    }
}
