//
// Created by tommy on 14/11/2024.
//

#include <iostream>
#include <thread>
#include <pthread.h>
#include <test_utils.h>
#include <unistd.h>

rc_object *gObj;
pthread_mutex_t g_lock;

void foo(void *data, size_t sz) {
  memset(data, 0, sz);
}

THREAD_ENTRY void thread_func() {
  pthread_mutex_lock(&g_lock);
  rc_object *obj = gObj;
  if (!obj) {
    pthread_mutex_unlock(&g_lock);
    return;
  }

  void *data = obj->data;
  size_t sz = obj->sz;
  rc_obj_retain(obj);
  pthread_mutex_unlock(&g_lock);

  // obj is now only kept alive by its reference
  usleep(10);

  rc_obj_release(obj); // potentially destroys obj, data is now unsafe
  usleep(10);

  pthread_mutex_lock(&g_lock);
  foo(data, sz); // BUG

  rc_obj_release(obj);
  gObj = nullptr;
  pthread_mutex_unlock(&g_lock);
}

int main() {
  pthread_mutex_init(&g_lock, NULL);

  std::cout << "Testing sequential execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    thread_func();
    thread_func();
  }

  std::cout << "Success!\n\nTesting parallel execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    std::thread t1(thread_func);
    std::thread t2(thread_func);
    t1.join();
    t2.join();
  }
  return 0;
}
