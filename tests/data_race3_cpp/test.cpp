//
// Created by tommy on 14/11/2024.
//

#include <iostream>
#include <thread>
#include <test_utils.h>
#include <pthread.h>

class TestCase {
public:
  TestCase() {
    gObj = OSObject::create();
  }

  THREAD_ENTRY void method() {
    // this is unsafe, as gObj can be destroyed before the lock is acquired
    OSObject *stackRef = gObj;
    pthread_mutex_lock(&g_lock);

    if (stackRef) {
      stackRef->release(); // BUG
      gObj = nullptr;
    }
    pthread_mutex_unlock(&g_lock);
  }

private:
  SHARED OSObject *gObj;
  pthread_mutex_t g_lock;
};

int main() {
  std::cout << "Testing sequential execution...\n";
  for (int i = 0; i < 2000; i++) {
    TestCase test;
    test.method();
    test.method();
  }

  std::cout << "Success!\n\nTesting parallel execution...\n";
  for (int i = 0; i < 2000; i++) {
    TestCase test;
    std::thread t1(&TestCase::method, &test);
    std::thread t2(&TestCase::method, &test);
    t1.join();
    t2.join();
  }
  return 0;
}
