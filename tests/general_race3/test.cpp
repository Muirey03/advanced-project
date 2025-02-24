//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

OSObject *gObj;
pthread_mutex_t g_lock;

THREAD_ENTRY void thread_func() {
  pthread_mutex_lock(&g_lock);
  OSObject *stackRef = gObj;
  stackRef->retain();
  pthread_mutex_unlock(&g_lock);

  // dropping the lock here is safe for now as we hold a stack reference

  stackRef->release(); // the stack ref should now be marked unsafe

  pthread_mutex_lock(&g_lock);
  stackRef->memberFn(); // BUG
  if (gObj) {
    gObj->release();
    gObj = nullptr;
  }
  pthread_mutex_unlock(&g_lock);
}

int main() {
  pthread_mutex_init(&g_lock, NULL);
  for (;;) {
    gObj = OSObject::create();
    std::thread t1(thread_func);
    std::thread t2(thread_func);
    t1.join();
    t2.join();
  }
  return 0;
}
