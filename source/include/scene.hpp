#pragma once

#include <vector>

#include "shape.hpp"

struct ShapeInstance {
  const Shape *shape;
  glm::mat4 world_from_object;
  glm::mat4 object_from_world;
};

struct Scene : public Shape {
  std::vector<ShapeInstance> instances;

  virtual std::optional<HitInfo> intersect(
      const Ray &ray, float t_min = 1e-5,
      float t_max = std::numeric_limits<float>::infinity()) const override;

  void addShape(Shape *shape, const glm::vec3 &pos = {0, 0, 0},
                const glm::vec3 &scale = {1, 1, 1},
                const glm::vec3 &rotate = {0, 0, 0});
};