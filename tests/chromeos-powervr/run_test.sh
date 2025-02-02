#!/bin/sh
rm -f reports/*.html
cmake --build ../../cmake-build-debug/ --target chromeos_powervr
