//
// Created by tommy on 14/11/2024.
//

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <atomic>
#include <pthread.h>
#include <assert.h>

#define RETAINED __attribute__((annotate("rc_ownership_retained")))
#define CONSUMED __attribute__((annotate("rc_ownership_consumed")))
#define RETURNS_RETAINED __attribute__((annotate("rc_ownership_returns_retained")))
#define TRACKED __attribute__((annotate("rc_ownership_tracked")))
#define THREAD_ENTRY __attribute__((annotate("thread_entrypoint")))
#define SHARED __attribute__((annotate("shared_resource")))

struct TRACKED rc_object {
	std::atomic<int> refcnt{1};
	pthread_mutex_t lock;
	void *data;
	size_t sz;
};

void rc_obj_retain(RETAINED struct rc_object *obj);

void rc_obj_release(CONSUMED struct rc_object *obj);

inline void rc_obj_lock(struct rc_object *obj) { pthread_mutex_lock(&obj->lock); }
inline void rc_obj_unlock(struct rc_object *obj) { pthread_mutex_unlock(&obj->lock); }

class OSMetaClassBase {
};

class OSObject : public OSMetaClassBase {
public:
	static OSObject *create() { return new OSObject(); }

	void release() {
		int old_refcnt = refCount.fetch_sub(1, std::memory_order_acq_rel);
		if (old_refcnt <= 0) { abort(); }
		if (old_refcnt == 1) {
			delete this;
		}
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

#endif //TEST_UTILS_H
