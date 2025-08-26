#include <cstdint>
#include <fstream>

#include "camera/film.hpp"
#include "utils/rgb.hpp"

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
  file << "P6\n"
       << width << " " << height << "\n255\n";

  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {
      const auto pixel = getPixel(x, y);
      RGB rgb(pixel.color / static_cast<float>(pixel.sample_count));
      // printf("%3lld, %3lld -> %3d, %3d, %3d\n", x, y, rgb.r, rgb.g, rgb.b);
      file << static_cast<uint8_t>(rgb.r)
           << static_cast<uint8_t>(rgb.g)
           << static_cast<uint8_t>(rgb.b);
    }
  }
}
