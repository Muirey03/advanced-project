#!/bin/sh
CMAKE_BUILD_DIR=../../cmake-build-release/
cmake --build ${CMAKE_BUILD_DIR} --target clang
rm -f reports/*.plist
rm -rf reports/html
make -C ./reports -j8 post_pre_all_clangsa
