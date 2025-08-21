
#include "film.hpp"

#include <fstream>

#include "glm/common.hpp"
#include "glm/fwd.hpp"

Film::Film(size_t width, size_t height) : width(width), height(height) {
  pixels.resize(width * height);
}

void Film::save(const std::filesystem::path &path) {
  // PPM
  // P3
  // 1920 1080
  // 255
  // 0, 0, 0
  // 255, 255, 255
  // ...
  std::ofstream file(path, std::ios::binary);
  file << "P6\n" << width << " " << height << "\n255\n";

  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      const auto p = getPixel(x, y);
      const glm::u8vec3 pi = glm::clamp(p * 255.f, 0.f, 255.f);
      file << pi.x << pi.y << pi.z;
    }
  }
}
