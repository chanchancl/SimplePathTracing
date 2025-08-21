

#include <glm/glm.hpp>
#include <iostream>

#include "camera.hpp"
#include "film.hpp"
#include "glm/geometric.hpp"
#include "model.hpp"
#include "plane.hpp"
#include "scene.hpp"
#include "sphere.hpp"
#include "threadpool.hpp"

using namespace std;

class SimpleTask : public Task {
public:
  virtual void run() override { cout << "Hello World!" << endl; }
};

int main() {
  ThreadPool pool{};
  Film film(1920, 1080);
  std::atomic<int> count = 0;

  Model model("models/simple_dragon.obj");

  Sphere sphere{{0, 0, 0}, 0.5f};
  // Shape &shape = sphere;

  Plane plane{{0, 0, 0}, {0, 1, 0}};

  Scene scene;
  scene.addShape(&model, {0, 0, 0}, {1, 3, 2});
  scene.addShape(&sphere, {0, 0, 1.5}, {0.3, 0.3, 0.3});
  scene.addShape(&plane, {0, -0.5, 0});

  Shape &shape = model;

  Camera camera{film, {-1.6, 0, 0}, {0, 0, 0}, 90};
  glm::vec3 light_pos{-1, 2, 1};

  printf("%lld, %lld\n", film.getWidth(), film.getHeight());
  pool.parallelFor(film.getWidth(), film.getHeight(), [&](auto x, auto y) {
    // cout << x << ' ' << y << endl;
    auto ray = camera.generateRay({x, y});
    auto hit_info = shape.intersect(ray, 0, 1e10);
    if (hit_info.has_value()) {
      auto L = glm::normalize(light_pos - hit_info->hit_point);
      auto cosine = glm::max(0.f, glm::dot(hit_info->normal, L));

      // film.setPixel(x, y, {1, 1, 1});
      film.setPixel(x, y, {cosine, cosine, cosine});
      // printf("%llu %llu, %f\n", x, y, cosine);
    }

    int n = ++count;
    if (n % (10 * film.getWidth()) == 0) {
      std::cout << static_cast<float>(n) / (film.getWidth() * film.getHeight())
                << std::endl;
    }
  });

  pool.wait();

  film.save("test.ppm");

  return 0;
}