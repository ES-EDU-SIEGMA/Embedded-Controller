#########################
## LEFT
add_executable(left
        ${CMAKE_CURRENT_LIST_DIR}/left_controller.c
        ${CMAKE_CURRENT_LIST_DIR}/helper.c
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
        # current hardware only allows '>=120000' for 4 cl Dispenser, without deformation
        # for 3 cl Dispenser '>=100000' is allowed, without deformation
        -DMOTOR_UP_SPEED=120000
        # current dispenser only allows '>=80000', without problems with refilling
        -DMOTOR_DOWN_SPEED=80000
)
GENERATE_PICO_BINARY(left)

#########################
## RIGHT
add_executable(right
        ${CMAKE_CURRENT_LIST_DIR}/right_controller.c
        ${CMAKE_CURRENT_LIST_DIR}/helper.c
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
        # current hardware only allows '>=120000' for 4 cl Dispenser, without deformation
        # for 3 cl Dispenser '>=100000' is allowed, without deformation
        -DMOTOR_UP_SPEED=120000
        # current dispenser only allows '>=80000', without problems with refilling
        -DMOTOR_DOWN_SPEED=80000
)
GENERATE_PICO_BINARY(right)

#########################
## RONDELL
add_executable(rondell
        ${CMAKE_CURRENT_LIST_DIR}/rondell_controller.c
        ${CMAKE_CURRENT_LIST_DIR}/helper.c
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
        # rondell requires motor to be slower!
        # otherwise finding the correct position won't work
        -DMOTOR_UP_SPEED=50000
        -DMOTOR_DOWN_SPEED=50000
)
GENERATE_PICO_BINARY(rondell)
