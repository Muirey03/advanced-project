#!/bin/bash
MACOS_VERSION='14.5'
CMAKE_BUILD_DIR=../../cmake-build-release/

cmake --build ${CMAKE_BUILD_DIR} --target clang --target diagtool --target clang-extdef-mapping
CSA=$(realpath ${CMAKE_BUILD_DIR}/llvm/llvm/bin/clang)

UNSIGNED_MAX=0xffffffff
MAX_STACK_DEPTH=5
CTU_IMPORT_THRESHOLD=${UNSIGNED_MAX}
MAX_NODES=225000
EXPLORATION_STRATEGY="unexplored_first_queue"
MAX_LOOP_ITERS=1
IPA_MODE="dynamic"

EXTRA_FLAGS="--analyzer-no-default-checks -Xclang -analyzer-config -Xclang ctu-phase1-inlining=all -Xclang -analyzer-inline-max-stack-depth=${MAX_STACK_DEPTH} -Xclang -analyzer-config -Xclang cfg-loopexit=true -Xclang -analyzer-config -Xclang max-times-inline-large=${UNSIGNED_MAX} -Xclang -analyzer-config -Xclang ctu-import-threshold=${CTU_IMPORT_THRESHOLD} -Xclang -analyzer-config -Xclang max-inlinable-size=${UNSIGNED_MAX} -Xclang -analyzer-config -Xclang max-nodes=${MAX_NODES} -Xclang -analyzer-config -Xclang ctu-max-nodes-pct=100 -Xclang -analyzer-config -Xclang exploration_strategy=${EXPLORATION_STRATEGY} -Xclang -analyzer-config -Xclang alpha.security.RacyUAF:Config=$(realpath racyuaf_config.yml) -Xclang -analyzer-max-loop -Xclang ${MAX_LOOP_ITERS} -Xclang -analyzer-config -Xclang ipa=${IPA_MODE} --include=$(realpath include/stubs.h)"
echo ${EXTRA_FLAGS} > extra_compile_flags.txt

export CC_ANALYZER_BIN='clangsa:'${CSA}
CodeChecker analyze ./darwin-xnu-build/.cache/${MACOS_VERSION}/compile_commands.json \
  --analyzers clangsa \
  --disable-all -e alpha.security.RacyUAF \
  --output ./reports \
  --ctu \
  --saargs extra_compile_flags.txt \
  --makefile
