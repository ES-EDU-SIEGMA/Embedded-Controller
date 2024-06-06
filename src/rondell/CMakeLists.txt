add_library(rondell_lib INTERFACE)
target_sources(rondell_lib INTERFACE rondell.c)
target_include_directories(rondell_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(rondell_lib INTERFACE
        pico_stdlib
        hardware_adc
        motor_lib
        serialUART_lib
        tmc2209_lib
        common
)