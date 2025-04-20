//
// Created by tommy on 14/11/2024.
//

#include <iostream>
#include <thread>
#include <test_utils.h>

OSObject *gObj;

THREAD_ENTRY void thread_func() {
  // test with no lock:
  if (gObj) {
    gObj->release(); // BUG
    gObj = nullptr;
  }
}

int main() {
  std::cout << "Testing sequential execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = OSObject::create();
    thread_func();
    thread_func();
  }

  std::cout << "Success!\n\nTesting parallel execution...\n";
  for (int i = 0; i < 2000; i++) {
    gObj = OSObject::create();
    std::thread t1(thread_func);
    std::thread t2(thread_func);
    t1.join();
    t2.join();
  }
  return 0;
}
