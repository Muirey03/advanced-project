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

struct TRACKED ipc_port;
extern void ip_release(CONSUMED struct ipc_port*);
extern void ip_reference(RETAINED struct ipc_port*);

struct TRACKED task;
extern void task_deallocate_grp(CONSUMED struct task*, uint32_t);
extern void task_reference_grp(RETAINED struct task*, uint32_t);

struct TRACKED ipc_voucher;
extern void ipc_voucher_release(CONSUMED struct ipc_voucher*);
extern void ipc_voucher_reference(RETAINED struct ipc_voucher*);
#endif

// TODO: struct socket

struct TRACKED fsevent_handle;

struct TRACKED ip_moptions;
extern void imo_addref(RETAINED struct ip_moptions *imo, int locked);
extern void imo_remref(CONSUMED struct ip_moptions *imo);

struct TRACKED fileglob;
struct proc;
extern int fg_drop(struct proc* p, CONSUMED struct fileglob *fg);
extern void fg_ref(struct proc* p, RETAINED struct fileglob *fg);

#endif
