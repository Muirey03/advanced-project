#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#define RETAINED __attribute__((annotate("rc_ownership_retained")))
#define CONSUMED __attribute__((annotate("rc_ownership_consumed")))
#define RETURNS_RETAINED __attribute__((annotate("rc_ownership_returns_retained")))
#define TRACKED __attribute__((annotate("rc_ownership_tracked")))
#define THREAD_ENTRY __attribute__((annotate("thread_entrypoint")))
#define SHARED __attribute__((annotate("shared_resource")))

#define flow_divert_pcb_zone struct flow_divert_pcb
#define zalloc_flags(z, f) (z*)malloc(sizeof(z))
#define FDLOG0(...)
#define FDLOG(...)

#define lck_mtx_init(m, x, y) pthread_mutex_init(m, NULL)
#define lck_rw_lock_shared pthread_rwlock_rdlock
#define lck_rw_lock_exclusive pthread_rwlock_wrlock
#define lck_rw_done pthread_rwlock_unlock
#define lck_mtx_lock pthread_mutex_lock
#define lck_mtx_unlock pthread_mutex_unlock
#define lck_mtx_t pthread_mutex_t
#define lck_rw_t pthread_rwlock_t

typedef int errno_t;

struct flow_divert_pcb;

struct socket {
	lck_mtx_t mtx;
	SHARED struct flow_divert_pcb* so_fd_pcb;
	uint32_t so_flags;
	uint16_t so_type;
};
typedef struct socket* socket_t;

#define SOCK_TYPE(so) so->so_type
#define SOCK_STREAM 1
#define SOCK_DGRAM 2

struct flow_divert_group {
	lck_rw_t lck;
};

struct TRACKED flow_divert_pcb {
	lck_mtx_t mtx;
	socket_t so;
	uint32_t hash;
	uint32_t flags;
	uint8_t log_level;
	struct flow_divert_group* group;
	uint32_t control_group_unit;
	uint32_t aggregate_unit;
	uint32_t policy_control_unit;
};

#define RB_FIND(...) NULL
#define RB_INSERT(...) NULL

#define SOF_FLOW_DIVERT         0x00800000

struct inpcb;
extern struct inpcb *sotoinpcb(struct socket*);
extern uint32_t necp_socket_get_flow_divert_control_unit(struct inpcb *, uint32_t*);

static inline unsigned long RandomULong(void) { return (unsigned long)rand(); }

extern uint32_t net_flowhash(const void *key, uint32_t len, const uint32_t seed);
extern uint32_t flow_divert_derive_kernel_control_unit(uint32_t, uint32_t *, bool *);
