#########################
## LEFT
add_executable(left ${CMAKE_CURRENT_LIST_DIR}/left_controller.c)
target_include_directories(left PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(left
        pico_stdio_usb
        pico_time
        hardware_adc
        helper
        serialUART_lib
        tmc2209_lib
        dispenser_lib
        limitSwitch_lib
        motor_lib
        )
make_to_output_file(left)

#########################
## RIGHT
add_executable(right ${CMAKE_CURRENT_LIST_DIR}/right_controller.c)
target_include_directories(right PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(right
        pico_stdio_usb
        pico_time
        hardware_adc
        helper
        serialUART_lib
        tmc2209_lib
        dispenser_lib
        limitSwitch_lib
        motor_lib
        )
make_to_output_file(right)

#########################
## RONDELL
add_executable(rondell ${CMAKE_CURRENT_LIST_DIR}/rondell_controller.c)
target_include_directories(rondell PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(rondell
        pico_stdio_usb
        pico_time
        hardware_adc
        helper
        rondell_lib
        serialUART_lib
        tmc2209_lib
        dispenser_lib
        limitSwitch_lib
        motor_lib
        )
make_to_output_file(rondell)
