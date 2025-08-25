

#include <cstdlib>
#include <glm/glm.hpp>
#include <iostream>
#include <random>

#include "camera.hpp"
#include "film.hpp"
#include "frame.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "model.hpp"
#include "plane.hpp"
#include "rgb.hpp"
#include "scene.hpp"
#include "sphere.hpp"
#include "threadpool.hpp"
using namespace std;

class SimpleTask : public Task {
public:
  virtual void run() override {
    cout << "Hello World!" << endl;
  }
};

int main() {
  ThreadPool pool{};
  std::atomic<int> count = 0;

  std::mt19937 gen(233233233);
  std::uniform_real_distribution<float> uniform(-1, 1);

  Model model("models/simple_dragon.obj");
  Sphere sphere{{0, 0, 0}, 1.f};
  Plane plane{{0, 0, 0}, {0, 1, 0}};

  Scene scene;
  scene.addShape(
      model,
      {RGB(202, 159, 117), false},
      {0, 0, 0},
      {1, 3, 2});
  scene.addShape(
      sphere,
      {{1, 1, 1}, false, RGB{255, 128, 128}},
      {0, 0, 2.5});
  scene.addShape(
      sphere,
      {{1, 1, 1}, false, RGB{128, 128, 255}},
      {0, 0, -2.5});
  scene.addShape(
      sphere,
      {{1, 1, 1}, true},
      {3, 0.5, -2});

  scene.addShape(
      plane,
      {RGB(128, 204, 157)},
      {0, -0.5, 0});

  Film film(192 * 4, 108 * 4);
  Camera camera{
      film,
      {-3.6, 0, 0},
      {0, 0, 0},
      45};

  int spp = 128;

  // printf("3init emissive -> %4.2f, %4.2f, %4.2f\n", scene.instances[0].material.emissive.r, scene.instances[0].material.emissive.g, scene.instances[0].material.emissive.b);
  printf("%lld, %lld\n", film.getWidth(), film.getHeight());
  pool.parallelFor(film.getWidth(), film.getHeight(), [&](auto x, auto y) {
    for (int i = 0; i < spp; i++) {
      auto ray = camera.generateRay({x, y}, {abs(uniform(gen)), abs(uniform(gen))});
      glm::vec3 beta = {1, 1, 1};
      glm::vec3 color = {0, 0, 0};
      while (true) {
        auto hit_info = scene.intersect(ray);
        if (hit_info.has_value()) {
          color += beta * hit_info->material->emissive;
          RGB t(color);
          // printf("init emissive -> %4.2f, %4.2f, %4.2f\n", scene.instances[0].material.emissive.r, scene.instances[0].material.emissive.g, scene.instances[0].material.emissive.b);
          // printf("beta -> %4.2f, %4.2f, %4.2f\n", beta.r, beta.g, beta.b);
          // printf("emissive -> %4.2f, %4.2f, %4.2f\n", hit_info->material->emissive.r, hit_info->material->emissive.g, hit_info->material->emissive.b);
          // printf("color -> %3d, %3d, %3d\n", t.r, t.g, t.b);
          beta *= hit_info->material->albedo;

          ray.origin = hit_info->hit_point;
          Frame frame(hit_info->normal);
          glm::vec3 light_direction;
          if (hit_info->material->is_specular) {
            glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
            light_direction = {-view_direction.x,
                               view_direction.y,
                               -view_direction.z};

          } else {
            do {
              light_direction = {uniform(gen),
                                 uniform(gen),
                                 uniform(gen)};
            } while (glm::length(light_direction) > 1.f);
            if (light_direction.y < 0) {
              light_direction.y = -light_direction.y;
            }
          }
          ray.direction = frame.worldFromLocal(light_direction);
        } else {
          break;
        }
      }
      // printf("init emissive -> %4.2f, %4.2f, %4.2f\n", scene.instances[0].material.emissive.r, scene.instances[0].material.emissive.g, scene.instances[0].material.emissive.b);
      film.addSample(x, y, color);
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