set(CSA ${CSA_CXX_EXECUTABLE})
set(CSA_CFLAGS "-fdiagnostics-color=always --analyze --analyzer-no-default-checks -Xclang -analyzer-checker=alpha.security.RacyUAF")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(REPORTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/reports")

# on macOS you may want to use a different sysroot when running the CSA
# add -DCSA_SYSROOT=... to your cmake command to define it:
if (DEFINED CSA_SYSROOT AND CSA_SYSROOT)
    set(EXTRA_CFLAGS "-isysroot ${CSA_SYSROOT}")
    set(CSA_CFLAGS "${CSA_CFLAGS} ${EXTRA_CFLAGS}")
endif ()
separate_arguments(CSA_CFLAGS_LIST UNIX_COMMAND ${CSA_CFLAGS})

function(add_simple_unit_test TEST_NAME SOURCES)
    list(APPEND CSA_CFLAGS_LIST -Xclang -analyzer-output=html -o "${REPORTS_DIR}/${TEST_NAME}")

    add_custom_target(${TEST_NAME}
            COMMAND ${CSA} ${CSA_CFLAGS_LIST} -I${INCLUDE_DIR} ${SOURCES}
            DEPENDS clang
            COMMENT "Running test \"${TEST_NAME}\" with CSA:"
    )

    separate_arguments(INFER_CFLAGS_LIST UNIX_COMMAND "-I${INCLUDE_DIR} ${EXTRA_CFLAGS}")
    add_custom_target(${TEST_NAME}_racerd
            COMMAND infer run --racerd-only -- ${CSA} ${INFER_CFLAGS_LIST} ${SOURCES}
            DEPENDS clang
            COMMENT "Running test \"${TEST_NAME}\" with Infer:"
    )

    if (DEFINED RELAY_DIR AND RELAY_DIR AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test.c")
        add_custom_command(
                OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/gcc-log.txt
                COMMAND ${CMAKE_COMMAND} -E echo "duppy -c -I../include -Wno-attributes -Wno-builtin-declaration-mismatch -Wno-free-nonheap-object test.c" > ${CMAKE_CURRENT_SOURCE_DIR}/gcc-log.txt
                COMMAND ${CMAKE_COMMAND} -E echo "rm test.o" >> ${CMAKE_CURRENT_SOURCE_DIR}/gcc-log.txt
                COMMENT "Generating gcc-log.txt for ${TEST_NAME}"
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.c
        )

        add_custom_command(
                OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ciltrees
                COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${RELAY_DIR}/scripts/dump-with-stats.sh
                COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_BINARY_DIR}/ciltrees
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/ciltrees ${CMAKE_CURRENT_BINARY_DIR}/ciltrees
                COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/ciltrees
                COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_SOURCE_DIR}/gcc-log.txt
                COMMENT "Running dump-with-stats.sh in ${TEST_NAME} to generate ciltrees"
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.c
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gcc-log.txt
        )

        add_custom_target(${TEST_NAME}_relay
                COMMAND ${CMAKE_COMMAND} -E chdir ${RELAY_DIR} ./relay_single.sh ${CMAKE_CURRENT_BINARY_DIR}/ciltrees > ${REPORTS_DIR}/${TEST_NAME}_relay.txt
                DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ciltrees 
                COMMENT "Running test \"${TEST_NAME}\" with RELAY:"
        )
    endif ()

    add_executable(${TEST_NAME}_nosan ${SOURCES})
    target_include_directories(${TEST_NAME}_nosan
            PRIVATE
            ${INCLUDE_DIR}
    )

    add_executable(${TEST_NAME}_tsan ${SOURCES})
    target_compile_options(${TEST_NAME}_tsan PRIVATE -fsanitize=thread)
    target_link_options(${TEST_NAME}_tsan PRIVATE -fsanitize=thread)
    target_include_directories(${TEST_NAME}_tsan
            PRIVATE
            ${INCLUDE_DIR}
    )
endfunction()

set(ALL_TESTS
        data_race1
        data_race1_cpp
        no_data_race1
        data_race2
        data_race2_cpp
        data_race3
        data_race3_cpp
        no_general_race1
        general_race2
        no_general_race2
        general_race3
        no_general_race3
        general_race4
        no_general_race4
        general_race5
        no_general_race5
        general_race6
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
