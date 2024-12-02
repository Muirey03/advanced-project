//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

extern OSObject *gObj;
pthread_mutex_t g_lock;

void thread_func() {
  // test global access with a lock:
  pthread_mutex_lock(&g_lock);
  if (gObj) {
    gObj->release();
    gObj = nullptr;
  }
  pthread_mutex_unlock(&g_lock);
}

int main() {
  gObj = OSObject::create();
  pthread_mutex_init(&g_lock, NULL);

  std::thread t1(thread_func);
  std::thread t2(thread_func);
  t1.join();
  t2.join();
  return 0;
}
