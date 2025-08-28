#pragma once

#include <optional>

#include "camera/ray.hpp"
#include "shape/shape.hpp"

struct Sphere : public Shape {
  glm::vec3 center;
  float radius;

  Sphere(const glm::vec3 &center, float radius)
      : center(center), radius(radius) {}
  std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
  virtual Bounds getBounds() const override { return {center - radius, center + radius}; };
};