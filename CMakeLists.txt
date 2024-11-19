cmake_minimum_required(VERSION 3.13)
project(advanced-project)

# Add llvm subdirectory
add_subdirectory(llvm/llvm)

# Define the path to the CSA clang++ executable
set(CSA_EXECUTABLE "${CMAKE_BINARY_DIR}/llvm/llvm/bin/clang++")

# Add unit tests subproject
add_subdirectory(tests)

# Run all tests
set(ALL_TESTS "")
list(APPEND ALL_TESTS basic_data_race1)
list(APPEND ALL_TESTS no_data_race1)
add_custom_target(all_tests
        COMMAND ${CMAKE_COMMAND} --build . --target ${ALL_TESTS}
        DEPENDS clang
        COMMENT "Running all tests:"
)
