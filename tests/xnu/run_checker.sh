#!/bin/bash
CMAKE_BUILD_DIR=../../cmake-build-release/
cmake --build ${CMAKE_BUILD_DIR} --target clang
rm -f reports/*.plist
rm -rf reports/html
make -C ./reports -j8
CodeChecker parse -e html -o reports/html reports
firefox /Users/tommy/advanced-project/tests/xnu/reports/html/index.html
