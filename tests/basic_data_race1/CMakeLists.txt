set(TEST_NAME basic_data_race1)
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")

set(CSA_CFLAGS "-I${INCLUDE_DIR}")

# on macOS you may want to use a different sysroot when running the CSA
# add -DCSA_SYSROOT=... to your cmake command to define it:
if(DEFINED CSA_SYSROOT AND CSA_SYSROOT)
    list(APPEND CSA_CFLAGS "-isysroot" "${CSA_SYSROOT}")
endif()

add_custom_target(${TEST_NAME}
        COMMAND ${CSA_EXECUTABLE} ${CSA_CFLAGS} --analyze -Xclang -analyzer-checker=alpha.security.RacyUAF ${SOURCES}
        DEPENDS clang
        COMMENT "Running test \"${TEST_NAME}\":"
)

# add an executable for intellisense:
add_executable(${TEST_NAME}_executable ${SOURCES})
target_include_directories(${TEST_NAME}_executable
        PRIVATE
        ${INCLUDE_DIR}
)
