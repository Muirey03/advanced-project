//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

extern "C" rc_object *get_object();

void foo(void *data, size_t sz) {
  memset(data, 0, sz);
}

THREAD_ENTRY void thread_func_internal(rc_object *obj) {
  void *data = obj->data;
  size_t sz = obj->sz;

  foo(data, sz);
  // NOBUG: fields on objects are assumed to have the same lifetime as the object itself unless they themselves are tracked too
  rc_obj_release(obj);
  foo(data, sz); // BUG
}

void thread_func() {
  thread_func_internal(get_object());
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
