######################################################################
# LimitSwitch Test
add_executable(integrationTest-limitSwitches integrationTest_limitSwitch.c)
target_include_directories(integrationTest-limitSwitches PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(integrationTest-limitSwitches
        pico_time
        pico_stdio_usb
        serialUART_lib
        dispenser_lib
        tmc2209_lib
        motor_lib
        limitSwitch_lib
        )
make_to_output_file(integrationTest-limitSwitches)

######################################################################
# Motor Test
add_executable(integrationTest-motor integrationTest_motor.c)
target_include_directories(integrationTest-motor PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(integrationTest-motor
        pico_stdio_usb
        serialUART_lib
        dispenser_lib
        tmc2209_lib
        motor_lib
        limitSwitch_lib
        )
make_to_output_file(integrationTest-motor)

######################################################################
# Dispenser Test
add_executable(integrationTest-dispenser integrationTest_dispenser.c)
target_include_directories(integrationTest-dispenser PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(integrationTest-dispenser
        pico_stdio_usb
        serialUART_lib
        dispenser_lib
        tmc2209_lib
        motor_lib
        limitSwitch_lib
        )
make_to_output_file(integrationTest-dispenser)
