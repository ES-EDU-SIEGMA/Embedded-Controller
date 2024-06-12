add_library(com_protocol INTERFACE)
target_sources(com_protocol INTERFACE com_protocol.c)
target_include_directories(com_protocol INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(com_protocol INTERFACE
        common
)
