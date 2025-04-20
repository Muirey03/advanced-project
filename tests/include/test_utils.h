//
// Created by tommy on 14/11/2024.
//

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <pthread.h>
#include <assert.h>

#define RETAINED __attribute__((annotate("rc_ownership_retained")))
#define CONSUMED __attribute__((annotate("rc_ownership_consumed")))
#define RETURNS_RETAINED __attribute__((annotate("rc_ownership_returns_retained")))
#define TRACKED __attribute__((annotate("rc_ownership_tracked")))
#define THREAD_ENTRY __attribute__((annotate("thread_entrypoint")))
#define SHARED __attribute__((annotate("shared_resource")))

struct TRACKED rc_object {
	int refcnt;
	struct rc_object *child;
	pthread_mutex_t lock;
	void *data;
	size_t sz;
};

inline void rc_obj_retain(RETAINED struct rc_object *obj) {
	__atomic_fetch_add(&obj->refcnt, 1, __ATOMIC_RELAXED);
}

inline void rc_obj_destroy(struct rc_object *o) {
	free(o->data);
}

inline void rc_obj_release(CONSUMED struct rc_object *obj) {
	int old_refcnt = __atomic_fetch_sub(&obj->refcnt, 1, __ATOMIC_ACQ_REL);
	assert(old_refcnt > 0);
	if (old_refcnt == 1) {
		rc_obj_destroy(obj);
	}
}

inline struct rc_object *get_object() {
	struct rc_object *o = (struct rc_object *) malloc(sizeof(struct rc_object));
	o->refcnt = 1;
	o->sz = 50;
	o->data = malloc(o->sz);
	pthread_mutex_init(&o->lock, NULL);
	return o;
}

inline void rc_obj_lock(struct rc_object *obj) { pthread_mutex_lock(&obj->lock); }
inline void rc_obj_unlock(struct rc_object *obj) { pthread_mutex_unlock(&obj->lock); }

#ifdef __cplusplus
#include <atomic>

class OSMetaClassBase {
};

class OSObject : public OSMetaClassBase {
public:
	static OSObject *create() { return new OSObject(); }

	void release() {
		int old_refcnt = refCount.fetch_sub(1, std::memory_order_acq_rel);
		assert(old_refcnt > 0);
	}

	void retain() {
		refCount.fetch_add(1, std::memory_order_relaxed);
	}

	void memberFn() {
		// dummy member function:
		retain();
		release();
	}

private:
	std::atomic<int> refCount{1};
};

#endif

#endif //TEST_UTILS_H
