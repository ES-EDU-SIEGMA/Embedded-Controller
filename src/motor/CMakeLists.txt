add_library(motor_lib INTERFACE)
target_sources(motor_lib INTERFACE motor.c)
target_include_directories(motor_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(motor_lib INTERFACE
        pico_stdlib
        hardware_gpio
        serialUART_lib
        tmc2209_lib)
