
#include <iostream>

#include "utils/progress.hpp"

Progress::Progress(size_t total, size_t step)
    : total(total), current(0), percent(0), last_percent(0), step(step) {
  std::cout << "0%" << std::endl;
  // std::cout << "Progress, total " << total << ", step " << step << std::endl;
}

void Progress::update(size_t count) {
  Guard guard(lock);

  current += count;
  percent = 100 * static_cast<float>(current) / static_cast<float>(total);
  if (percent - last_percent >= step) {
    last_percent = percent;
    std::cout << percent << "%" << std::endl;
    // std::cout << "count " << count << ", current " << current << ", total " << total << std::endl;
  }
}