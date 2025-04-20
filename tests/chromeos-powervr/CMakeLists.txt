set(CSA ${CSA_C_EXECUTABLE})
list(APPEND CSA_CFLAGS_LIST -Wno-address-of-packed-member)
add_simple_unit_test(chromeos-powervr "${CMAKE_CURRENT_SOURCE_DIR}/src/server_cmm_bridge.c")
add_relay_test(chromeos-powervr "${CMAKE_CURRENT_SOURCE_DIR}/src/server_cmm_bridge.c")
