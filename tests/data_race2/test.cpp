//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <test_utils.h>

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

pthread_mutex_t g_lock;
MyObject *gObj;

void thread_func() {
  pthread_mutex_lock(&g_lock);
  MyObject *obj = gObj;
  obj->retain();
  pthread_mutex_unlock(&g_lock);

  OSObject *field = obj->field0;
  field->memberFn(); // BUG (if strict, otherwise NOBUG) TODO: actually I think this is NOT a bug
  //  we hold a reference on obj, and we can assume that obj holds a reference on field0,
  //  so field is guaranteed to be safe to access.
  //  HOWEVER, field0's fields themselves cannot be accessed outside of a lock in STRICT mode.

  obj->release();
}

int main() {
  pthread_mutex_init(&g_lock, NULL);
  for (;;) {
    gObj = MyObject::create();
    std::thread t1(thread_func);
    std::thread t2(thread_func);
    t1.join();
    t2.join();
  }
  return 0;
}
