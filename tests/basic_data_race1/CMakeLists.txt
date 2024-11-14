set(TEST_NAME basic_data_race1)
set(SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test.cpp")
set(INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")

add_custom_target(${TEST_NAME}
        COMMAND ${CSA_EXECUTABLE} -I${INCLUDE_DIRS} --analyze -Xclang -analyzer-checker=alpha.security.RacyUAF ${SOURCES}
        DEPENDS clang
        COMMENT "Running test \"${TEST_NAME}\":"
)

# add an executable for intellisense:
add_executable(${TEST_NAME}_executable ${SOURCES})
target_include_directories(${TEST_NAME}_executable
        PRIVATE
        ${INCLUDE_DIRS}
)
