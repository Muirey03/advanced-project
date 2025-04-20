//
// Created by tommy on 14/11/2024.
//

#include <iostream>
#include <thread>
#include <test_utils.h>
#include <pthread.h>

class MyObject : public OSObject {
public:
  MyObject() {
    pthread_mutex_init(&lock, NULL);
    field0 = OSObject::create();
  }

  static MyObject *create() {
    return new MyObject;
  }

  OSObject *field0 = nullptr;
  pthread_mutex_t lock;
};

class TestCase {
public:
  TestCase() {
    gObj = MyObject::create();
  }

  THREAD_ENTRY void method() {
    pthread_mutex_lock(&g_lock);
    MyObject *obj = gObj;
    obj->retain();
    pthread_mutex_unlock(&g_lock);

    if (obj->field0) {
      obj->field0->release(); // BUG
      obj->field0 = NULL;
    }
    obj->release();
  }

private:
  SHARED MyObject *gObj;
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
