//
// Created by tommy on 14/11/2024.
//

#include <stdio.h>
#include <pthread.h>
#include <test_utils.h>
#include <unistd.h>
#include <string.h>

void foo(void *data, size_t sz) {
  memset(data, 0, sz);
}

THREAD_ENTRY void thread_func_internal(struct rc_object *obj) {
  void *data = obj->data;
  size_t sz = obj->sz;

  foo(data, sz);
  // NOBUG: fields on objects are assumed to have the same lifetime as the object itself unless they themselves are tracked too
  rc_obj_release(obj);
  usleep(10);

  foo(data, sz); // BUG
}

pthread_mutex_t g_lock;
struct rc_object *gObj;

void *thread_func1(void *unused) {
  pthread_mutex_lock(&g_lock);
  struct rc_object *obj = gObj;
  if (!obj) {
    pthread_mutex_unlock(&g_lock);
    return NULL;
  }
  rc_obj_retain(obj);
  pthread_mutex_unlock(&g_lock);

  thread_func_internal(obj);
  return NULL;
}

void *thread_func2(void *unused) {
  pthread_mutex_lock(&g_lock);
  if (gObj) {
    rc_obj_release(gObj);
    gObj = NULL;
  }
  pthread_mutex_unlock(&g_lock);
  return NULL;
}

int main() {
  pthread_mutex_init(&g_lock, NULL);

  printf("Testing sequential execution...\n");
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    thread_func1(NULL);
    thread_func2(NULL);
  }

  printf("Success!\n\nTesting parallel execution...\n");
  for (int i = 0; i < 2000; i++) {
    gObj = get_object();
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func1, NULL);
    pthread_create(&t2, NULL, thread_func2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
  }
  return 0;
}
