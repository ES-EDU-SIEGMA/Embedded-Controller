add_library(dispenser_lib INTERFACE)
target_sources(dispenser_lib INTERFACE dispenser.c)
target_include_directories(dispenser_lib INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(dispenser_lib INTERFACE
        pico_time
        limitSwitch_lib
        motor_lib
        serialUART_lib
        common
)
