//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

extern "C" int create_object(rc_object **);

extern "C" rc_object *get_object();

void foo(void *data, size_t sz) {
  memset(data, 0, sz);
}

void bar(rc_object *x) {
}

void thread_func() {
  rc_object *obj = get_object();
  void *data = obj->data;
  size_t sz = obj->sz;

  foo(data, sz); // NOBUG
  rc_obj_release(obj);
  foo(data, sz); // BUG
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
