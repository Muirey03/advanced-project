//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

struct vm_object;
typedef struct vm_object *vm_object_t;

typedef struct {
  uintptr_t opaque[2];
} lck_rw_t;

extern void lck_rw_lock_exclusive(lck_rw_t *);

extern void lck_rw_unlock_exclusive(lck_rw_t *);

struct TRACKED vm_object {
  lck_rw_t Lock;
  vm_object_t vo_copy;
};

void vm_object_lock(vm_object_t object) {
  lck_rw_lock_exclusive(&object->Lock);
}

void vm_object_unlock(vm_object_t object) {
  lck_rw_unlock_exclusive(&object->Lock);
}

__attribute__((noinline)) void use_obj(vm_object_t object) {
}

__attribute((noinline)) void vm_object_reference_locked(RETAINED vm_object_t object) {
}

__attribute((noinline)) void vm_object_deallocate(CONSUMED vm_object_t object) {
}

THREAD_ENTRY void thread_func_internal(vm_object_t object) {
  vm_object_lock(object);
  vm_object_t copy = object->vo_copy;
  vm_object_lock(copy);
  vm_object_unlock(object);

  // the only thing keeping object alive now is it's reference

  vm_object_deallocate(object); // potentially destroy object, but don't mark copy as unsafe as it is still locked
  use_obj(copy); // NOBUG
}

extern vm_object_t g_obj;

void thread_func() {
  thread_func_internal(g_obj);
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
