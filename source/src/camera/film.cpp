#include <cstdint>
#include <fstream>

#include "camera/film.hpp"
#include "thread/threadpool.hpp"
#include "utils/profile.hpp"
#include "utils/rgb.hpp"

Film::Film(size_t width, size_t height) : width(width), height(height) {
  pixels.resize(width * height);
}

void Film::save(const std::filesystem::path &path) {
  // PROFILE("Film::save " + path.string());
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

  std::vector<uint8_t> buffer(width * height * 3);
  thread_pool.parallelFor(
    width, height,
    [&](size_t x, size_t y) {
      const auto pixel = getPixel(x, y);
      RGB rgb(pixel.color / static_cast<float>(pixel.sample_count));
      auto idx = (y * width + x) * 3;
      buffer[idx + 0] = rgb.r;
      buffer[idx + 1] = rgb.g;
      buffer[idx + 2] = rgb.b;
      // printf("%3lld, %3lld -> %3d, %3d, %3d\n", x, y, rgb.r, rgb.g, rgb.b);
    },
    false);
  thread_pool.wait();

  file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
}
