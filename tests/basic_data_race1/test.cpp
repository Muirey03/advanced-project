//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <test_utils.h>

OSObject *gObj;

THREAD_ENTRY void thread_func() {
  // test with no lock:
  if (gObj) {
    gObj->release(); // BUG
    gObj = nullptr;
  }
}

int main() {
  for (;;) {
    gObj = OSObject::create();
    std::thread t1(thread_func);
    std::thread t2(thread_func);
    t1.join();
    t2.join();
  }
  return 0;
}
