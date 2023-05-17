cmake_minimum_required(VERSION 3.13)
include(helpers.cmake)

add_definitions(-DARDUINO_ARCH_RP2040)

if (UNIT_TEST)
    message(NOTICE "INFO: Building Unit tests")

    project(SIEGMA C CXX ASM)

    include(CTest)

    add_subdirectory(extern/unity)
    add_subdirectory(test/unit)
else ()
    if (DEBUG)
        add_definitions(-DDEBUG)
    endif ()

    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

    include(extern/pico-sdk/pico_sdk_init.cmake)

    project(SIEGMA C CXX ASM)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)

    set(PICO_BOARD pimoroni_tiny2040)

    pico_sdk_init()

    if (TARGET tinyusb_device)
        # include libraries
        add_subdirectory(src/tmc2209)
        add_subdirectory(src/serialUART)
        add_subdirectory(src/dispenser)
        add_subdirectory(src/limitSwitch)
        add_subdirectory(src/rondell)
        add_subdirectory(src/motor)
        add_subdirectory(src/hardwareHelper)

        # include integration tests
        add_subdirectory(test/integration)

        # include main target
        add_subdirectory(src)
    elseif (PICO_ON_DEVICE)
        message(WARNING "not building Project because TinyUSB submodule is not initialized in the SDK")
    endif ()
endif ()
