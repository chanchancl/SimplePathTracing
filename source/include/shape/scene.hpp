#pragma once

#include <vector>

#include "accelerate/scene_bvh.hpp"
#include "shape/shape.hpp"

struct Scene : public Shape {
  std::vector<ShapeInstance> instances;
  SceneBVH bvh;

  virtual std::optional<HitInfo> intersect(
    const Ray &ray,
    float t_min = 1e-5,
    float t_max = std::numeric_limits<float>::infinity()) const override;

  void addShape(
    Shape &shape,
    const Material &material = {},
    const glm::vec3 &pos = {0, 0, 0},
    const glm::vec3 &scale = {1, 1, 1},
    const glm::vec3 &rotate = {0, 0, 0});

  void build() { bvh.build(std::move(instances)); }
};