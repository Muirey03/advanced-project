cmake_minimum_required(VERSION 3.13)
project(advanced-project)

# Add llvm subdirectory (expects LLVM to be built already)
add_subdirectory(llvm/llvm)

# Define the path to the clang++ executable in the llvm build directory
set(CSA_EXECUTABLE "${CMAKE_BINARY_DIR}/llvm/llvm/bin/clang++")

# Make sure that clang++ executable exists in the llvm build directory
if(NOT EXISTS ${CSA_EXECUTABLE})
        message(FATAL_ERROR "clang++ not found in expected location: ${CSA_EXECUTABLE}")
endif()

add_subdirectory(tests)

add_custom_target(all_tests
        COMMAND ${CMAKE_COMMAND} --build . --target basic_data_race1
        DEPENDS basic_data_race1
        COMMENT "Running all tests:"
)
