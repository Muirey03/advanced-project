#!/bin/sh
cmake --build ../cmake-build-debug/ --target "${1}_tsan"

echo "Testing with ThreadSanitizer..."
"../cmake-build-debug/tests/${1}/${1}_tsan"
