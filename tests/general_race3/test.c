//
// Created by tommy on 14/11/2024.
//

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <test_utils.h>

pthread_mutex_t g_lock;
struct rc_object *gObj;

THREAD_ENTRY void* thread_func(void* unused) {
  pthread_mutex_lock(&g_lock);
  struct rc_object *stackRef = gObj;
  if (!stackRef) {
    pthread_mutex_unlock(&g_lock);
    return NULL;
  }
  rc_obj_retain(stackRef);
  pthread_mutex_unlock(&g_lock);

  // dropping the lock here is safe for now as we hold a stack reference
  usleep(10);

  rc_obj_release(stackRef); // the stack ref should now be marked unsafe

  usleep(10);

  pthread_mutex_lock(&g_lock);
  memset(stackRef->data, 0, stackRef->sz); // BUG
  rc_obj_release(stackRef);
  gObj = NULL;
  pthread_mutex_unlock(&g_lock);
  return NULL;
}

int main() {
  pthread_mutex_init(&g_lock, NULL);
  
  printf("Testing sequential execution...\n");
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    thread_func(NULL);
    thread_func(NULL);
  }

  printf("Success!\n\nTesting parallel execution...\n");
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
  }
  return 0;
}

