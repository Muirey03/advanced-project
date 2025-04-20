//
// Created by tommy on 14/11/2024.
//

#include <iostream>
#include <thread>
#include <pthread.h>
#include <test_utils.h>
#include <unistd.h>

void foo(void *data, size_t sz) {
  memset(data, 0, sz);
}

THREAD_ENTRY void thread_func_internal(rc_object *obj) {
  void *data = obj->data;
  size_t sz = obj->sz;

  foo(data, sz);
  // NOBUG: fields on objects are assumed to have the same lifetime as the object itself unless they themselves are tracked too
  rc_obj_release(obj);
  usleep(10);

  foo(data, sz); // BUG
}

pthread_mutex_t g_lock;
rc_object *gObj;

void thread_func1() {
  pthread_mutex_lock(&g_lock);
  rc_object *obj = gObj;
  if (!obj) {
    pthread_mutex_unlock(&g_lock);
    return;
  }
  rc_obj_retain(obj);
  pthread_mutex_unlock(&g_lock);

  thread_func_internal(obj);
}

void thread_func2() {
  pthread_mutex_lock(&g_lock);
  if (gObj) {
    rc_obj_release(gObj);
    gObj = NULL;
  }
  pthread_mutex_unlock(&g_lock);
}

int main() {
  pthread_mutex_init(&g_lock, NULL);

  std::cout << "Testing sequential execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    thread_func1();
    thread_func2();
  }

  std::cout << "Success!\n\nTesting parallel execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    std::thread t1(thread_func1);
    std::thread t2(thread_func2);
    t1.join();
    t2.join();
  }
  return 0;
}
