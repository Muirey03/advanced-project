//
// Created by tommy on 14/11/2024.
//

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <atomic>

class OSMetaClassBase {
};

class OSObject : public OSMetaClassBase {
public:
	static OSObject *create() { return new OSObject(); }

	void release() {
		int old_refcnt = refCount.fetch_sub(1, std::memory_order_acq_rel);
		assert(old_refcnt > 0);
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
