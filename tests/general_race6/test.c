//
// Created by tommy on 14/11/2024.
//

#include <stdio.h>
#include <pthread.h>
#include <test_utils.h>
#include <unistd.h>
#include <string.h>

struct rc_object *gObj;
pthread_mutex_t g_lock;

void foo(void *data, size_t sz) {
  memset(data, 0, sz);
}

THREAD_ENTRY void *thread_func(void *unused) {
  pthread_mutex_lock(&g_lock);
  struct rc_object *obj = gObj;
  if (!obj) {
    pthread_mutex_unlock(&g_lock);
    return NULL;
  }

  void *data = obj->data;
  size_t sz = obj->sz;
  rc_obj_retain(obj);
  pthread_mutex_unlock(&g_lock);

  // obj is now only kept alive by its reference
  usleep(10);

  rc_obj_release(obj); // potentially destroys obj, data is now unsafe
  usleep(10);

  pthread_mutex_lock(&g_lock);
  foo(data, sz); // BUG

  rc_obj_release(obj);
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
