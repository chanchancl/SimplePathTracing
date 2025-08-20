#pragma once

#include <atomic>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

#include "spinlock.hpp"

class Task {
public:
  virtual void run() = 0;
};

using ParallelFunction = std::function<void(size_t, size_t)>;

class ThreadPool {
private:
  std::vector<std::thread> threads;
  std::queue<Task *> tasks;
  SpinLock spinlock;
  std::atomic_bool alive;
  std::atomic_int pending_count;

public:
  ThreadPool(size_t thread_count = 0);
  ~ThreadPool();

  void addTask(Task *);
  Task *getTask();

  void parallelFor(size_t width, size_t height, const ParallelFunction &fun);
  void wait() const;

  static void runTask(ThreadPool *master);
};