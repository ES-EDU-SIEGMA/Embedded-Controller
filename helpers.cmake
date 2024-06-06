include(FetchContent)

MACRO(FETCH_UNITY)
     FetchContent_Declare(
            unity
            GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
            GIT_TAG v2.5.2
    )
    FetchContent_Populate(unity)

    add_subdirectory(${unity_SOURCE_DIR})
ENDMACRO()

MACRO(INCLUDE_PICO_SDK)
    set(PICO_SDK_FETCH_FROM_GIT on)
    include(pico_sdk_import.cmake)
ENDMACRO()

MACRO(LOAD_PICO_SDK board)
    set(PICO_BOARD ${board})
    pico_sdk_init()
ENDMACRO()

MACRO(GENERATE_PICO_BINARY target)
    # enable STDIO via USB
    pico_enable_stdio_usb(${target} 1)
    # disable STDIO via UART
    pico_enable_stdio_uart(${target} 0)

    # create map/bin/hex/uf2 file etc.
    pico_add_uf2_output(${target})
ENDMACRO()

