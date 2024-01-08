# dispenser
add_executable(unit-test_dispenser
        ${CMAKE_CURRENT_LIST_DIR}/test_dispenser.c

        ${CMAKE_CURRENT_LIST_DIR}/dummies/hardware/gpio.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/pico/time.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/serialUART/serialUART.h
        ${CMAKE_CURRENT_LIST_DIR}/dummies/tmc2209/tmc2209.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/tmc2209/tmc2209.h

        ${CMAKE_SOURCE_DIR}/src/dispenser/dispenser.c
        ${CMAKE_SOURCE_DIR}/src/motor/motor.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/limitSwitch/limitSwitch.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/limitSwitch/limitSwitch.h

        ${CMAKE_SOURCE_DIR}/src/common.h
)
target_include_directories(unit-test_dispenser PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dummies
        ${CMAKE_CURRENT_LIST_DIR}/dummies/serialUART
        ${CMAKE_CURRENT_LIST_DIR}/dummies/tmc2209

        ${CMAKE_SOURCE_DIR}/extern

        ${CMAKE_SOURCE_DIR}/src/dispenser
        ${CMAKE_SOURCE_DIR}/src/motor
        ${CMAKE_CURRENT_LIST_DIR}/dummies/limitSwitch

        ${CMAKE_SOURCE_DIR}/src
        )
target_link_libraries(unit-test_dispenser
        unity
        )
add_test(unit-test_dispenser unit-test_dispenser)

# Limit switch
add_executable(unit-test_limitSwitch
        ${CMAKE_CURRENT_LIST_DIR}/test_limitSwitch.c
        ${CMAKE_CURRENT_LIST_DIR}/dummies/hardware/gpio.c
        ${CMAKE_SOURCE_DIR}/src/limitSwitch/limitSwitch.c
        ${CMAKE_SOURCE_DIR}/src/common.h
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
)
add_test(unit-test_limitSwitch unit-test_limitSwitch)

# hardware helper
add_executable(unit-test_hardwareHelper
        ${CMAKE_CURRENT_LIST_DIR}/test_hardwareHelper.c
        dummies/hardwareHelper/helper.c
)
target_include_directories(unit-test_hardwareHelper PUBLIC
        ${CMAKE_SOURCE_DIR}/src/hardwareHelper
        ${CMAKE_CURRENT_LIST_DIR}/dummies/hardwareHelper
)
target_link_libraries(unit-test_hardwareHelper
        unity
)
add_test(unit-test_hardwareHelper unit-test_hardwareHelper)