//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <test_utils.h>
#include <pthread.h>

class TestCase {
public:
  TestCase() {
    gObj = OSObject::create();
  }

  THREAD_ENTRY void method() {
    // test with no lock:
    if (gObj) {
      gObj->release(); // BUG
      gObj = nullptr;
    }
  }

private:
  SHARED OSObject *gObj;
  pthread_mutex_t g_lock;
};

int main() {
  for (;;) {
    TestCase test;
    std::thread t1(&TestCase::method, &test);
    std::thread t2(&TestCase::method, &test);
    t1.join();
    t2.join();
  }
  return 0;
}
