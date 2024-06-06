cmake_minimum_required(VERSION 3.13)
include(helpers.cmake)

if (DEBUG_PRINT)
    add_definitions(-DDEBUG)
endif ()

if (UNIT_TEST)
    message(NOTICE "INFO: Building Unit tests")

    project(SIEGMA C CXX ASM)

    FETCH_UNITY()
    include(CTest)

    # add libraries
    add_subdirectory(src/common)

    # add unit tests
    add_subdirectory(test/unit)
else ()
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

    INCLUDE_PICO_SDK()

    project(SIEGMA C CXX ASM)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)

    LOAD_PICO_SDK(pimoroni_tiny2040)

    if (TARGET tinyusb_device)
        # include libraries
        add_subdirectory(src/common)
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
