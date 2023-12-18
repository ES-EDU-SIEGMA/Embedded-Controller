#define SOURCE_FILE "DISPENSER_TEST"

#include "common.h"
#include "dispenser.h"
#include "serialUART.h"
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <pico/stdio.h>
#include <pico/stdio_usb.h>
#include <pico/time.h>

#define SERIAL_UART SERIAL2
#define NUMBER_OF_DISPENSERS 4
#define DISPENSER_SEARCH_TIMEOUT 250

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

static dispenser_t initDispenser(dispenser_t *dispenser, uint8_t dispenserId) {
    PRINT("Dispenser %i selected", dispenserId)
    switch (dispenserId) {
    case 0:
        dispenserCreate(dispenser, 0, SERIAL_UART, 4, DISPENSER_SEARCH_TIMEOUT);
        break;
    case 1:
        dispenserCreate(dispenser, 1, SERIAL_UART, 4, DISPENSER_SEARCH_TIMEOUT);
        break;
    case 2:
        dispenserCreate(dispenser, 2, SERIAL_UART, 4, DISPENSER_SEARCH_TIMEOUT);
        break;
    case 3:
        dispenserCreate(dispenser, 3, SERIAL_UART, 4, DISPENSER_SEARCH_TIMEOUT);
        break;
    default:
        PRINT("Invalid Dispenser")
        break;
    }
}

int main() {
    initPico(true);

    dispenser_t dispenser[NUMBER_OF_DISPENSERS];

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
            initDispenser(&dispenser[id], id);
            break;
        case 't':
            PRINT("Set halt time for dispenser %lu:", id)
            // use ascii code as number reference for character!!
            // 0 -> 48
            // 1 -> 49
            // 2 -> 50
            // ...
            // P -> 80
            // haltTime/100 == steps
            uint32_t haltTime = getchar_timeout_us(10000000);
            haltTime = haltTime - 48;
            if (haltTime == PICO_ERROR_TIMEOUT) {
                PRINT("No halt time received. Set to 10s.")
                haltTime = 10000;
            } else {
                haltTime *= 100;
            }
            PRINT("Halt time set to %lu", haltTime)
            dispenserSetHaltTime(&dispenser[id], haltTime);
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
                dispenserChangeStates(&dispenser[id]);
                // When all dispensers are finished, they are in the state sleep
            } while (DISPENSER_STATE_SLEEP != getDispenserState(dispenser));
            PRINT("Ready")
            break;
        case 'h':
            PRINT("Stop")
            dispenserEmergencyStop(dispenser);
        default:
            PRINT("Invalid command received!")
        }
        PRINT("#####")
    }
}
