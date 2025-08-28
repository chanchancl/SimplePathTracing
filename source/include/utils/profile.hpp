#pragma once

#include <chrono>
#include <string>

#define ENABLE_PROFILE

#ifdef ENABLE_PROFILE
#define PROFILE(name) Profile __profile(name);
#else
#define PROFILE(name)
#endif

struct Profile {
  Profile(const std::string &name);
  ~Profile();

  std::string name;
  std::chrono::high_resolution_clock::time_point start;
};