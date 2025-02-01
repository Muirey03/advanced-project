set(CSA ${CSA_C_EXECUTABLE})
list(APPEND CSA_CFLAGS_LIST -Xclang -analyzer-output=html -o "${CMAKE_CURRENT_SOURCE_DIR}/reports" -Wno-address-of-packed-member)
add_simple_unit_test(chromeos_powervr "${CMAKE_CURRENT_SOURCE_DIR}/src/server_cmm_bridge.c")
