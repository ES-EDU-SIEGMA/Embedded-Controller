## MAIN
add_executable(main ${CMAKE_CURRENT_LIST_DIR}/main.c)
target_include_directories(main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(main
        pico_stdlib
        hardware_adc
        rondell_lib
        serialUART_lib
        tmc2209_lib
        dispenser_lib
        limitSwitch_lib
        motor_lib
        )
make_to_output_file(main)