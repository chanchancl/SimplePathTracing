
#include <chrono>
#include <cmath>
#include <cstddef>
#include <thread>

#include "thread/spinlock.hpp"
#include "thread/threadpool.hpp"

using namespace std::chrono_literals;
ThreadPool thread_pool{};

void ThreadPool::runTask(ThreadPool *master) {
  while (master->alive) {
    if (master->tasks.empty()) {
      std::this_thread::sleep_for(2ms);
      continue;
    }
    Task *task = master->getTask();
    if (task != nullptr) {
      task->run();
      delete task;
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
  ParallelForTask(size_t x, size_t y, size_t width, size_t height, const ParallelFunction &fun)
      : x(x), y(y), width(width), height(height), fun(fun) {}
  size_t x, y;
  size_t width, height;
  ParallelFunction fun;

  void run() override {
    for (size_t iy = 0; iy < height; iy++) {
      for (size_t ix = 0; ix < width; ix++) {
        fun(x + ix, y + iy);
      }
    }
  }
};

void ThreadPool::parallelFor(size_t width, size_t height, const ParallelFunction &fun, bool complex) {
  // PROFILE("paralleFor");

  float chunk_width_float = static_cast<float>(width) / std::sqrt(threads.size());
  float chunk_height_float = static_cast<float>(height) / std::sqrt(threads.size());
  if (complex) {
    chunk_width_float /= std::sqrt(16);
    chunk_height_float /= std::sqrt(16);
  }
  size_t chunk_width = std::ceil(chunk_width_float);
  size_t chunk_height = std::ceil(chunk_height_float);

  auto rcw = chunk_width, rch = chunk_height;

  for (size_t y = 0; y < height; y += chunk_height) {
    for (size_t x = 0; x < width; x += chunk_width) {
      // printf("Add task (%lld, %lld)\n", x, y);
      if (x + chunk_width > width) {
        chunk_width = width - x;
      }
      if (y + chunk_height > height) {
        chunk_height = height - y;
      }
      addTask(new ParallelForTask(x, y, chunk_width, chunk_height, fun));
      chunk_width = rcw;
      chunk_height = rch;
    }
  }
}

void ThreadPool::wait() const {
  while (pending_count > 0)
    std::this_thread::yield();
}
