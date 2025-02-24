#ifndef STUBS_H
#define STUBS_H

#include <pexpert/arm64/board_config.h>

#undef CPU_HAS_APPLE_PAC
#undef HAS_APPLE_PAC
#define CONFIG_ROSETTA 1

#define THREAD_ENTRY __attribute__((annotate("thread_entrypoint")))

#define __builtin_xnu_type_signature(...) "12"
#define __builtin_xnu_type_summary(...) 0
#define __builtin_xnu_types_compatible(...) 1

#endif
