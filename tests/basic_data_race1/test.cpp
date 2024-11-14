//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <test_utils.h>

extern OSObject* gObj;

void thread_func() {
	gObj->release();
    gObj = nullptr;
}

int main() {
	for (;;) {
		std::thread t1(thread_func);
		std::thread t2(thread_func);
        t1.join();
        t2.join();
	}
	return 0;
}
