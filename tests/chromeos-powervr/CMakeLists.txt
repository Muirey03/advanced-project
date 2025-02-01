set(TEST_NAME chromeos_powervr)

set(CMAKE_C_COMPILER ${CSA_C_EXECUTABLE})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CSA_CFLAGS} -Wno-address-of-packed-member")

set(SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/server_cmm_bridge.c
)

add_custom_target(force_rebuild ALL COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${SOURCES})
add_library(${TEST_NAME} OBJECT ${SOURCES})
add_dependencies(${TEST_NAME} clang force_rebuild)
