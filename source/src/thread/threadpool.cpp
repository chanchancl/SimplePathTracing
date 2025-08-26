
#include <thread>

#include "thread/spinlock.hpp"
#include "thread/threadpool.hpp"

ThreadPool thread_pool{};

void ThreadPool::runTask(ThreadPool *master) {
  while (master->alive) {
    Task *task = master->getTask();
    if (task != nullptr) {
      task->run();
      master->pending_count--;
    } else {
      std::this_thread::yield();
    }
  }
}

ThreadPool::ThreadPool(size_t thread_count) {
  alive = true;
  pending_count = 0;
  if (thread_count == 0) {
    thread_count = std::thread::hardware_concurrency();
  }

  for (size_t i = 0; i < thread_count; i++) {
    threads.push_back(std::thread(ThreadPool::runTask, this));
  }
}

ThreadPool::~ThreadPool() {
  wait();
  alive = false;
  for (size_t i = 0; i < threads.size(); i++) {
    threads[i].join();
  }
  threads.clear();
}

void ThreadPool::addTask(Task *task) {
  Guard guard(spinlock);

  pending_count++;
  tasks.push(task);
}
Task *ThreadPool::getTask() {
  Guard guard(spinlock);

  if (tasks.empty())
    return nullptr;

  Task *ret = tasks.front();
  tasks.pop();
  return ret;
}

struct ParallelForTask : public Task {
  ParallelForTask(size_t x, size_t y, const ParallelFunction &fun)
      : x(x), y(y), fun(fun) {}
  size_t x, y;
  ParallelFunction fun;

  void run() override { fun(x, y); }
};

void ThreadPool::parallelFor(size_t width, size_t height, const ParallelFunction &fun) {
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      // printf("Add task (%lld, %lld)\n", x, y);
      addTask(new ParallelForTask(x, y, fun));
    }
  }
}

void ThreadPool::wait() const {
  while (pending_count > 0)
    std::this_thread::yield();
}
