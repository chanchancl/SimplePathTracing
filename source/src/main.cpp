
#include <cstdio>

#include "glm/glm.hpp"

#include "camera/camera.hpp"
#include "camera/film.hpp"
#include "renderer/debug_renderer.hpp"
#include "renderer/normal_render.hpp"
#include "renderer/sample_rt_renderer.hpp"
#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"
#include "shape/sphere.hpp"
#include "utils/rgb.hpp"
#include "utils/rng.hpp"

int main() {
  Model model("models/dragon_871k.obj");
  Sphere sphere{{0, 0, 0}, 1.f};
  Plane plane{{0, 0, 0}, {0, 1, 0}};

  Scene scene{};
  RNG rng{1234};
  for (int i = 0; i < 10000; i++) {
    glm::vec3 random_pos{
      rng.uniform() * 100 - 50,
      rng.uniform() * 2,
      rng.uniform() * 100 - 50};
    float u = rng.uniform();
    if (u < 0.9) {
      scene.addShape(
        model,
        {RGB(202, 150, 117), rng.uniform() > 0.5},
        random_pos,
        {1, 1, 1},
        {rng.uniform() * 360, rng.uniform() * 360, rng.uniform() * 360});
    } else if (u < 0.95) {
      scene.addShape(
        sphere,
        {{rng.uniform(), rng.uniform(), rng.uniform()}, true},
        random_pos,
        {0.4, 0.4, 0.4});
    } else {
      random_pos.y += 6;
      scene.addShape(
        sphere,
        {{1, 1, 1}, false, {rng.uniform() * 4, rng.uniform() * 4, rng.uniform() * 4}},
        random_pos);
    }
  }

  scene.addShape(
    plane,
    {RGB(128, 204, 157)},
    {0, -0.5, 0});

  scene.build();

  Film film(192 * 4, 108 * 4);
  Camera camera{
    film,
    {-12, 5, -12},
    {0, 0, 0},
    45};

  printf("%lld, %lld\n", film.getWidth(), film.getHeight());

  NormalRenderer normalRender(camera, scene);
  normalRender.render(1, "normal.ppm");

  BoundsTestCountRenderer btc_renderer{camera, scene};
  btc_renderer.render(1, "BTC.ppm");
  TriangleTestCountRenderer ttc_renderer{camera, scene};
  ttc_renderer.render(1, "TTC.ppm");

  SimpleRTRenderer sampleRTRender(camera, scene);
  sampleRTRender.render(128, "simpleRT.ppm");

  return 0;
}