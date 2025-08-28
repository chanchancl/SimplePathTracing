#pragma once

#include <cstddef>

#include "thread/spinlock.hpp"

class Progress {
  size_t total, current;
  size_t percent, last_percent, step;
  SpinLock lock;

public:
  Progress(size_t total, size_t step = 1);

  void update(size_t count);
};