######################################################################
# LimitSwitch Test
add_executable(integrationTest-limitSwitches integrationTest_limitSwitch.c)
target_include_directories(integrationTest-limitSwitches PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(integrationTest-limitSwitches
        pico_stdlib
        pico_bootrom
        pico_stdio_usb
        pico_time
        hardware_watchdog
        hardware_gpio
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
target_compile_definitions(integrationTest-motor PUBLIC
        # current hardware only allows '>=120000' for 4 cl Dispenser, without deformation
        # for 3 cl Dispenser '>=100000' is allowed, without deformation
        -DMOTOR_UP_SPEED=120000
        # current dispenser only allows '>=80000', without problems with refilling
        -DMOTOR_DOWN_SPEED=80000
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
target_compile_definitions(integrationTest-dispenser PUBLIC
        # current hardware only allows '>=120000' for 4 cl Dispenser, without deformation
        # for 3 cl Dispenser '>=100000' is allowed, without deformation
        -DMOTOR_UP_SPEED=120000
        # current dispenser only allows '>=80000', without problems with refilling
        -DMOTOR_DOWN_SPEED=80000
)
make_to_output_file(integrationTest-dispenser)

######################################################################
# ADC Test
add_executable(integrationTest-adc integrationTest_adc.c)
target_include_directories(integrationTest-adc PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(integrationTest-adc
        pico_stdlib
        pico_bootrom
        pico_stdio_usb
        pico_time
        hardware_watchdog
        hardware_gpio
        hardware_adc
        )
make_to_output_file(integrationTest-adc)

