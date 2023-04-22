#define SOURCE_FILE "DISPENSER_TEST"

#include "common.h"
#include "dispenser.h"
#include "dispenser_internal.h"
#include "serialUART.h"
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#define SERIAL_UART SERIAL2

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

    Dispenser_t dispenser[NUMBER_OF_DISPENSERS];

    PRINT("##########\n# Start Test")
    while (true) {
        PRINT("First write Stepper ID (0-%i) and then command: "
              "Setup (s), Set Time (t), Run (r), Halt (while in run) (h)",
              NUMBER_OF_DISPENSERS - 1)

        uint32_t id = getchar_timeout_us(10000000);
        if (id == PICO_ERROR_TIMEOUT) {
            continue;
        } else if (id - 48 > NUMBER_OF_DISPENSERS - 1) {
            PRINT("Wrong ID")
            continue;
        } else {
            // convert ascii character to integer number
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
            dispenser[id] = createDispenser(id, SERIAL_UART);
            break;
        case 't':
            PRINT("Set halt time for dispenser %lu:", id)
            uint32_t haltTime = getchar_timeout_us(10000000);
            if (haltTime == PICO_ERROR_TIMEOUT) {
                PRINT("No halt time received. Set to 10s.")
                haltTime = 10000;
            }
            PRINT("Halt time set to %lu", haltTime)
            setDispenserHaltTime(&dispenser[id], haltTime);
            break;
        case 'r':
            PRINT("Running")
            absolute_time_t time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
            do {
                uint32_t commandInRun = getchar_timeout_us(0);
                if (commandInRun == 'h') {
                    break;
                }
                sleep_until(time);
                time = make_timeout_time_ms(DISPENSER_STEP_TIME_MS);
                // Checks for each dispenser if their next state is reached and perform the
                // according action
                dispenserDoStep(&dispenser[id]);
                // When all dispensers are finished, they are in the state sleep

                // TODO: implement dispenserGetState(...)
            } while (dispenser->state.function != sleepState);
            PRINT("Ready")
            break;
        default:
            PRINT("Invalid command received!")
        }
        PRINT("#####")
    }
}
