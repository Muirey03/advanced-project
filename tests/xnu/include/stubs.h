#ifndef STUBS_H
#define STUBS_H

#include <pexpert/arm64/board_config.h>

#undef CPU_HAS_APPLE_PAC
#undef HAS_APPLE_PAC
#define CONFIG_ROSETTA 1

#define THREAD_ENTRY __attribute__((annotate("thread_entrypoint")))
#define TRACKED __attribute__((annotate("rc_ownership_tracked")))
#define RETAINED __attribute__((annotate("rc_ownership_retained")))
#define CONSUMED __attribute__((annotate("rc_ownership_consumed")))

#define __builtin_xnu_type_signature(...) "12"
#define __builtin_xnu_type_summary(...) 0
#define __builtin_xnu_types_compatible(...) 1

#if MACH_KERNEL_PRIVATE
struct TRACKED vm_object;
extern void vm_object_reference(RETAINED struct vm_object* object);
extern void vm_object_reference_locked(RETAINED struct vm_object* object);
extern void vm_object_reference_shared(RETAINED struct vm_object* object);
extern void vm_object_deallocate(CONSUMED struct vm_object* object);
#endif

#endif
