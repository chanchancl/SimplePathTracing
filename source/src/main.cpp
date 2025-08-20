

#include <glm/glm.hpp>
#include <iostream>

#include "camera.hpp"
#include "film.hpp"
#include "glm/geometric.hpp"
#include "sphere.hpp"
#include "threadpool.hpp"

using namespace std;

class SimpleTask : public Task {
public:
  virtual void run() override { cout << "Hello World!" << endl; }
};

int main() {
  ThreadPool pool{};
  Film film(800, 450);
  Camera camera{film, {0, 0, 1}, {0, 0, 0}, 90};

  Sphere sphere{{0, 0, 0}, 0.5f};

  glm::vec3 light_pos{1, 1, 1};

  printf("%lld, %lld\n", film.getWidth(), film.getHeight());
  pool.parallelFor(film.getWidth(), film.getHeight(), [&](auto x, auto y) {
    // cout << x << ' ' << y << endl;
    auto ray = camera.generateRay({x, y});
    auto result = sphere.intersect(ray);
    if (result.has_value()) {
      auto hit_point = ray.hit(result.value());
      auto normal = glm::normalize(hit_point - sphere.center);
      auto L = glm::normalize(light_pos - hit_point);
      auto cosine = glm::max(0.f, glm::dot(L, normal));

      film.setPixel(x, y, {cosine, cosine, cosine});
    }
  });

  pool.wait();

  film.save("test.ppm");

  return 0;
}