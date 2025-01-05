//
// Created by tommy on 14/11/2024.
//

#include <thread>
#include <pthread.h>
#include <test_utils.h>

extern OSObject *gObj;
pthread_mutex_t g_lock;

void thread_func() {
  pthread_mutex_lock(&g_lock);
  OSObject* stackRef = gObj;
  pthread_mutex_unlock(&g_lock);

  // dropping the lock here means that gObj could be destroyed
  // as there are no stack references to it

  pthread_mutex_lock(&g_lock);
  if (stackRef) {
    stackRef->release(); // BUG
    gObj = nullptr;
  }
  pthread_mutex_unlock(&g_lock);
}

int main() {
  gObj = OSObject::create();
  pthread_mutex_init(&g_lock, NULL);
  std::thread t1(thread_func);
  std::thread t2(thread_func);
  t1.join();
  t2.join();
  return 0;
}
