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

void vm_fault_page(vm_object_t object) {
  // under certain circumstances, this can drop the object's lock
  if ((rand() % 100) == 0) {
    vm_object_unlock(object);
    vm_object_lock(object);
  }
}

void vm_object_update(vm_object_t object) {
  vm_object_t copy_object;
  if ((copy_object = object->vo_copy)) {
    vm_object_lock(copy_object);
    vm_object_reference_locked(copy_object); // copy_object must be retained because vm_fault_page can drop its lock
  } else
    return;
  vm_object_unlock(object);

  vm_fault_page(copy_object);
  use_obj(copy_object); // NOBUG

  vm_object_lock(object);
}

THREAD_ENTRY void memory_object_lock_request(vm_object_t object) {
  vm_object_lock(object);
  vm_object_update(object);
  vm_object_unlock(object);
}

extern vm_object_t g_obj;

void thread_func() {
  memory_object_lock_request(g_obj);
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
