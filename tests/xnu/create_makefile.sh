#!/bin/bash
MACOS_VERSION='14.5'
CMAKE_BUILD_DIR=../../cmake-build-release/

cmake --build ${CMAKE_BUILD_DIR} --target clang --target diagtool --target clang-extdef-mapping
CSA=$(realpath ${CMAKE_BUILD_DIR}/llvm/llvm/bin/clang)

EXTRA_FLAGS="--analyzer-no-default-checks --include="$(realpath include/stubs.h)
echo ${EXTRA_FLAGS} > extra_compile_flags.txt

export CC_ANALYZER_BIN='clangsa:'${CSA}
CodeChecker analyze ./darwin-xnu-build/.cache/${MACOS_VERSION}/compile_commands.json \
  --analyzers clangsa \
  --disable-all -e alpha.security.RacyUAF \
  --output ./reports \
  --ctu \
  --saargs extra_compile_flags.txt \
  --makefile
