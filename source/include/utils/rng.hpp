#pragma once

#include <random>

class RNG {
  mutable std::mt19937 gen;
  mutable std::uniform_real_distribution<float> uniform_d{0, 1};

public:
  RNG(size_t seed) { setSeed(seed); }
  RNG() : RNG(0) {}

  void setSeed(size_t seed) { gen.seed(seed); }
  float uniform() const { return uniform_d(gen); }
};