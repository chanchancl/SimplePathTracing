#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <filesystem>
#include <mutex>
#include <vector>

struct Pixel {
  glm::vec3 color{0, 0, 0};
  int sample_count{0};
};

class Film {
  size_t width, height;
  std::vector<Pixel> pixels;
  std::mutex mutex;

public:
  Film(size_t width, size_t height);

  void save(const std::filesystem::path &path);

  size_t getWidth() const { return width; }
  size_t getHeight() const { return height; }
  Pixel getPixel(size_t x, size_t y) { return pixels[y * width + x]; }
  void addSample(size_t x, size_t y, const glm::vec3 &color) {
    std::lock_guard<std::mutex> guard(mutex);

    pixels[y * width + x].color += color;
    pixels[y * width + x].sample_count++;
  }

  void clean() {
    pixels.clear();
    pixels.resize(width * height);
  }
};
