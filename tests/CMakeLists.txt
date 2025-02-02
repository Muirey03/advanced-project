set(CSA ${CSA_CXX_EXECUTABLE})
set(CSA_CFLAGS "-fdiagnostics-color=always --analyze --analyzer-no-default-checks -Xclang -analyzer-checker=alpha.security.RacyUAF")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(REPORTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/reports")

# on macOS you may want to use a different sysroot when running the CSA
# add -DCSA_SYSROOT=... to your cmake command to define it:
if (DEFINED CSA_SYSROOT AND CSA_SYSROOT)
    set(CSA_CFLAGS "${CSA_CFLAGS} -isysroot ${CSA_SYSROOT}")
endif ()
separate_arguments(CSA_CFLAGS_LIST UNIX_COMMAND ${CSA_CFLAGS})

function(add_simple_unit_test TEST_NAME SOURCES)
    list(APPEND CSA_CFLAGS_LIST -Xclang -analyzer-output=html -o "${REPORTS_DIR}/${TEST_NAME}")

    add_custom_target(${TEST_NAME}
            COMMAND ${CSA} ${CSA_CFLAGS_LIST} -I${INCLUDE_DIR} ${SOURCES}
            DEPENDS clang
            COMMENT "Running test \"${TEST_NAME}\":"
    )

    # add an executable for intellisense:
    add_executable(${TEST_NAME}_executable ${SOURCES})
    target_include_directories(${TEST_NAME}_executable
            PRIVATE
            ${INCLUDE_DIR}
    )
endfunction()

set(ALL_TESTS
        basic_data_race1
        no_data_race1
        general_race1
        no_general_race1
        general_race2
        no_general_race2
        general_race3
        general_race4
        #        data_race2
        chromeos-powervr
)

foreach (test_dir IN LISTS ALL_TESTS)
    add_subdirectory(${test_dir})
endforeach ()

# Run all tests
add_custom_target(all_tests
        COMMAND ${CMAKE_COMMAND} --build .. --target ${ALL_TESTS}
        DEPENDS clang
        COMMENT "Running all tests:"
)
