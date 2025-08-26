#pragma once

#include "shape/shape.hpp"

struct Plane : public Shape {
  glm::vec3 point;
  glm::vec3 normal;

  Plane(const glm::vec3 &point, const glm::vec3 normal)
      : point(point), normal(normal) {}

  virtual std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
};