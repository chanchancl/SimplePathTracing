
#include <iostream>

#include <glm/glm.hpp>

#include "camera/camera.hpp"
#include "camera/film.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "material/conductor_material.hpp"
#include "material/dielectric_material.hpp"
#include "material/diffuse_material.hpp"
#include "material/ground_material.hpp"
#include "material/specular_material.hpp"
#include "renderer/debug_renderer.hpp"
#include "renderer/normal_render.hpp"
#include "renderer/path_tracing_renderer.hpp"

// #include "renderer/sample_rt_renderer.hpp"
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
  for (int i = -3; i <= 3; i++) {
    scene.addShape(sphere,
                   new DielectricMaterial{1.f + 0.2f * (i + 3), {1, 1, 1}},
                   {0, 0.5, i * 2},
                   {0.8, 0.8, 0.8});
  }

  for (int i = -3; i <= 3; i++) {
    glm::vec3 c = RGB::GenerateHeatmapRGB((i + 3.f) / 6.f);
    scene.addShape(sphere,
                   new ConductorMaterial{
                     glm::vec3(2.f - c * 2.f),
                     glm::vec3(2.f + c * 3.f)},
                   {0, 2.5, i * 2},
                   {0.8, 0.8, 0.8});
  }

  scene.addShape(model,
                 new DielectricMaterial(1.5, RGB(245, 245, 245)),
                 {-5, 0.9, 2},
                 {4, 4, 4});

  scene.addShape(model,
                 new ConductorMaterial({0.1, 1.2, 1.8}, {5, 2.5, 2}),
                 {-5, 0.9, -2},
                 {4, 4, 4});

  // plane
  scene.addShape(plane, new GroundMaterial(RGB(128, 204, 157)), {0, -0.5, 0});

  auto *light_material = new DiffuseMaterial({1, 1, 1});
  light_material->setEmissive({0.95, 0.95, 1});
  scene.addShape(plane, light_material, {0, 10, 0});

  scene.build();

  // Film film(192 * 4, 108 * 4);
  Film film(1920, 1080);
  Camera camera{film, {-10, 3.8, 0}, {0, 0, 0}, 45};

  NormalRenderer normalRender(camera, scene);
  normalRender.render(1, "normal.ppm");

  BoundsTestCountRenderer btc_renderer{camera, scene};
  btc_renderer.render(1, "BTC.ppm");
  TriangleTestCountRenderer ttc_renderer{camera, scene};
  ttc_renderer.render(1, "TTC.ppm");

  // SimpleRTRenderer sampleRTRender(camera, scene);
  // sampleRTRender.render(128, "simpleRT.ppm");

  PathTracingRenderer path_tracing_RTRender(camera, scene);
  path_tracing_RTRender.render(128, "path_tracing.ppm");

  return 0;
}

void scene1(const RNG &rng, Scene &scene, Model &model, Sphere &sphere) {
  for (int i = 0; i < 10000; i++) {
    glm::vec3 random_pos{
      rng.uniform() * 100 - 50,
      rng.uniform() * 2,
      rng.uniform() * 100 - 50};
    float u = rng.uniform();
    if (u < 0.9) {
      Material *material;
      if (rng.uniform() < 0.5) {
        material = new SpecularMaterial{RGB(202, 150, 117)};
      } else {
        material = new DiffuseMaterial{RGB(202, 150, 117)};
      }
      scene.addShape(
        model,
        material,
        random_pos,
        {1, 1, 1},
        {rng.uniform() * 360, rng.uniform() * 360, rng.uniform() * 360});
    } else if (u < 0.95) {
      scene.addShape(
        sphere,
        new SpecularMaterial{{rng.uniform(), rng.uniform(), rng.uniform()}},
        random_pos,
        {0.4, 0.4, 0.4});
    } else {
      random_pos.y += 6;
      auto *material = new DiffuseMaterial{{1, 1, 1}};
      material->setEmissive({rng.uniform() * 4, rng.uniform() * 4, rng.uniform() * 4});
      scene.addShape(
        sphere,
        material,
        random_pos);
    }
  }
}