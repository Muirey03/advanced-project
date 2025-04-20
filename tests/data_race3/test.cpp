//
// Created by tommy on 14/11/2024.
//

#include <iostream>
#include <thread>
#include <pthread.h>
#include <test_utils.h>

OSObject *gObj;
pthread_mutex_t g_lock;

THREAD_ENTRY void thread_func() {
  // this is unsafe, as gObj can be destroyed before the lock is acquired
  OSObject *stackRef = gObj;
  pthread_mutex_lock(&g_lock);

  if (stackRef) {
    stackRef->release(); // BUG
    gObj = nullptr;
  }
  pthread_mutex_unlock(&g_lock);
}

int main() {
  pthread_mutex_init(&g_lock, NULL);

  std::cout << "Testing sequential execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = OSObject::create();
    thread_func();
    thread_func();
  }

  std::cout << "Success!\n\nTesting parallel execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = OSObject::create();
    std::thread t1(thread_func);
    std::thread t2(thread_func);
    t1.join();
    t2.join();
  }
  return 0;
}
