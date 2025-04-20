#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include "flow_divert_support.h"

#define GROUP_COUNT_MAX                                 31
#define FLOW_DIVERT_FLOW_IS_TRANSPARENT   0x00001000

lck_rw_t g_flow_divert_group_lck;
static struct flow_divert_pcb           nil_pcb;
static struct flow_divert_group         **g_flow_divert_groups  = NULL;
static uint32_t                         g_active_group_count    = 0;

extern void flow_divert_set_protosw(struct socket*);
extern void flow_divert_set_udp_protosw(struct socket*);

void FDRETAIN(RETAINED struct flow_divert_pcb* pdb) {}
void FDRELEASE(CONSUMED struct flow_divert_pcb* pdb) {}

void socket_lock(struct socket* so, int ref) {
	lck_mtx_lock(&so->mtx);
}

void socket_unlock(struct socket* so, int ref) {
	lck_mtx_unlock(&so->mtx);
}

static struct flow_divert_pcb *
flow_divert_pcb_create(socket_t so)
{
	struct flow_divert_pcb  *new_pcb = NULL;

	new_pcb = zalloc_flags(flow_divert_pcb_zone, Z_WAITOK | Z_ZERO);
	lck_mtx_init(&new_pcb->mtx, &flow_divert_mtx_grp, &flow_divert_mtx_attr);
	new_pcb->so = so;
	new_pcb->log_level = nil_pcb.log_level;

	FDRETAIN(new_pcb);      /* Represents the socket's reference */

	return new_pcb;
}

static errno_t
flow_divert_pcb_insert(struct flow_divert_pcb *fd_cb, uint32_t ctl_unit)
{
	errno_t                                                 error                                           = 0;
	struct                                          flow_divert_pcb *exist          = NULL;
	struct flow_divert_group        *group;
	static uint32_t                         g_nextkey                                       = 1;
	static uint32_t                         g_hash_seed                                     = 0;
	int                                                     try_count                                       = 0;

	if (ctl_unit == 0 || ctl_unit >= GROUP_COUNT_MAX) {
		return EINVAL;
	}

	socket_unlock(fd_cb->so, 0);
	lck_rw_lock_shared(&g_flow_divert_group_lck);

	if (g_flow_divert_groups == NULL || g_active_group_count == 0) {
		FDLOG0(LOG_ERR, &nil_pcb, "No active groups, flow divert cannot be used for this socket");
		error = ENETUNREACH;
		goto done;
	}

	group = g_flow_divert_groups[ctl_unit];
	if (group == NULL) {
		FDLOG(LOG_ERR, &nil_pcb, "Group for control unit %u is NULL, flow divert cannot be used for this socket", ctl_unit);
		error = ENETUNREACH;
		goto done;
	}

	socket_lock(fd_cb->so, 0);

	do {
		uint32_t        key[2];
		uint32_t        idx;

		key[0] = g_nextkey++;
		key[1] = RandomULong();

		if (g_hash_seed == 0) {
			g_hash_seed = RandomULong();
		}

		fd_cb->hash = net_flowhash(key, sizeof(key), g_hash_seed);

		for (idx = 1; idx < GROUP_COUNT_MAX; idx++) {
			struct flow_divert_group *curr_group = g_flow_divert_groups[idx];
			if (curr_group != NULL && curr_group != group) {
				lck_rw_lock_shared(&curr_group->lck);
				exist = RB_FIND(fd_pcb_tree, &curr_group->pcb_tree, fd_cb);
				lck_rw_done(&curr_group->lck);
				if (exist != NULL) {
					break;
				}
			}
		}

		if (exist == NULL) {
			lck_rw_lock_exclusive(&group->lck);
			exist = RB_INSERT(fd_pcb_tree, &group->pcb_tree, fd_cb);
			lck_rw_done(&group->lck);
		}
	} while (exist != NULL && try_count++ < 3);

	if (exist == NULL) {
		fd_cb->group = group;
		FDRETAIN(fd_cb);                /* The group now has a reference */
	} else {
		fd_cb->hash = 0;
		error = EEXIST;
	}

	socket_unlock(fd_cb->so, 0);

done:
	lck_rw_done(&g_flow_divert_group_lck);
	socket_lock(fd_cb->so, 0);

	return error;
}

static errno_t
flow_divert_pcb_init_internal(struct socket *so, uint32_t ctl_unit, uint32_t aggregate_unit)
{
	errno_t error = 0;
	struct flow_divert_pcb *fd_cb;
	uint32_t agg_unit = aggregate_unit;
	bool is_aggregate = false;
	uint32_t group_unit = flow_divert_derive_kernel_control_unit(ctl_unit, &agg_unit, &is_aggregate);

	if (group_unit == 0) {
		return EINVAL;
	}

	if (so->so_flags & SOF_FLOW_DIVERT) {
		return EALREADY;
	}

	fd_cb = flow_divert_pcb_create(so);
	if (fd_cb != NULL) {
		so->so_fd_pcb = fd_cb;
		so->so_flags |= SOF_FLOW_DIVERT;
		fd_cb->control_group_unit = group_unit;
		fd_cb->policy_control_unit = ctl_unit;
		fd_cb->aggregate_unit = agg_unit;
		if (is_aggregate) {
			fd_cb->flags |= FLOW_DIVERT_FLOW_IS_TRANSPARENT;
		} else {
			fd_cb->flags &= ~FLOW_DIVERT_FLOW_IS_TRANSPARENT;
		}

		error = flow_divert_pcb_insert(fd_cb, group_unit);
		if (error) {
			FDLOG(LOG_ERR, fd_cb, "pcb insert failed: %d", error);
			so->so_fd_pcb = NULL;
			so->so_flags &= ~SOF_FLOW_DIVERT;
			FDRELEASE(fd_cb);
		} else {
			if (SOCK_TYPE(so) == SOCK_STREAM) {
				flow_divert_set_protosw(so);
			} else if (SOCK_TYPE(so) == SOCK_DGRAM) {
				flow_divert_set_udp_protosw(so);
			}

			FDLOG0(LOG_INFO, fd_cb, "Created");
		}
	} else {
		error = ENOMEM;
	}

	return error;
}


void disconnectx(socket_t so) {
	FDRELEASE(so->so_fd_pcb);
	so->so_fd_pcb = NULL;
}

errno_t
flow_divert_pcb_init(struct socket *so)
{
	struct inpcb *inp = sotoinpcb(so);
	uint32_t aggregate_units = 0;
	uint32_t ctl_unit = necp_socket_get_flow_divert_control_unit(inp, &aggregate_units);
	return flow_divert_pcb_init_internal(so, ctl_unit, aggregate_units);
}

socket_t shared_so = NULL;

void* thread1(void* unsued) {
	socket_lock(shared_so, 0);
	flow_divert_pcb_init(shared_so);
	socket_unlock(shared_so, 0);
}

void* thread2(void* unsued) {
	socket_lock(shared_so, 0);
	disconnectx(shared_so);
	socket_unlock(shared_so, 0);
}

int main() {
	for (;;) {
		shared_so = malloc(sizeof(struct socket));
		pthread_t t1, t2;
		pthread_create(&t1, NULL, thread1, NULL);
		pthread_create(&t2, NULL, thread2, NULL);
		pthread_join(t1, NULL);
		pthread_join(t2, NULL);
	}
}
