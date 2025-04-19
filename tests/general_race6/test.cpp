//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

rc_object *gObj;
pthread_mutex_t g_lock;

void foo(void *data, size_t sz) {
  memset(data, 0, sz);
}

THREAD_ENTRY void thread_func() {
  pthread_mutex_lock(&g_lock);
  rc_object *obj = gObj;
  void *data = obj->data;
  size_t sz = obj->sz;
  rc_obj_retain(obj);
  pthread_mutex_unlock(&g_lock);

  // obj is now only kept alive by its reference

  rc_obj_release(obj); // potentially destroys obj, data is now unsafe
  pthread_mutex_lock(&g_lock);
  gObj = nullptr;
  foo(data, sz); // BUG
  pthread_mutex_unlock(&g_lock);
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
