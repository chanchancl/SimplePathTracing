#pragma once

#include "glm/glm.hpp"

#include "camera/ray.hpp"

struct Bounds {
  Bounds() : b_min(1e9), b_max(-1e9) {}
  Bounds(const glm::vec3 &b_min, const glm::vec3 &b_max)
      : b_min(b_min), b_max(b_max) {}

  void expand(const glm::vec3 &pos) {
    b_min = glm::min(pos, b_min);
    b_max = glm::max(pos, b_max);
  }

  void expand(const Bounds &other) {
    b_min = glm::min(b_min, other.b_min);
    b_max = glm::max(b_max, other.b_max);
  }

  bool hasIntersection(const Ray &ray, float t_min, float t_max) const;
  bool hasIntersection(const Ray &ray, const glm::vec3 &inv_direction, float t_min, float t_max) const;

  glm::vec3 diagonal() const { return b_max - b_min; }

  float area() const {
    auto diag = diagonal();
    return (diag.x * (diag.y + diag.z) + diag.y * diag.z) * 2.f;
  }

  glm::vec3 getCorner(size_t idx) const {
    auto corner = b_max;
    if ((idx & 0b001) == 0)
      corner.x = b_min.x;
    if ((idx & 0b010) == 0)
      corner.y = b_min.y;
    if ((idx & 0b100) == 0)
      corner.z = b_min.z;
    return corner;
  }

  bool isValid() const {
    return b_max.x >= b_min.x && b_max.y >= b_min.y && b_max.z >= b_min.z;
  }

  glm::vec3 b_min;
  glm::vec3 b_max;
};