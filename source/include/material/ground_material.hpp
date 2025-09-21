#pragma once

#include "material/material.hpp"

class GroundMaterial : public Material {
  glm::vec3 albedo{};

public:
  GroundMaterial(const glm::vec3 &albedo) : albedo(albedo) {}
  virtual glm::vec3 sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, glm::vec3 &beta, const RNG &rng) const override;
};
