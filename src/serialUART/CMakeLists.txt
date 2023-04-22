add_library(serialUART_lib INTERFACE)
target_sources(serialUART_lib INTERFACE serialUART.c)
target_include_directories(serialUART_lib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(serialUART_lib INTERFACE
        hardware_uart
        hardware_gpio
        hardware_irq)
