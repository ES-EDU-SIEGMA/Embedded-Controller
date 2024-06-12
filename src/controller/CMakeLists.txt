# region LEFT
add_executable(left
        ${CMAKE_CURRENT_LIST_DIR}/side_controller.c
)
target_include_directories(left PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(left
        pico_stdlib
        pico_stdio
        serialUART_lib
        tmc2209_lib
        dispenser_lib
        limitSwitch_lib
        motor_lib
        com_protocol
)
target_compile_definitions(left PUBLIC
        -DCONTROLLER_ID="LEFT"
        -DSERIAL_UART=SERIAL1
        # current hardware only allows '>=120000' for 4 cl Dispenser, without deformation
        # for 3 cl Dispenser '>=100000' is allowed, without deformation
        -DMOTOR_UP_SPEED=120000
        # current dispenser only allows '>=80000', without problems with refilling
        -DMOTOR_DOWN_SPEED=80000
)
GENERATE_PICO_BINARY(left)
# endregion LEFT

# region RIGHT
add_executable(right
        ${CMAKE_CURRENT_LIST_DIR}/side_controller.c
)
target_include_directories(right PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(right
        pico_stdlib
        pico_stdio
        serialUART_lib
        tmc2209_lib
        dispenser_lib
        limitSwitch_lib
        motor_lib
        com_protocol
)
target_compile_definitions(right PUBLIC
        -DCONTROLLER_ID="RIGHT"
        -DSERIAL_UART=SERIAL1
        # for 4cl Dispenser '>=120000' is possible, without deformation
        # for 3cl Dispenser '>=100000' is possible, without deformation
        -DMOTOR_UP_SPEED=100000
        # without problems with refilling only '>=80000' is possible
        -DMOTOR_DOWN_SPEED=80000
)
GENERATE_PICO_BINARY(right)
# endregion RIGHT

# region RONDELL
add_executable(rondell
        ${CMAKE_CURRENT_LIST_DIR}/rondell_controller.c
)
target_include_directories(rondell PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(rondell
        pico_stdlib
        pico_stdio
        rondell_lib
        serialUART_lib
        tmc2209_lib
        dispenser_lib
        limitSwitch_lib
        motor_lib
        com_protocol
)
target_compile_definitions(rondell PUBLIC
        -DCONTROLLER_ID="RONDELL"
        -DSERIAL_UART=SERIAL1
        # rondell requires motor to be slower!
        # otherwise finding the correct position won't work
        -DMOTOR_UP_SPEED=50000
        -DMOTOR_DOWN_SPEED=50000
)
GENERATE_PICO_BINARY(rondell)
# endregion RONDELL
