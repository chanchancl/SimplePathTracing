#pragma once

#include "glm/fwd.hpp"
#include <cstddef>
#include <filesystem>
#include <glm/glm.hpp>
#include <vector>

class Film {
public:
  Film(size_t width, size_t height);

  void save(const std::filesystem::path &path);

  size_t getWidth() const { return width; }
  size_t getHeight() const { return height; }
  glm::vec3 getPixel(size_t x, size_t y) { return pixels[y * width + x]; }
  glm::vec3 setPixel(size_t x, size_t y, const glm::vec3 &p) {
    return pixels[y * width + x] = p;
  }

private:
  size_t width, height;
  std::vector<glm::vec3> pixels;
};
