cmake_minimum_required(VERSION 3.13)
project(advanced-project)

# Add llvm subdirectory
add_subdirectory(llvm/llvm)

# Define the path to the CSA clang++ executable
set(CSA_EXECUTABLE "${CMAKE_BINARY_DIR}/llvm/llvm/bin/clang++")

# Add unit tests subproject
add_subdirectory(tests)
