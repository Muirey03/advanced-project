#!/bin/bash
CMAKE_BUILD_DIR=../../cmake-build-release/
cmake --build ${CMAKE_BUILD_DIR} --target clang
make -C ./reports -j6
CodeChecker parse ./reports
