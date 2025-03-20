//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

OSObject *gObj;
pthread_mutex_t g_lock;

THREAD_ENTRY void thread_func() {
  // this is safe, as we take the lock before we save any stack references
  pthread_mutex_lock(&g_lock);
  OSObject *stackRef = gObj;
  gObj = nullptr;
  pthread_mutex_unlock(&g_lock);

  if (stackRef) {
    stackRef->release(); // NOBUG
  }
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
