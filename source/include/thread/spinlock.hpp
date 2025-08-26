#pragma once

#include <atomic>
#include <mutex>

class SpinLock {
private:
  std::atomic_flag flag;
  std::mutex m;

public:
  void acquire() {
    // while (flag.test_and_set(std::memory_order_acquire)) {
    //   std::this_thread::yield();
    // }
    m.lock();
  }
  void release() {
    // flag.clear(std::memory_order_release);
    m.unlock();
  }
};

class Guard {
private:
  SpinLock &spinlock;

public:
  Guard(SpinLock &spinlock) : spinlock(spinlock) { spinlock.acquire(); }
  ~Guard() { spinlock.release(); }
};