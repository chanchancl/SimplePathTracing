
#include <cstdio>
#include <glm/glm.hpp>
#include <iostream>

#include "camera/camera.hpp"
#include "camera/film.hpp"
#include "renderer/normal_render.hpp"
#include "renderer/sample_rt_renderer.hpp"
#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"
#include "shape/sphere.hpp"
#include "utils/progress.hpp"
#include "utils/rgb.hpp"
#include "utils/rng.hpp"

int main() {
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
  for (auto &inst : scene.instances) {
    auto &a = inst.material.albedo;
    printf("Instance : %.2f, %.2f, %.2f\n", a.r, a.g, a.b);
  }
  Progress progress(film.getWidth() * film.getHeight());
  RNG rng(23322233);
  printf("%lld, %lld\n", film.getWidth(), film.getHeight());

  NormalRenderer normalRender(camera, scene);
  normalRender.render(1, "normal.ppm");
  film.clean();

  SimpleRTRenderer sampleRTRender(camera, scene);
  sampleRTRender.render(64, "simpleRT.ppm");

  return 0;
}