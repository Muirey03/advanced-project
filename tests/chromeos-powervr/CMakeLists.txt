set(TEST_NAME chromeos_powervr)

set(CMAKE_C_COMPILER ${CSA_C_EXECUTABLE})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CSA_CFLAGS} -Wno-address-of-packed-member")

set(SOURCES
        src/server_cmm_bridge.c
)

add_library(${TEST_NAME} OBJECT ${SOURCES})
add_dependencies(${TEST_NAME} clang)
