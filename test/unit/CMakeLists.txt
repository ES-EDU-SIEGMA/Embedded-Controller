# dispenser
add_executable(unit-test_dispenser
        ${CMAKE_CURRENT_LIST_DIR}/test_dispenser.c

        ${CMAKE_CURRENT_LIST_DIR}/dummies/hardware/gpio.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/pico/time.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/tmc2209/tmc2209.c

        ${CMAKE_SOURCE_DIR}/src/dispenser/dispenser.c
        ${CMAKE_SOURCE_DIR}/src/motor/motor.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/limitSwitch/limitSwitch.c
)
target_include_directories(unit-test_dispenser PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dummies
        ${CMAKE_CURRENT_LIST_DIR}/dummies/serialUART
        ${CMAKE_CURRENT_LIST_DIR}/dummies/tmc2209

        ${CMAKE_SOURCE_DIR}/extern

        ${CMAKE_SOURCE_DIR}/src/dispenser
        ${CMAKE_SOURCE_DIR}/src/motor
        ${CMAKE_SOURCE_DIR}/src/limitSwitch
        ${CMAKE_CURRENT_LIST_DIR}/dummies/limitSwitch

        ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(unit-test_dispenser
        unity
        common
)
target_compile_definitions(unit-test_dispenser PUBLIC
        -DSERIAL_UART=SERIAL1
        # current hardware only allows '>=120000' for 4 cl Dispenser, without deformation
        # for 3 cl Dispenser '>=100000' is allowed, without deformation
        -DMOTOR_UP_SPEED=120000
        # current dispenser only allows '>=80000', without problems with refilling
        -DMOTOR_DOWN_SPEED=80000
)
add_test(unit-test_dispenser unit-test_dispenser)

# Limit switch
add_executable(unit-test_limitSwitch
        ${CMAKE_CURRENT_LIST_DIR}/test_limitSwitch.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/hardware/gpio.c
        ${CMAKE_SOURCE_DIR}/src/limitSwitch/limitSwitch.c
)
target_include_directories(unit-test_limitSwitch PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dummies
        ${CMAKE_SOURCE_DIR}/extern
        ${CMAKE_SOURCE_DIR}/src/limitSwitch
        ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(unit-test_limitSwitch
        unity
        common
)
add_test(unit-test_limitSwitch unit-test_limitSwitch)

# hardware helper
#add_executable(unit-test_comProtcol
#        ${CMAKE_CURRENT_LIST_DIR}/test_comProtocol.c
#)
#target_link_libraries(unit-test_comProtcol
#        unity
#        com_protocol
#        common
#)
#add_test(unit-test_hardwareHelper unit-test_hardwareHelper)