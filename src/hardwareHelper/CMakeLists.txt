add_library(helper INTERFACE)
target_sources(helper INTERFACE helper.c)
target_include_directories(helper INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(helper INTERFACE
        pico_bootrom
        pico_stdio_usb
        pico_time
        hardware_watchdog
        common
)
