#pragma once

#include <atomic>
#include <thread>

class SpinLock {
private:
  std::atomic_flag flag;

public:
  void acquire() {
    while (flag.test_and_set(std::memory_order_acquire)) {
      std::this_thread::yield();
    }
  }
  void release() { flag.clear(std::memory_order_release); }
};

class Guard {
private:
  SpinLock &spinlock;

public:
  Guard(SpinLock &spinlock) : spinlock(spinlock) { spinlock.acquire(); }
  ~Guard() { spinlock.release(); }
};