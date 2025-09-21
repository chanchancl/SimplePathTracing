#pragma once

#include "material/material.hpp"

class DielectricMaterial : public Material {
  float ior{};
  glm::vec3 albedo_r{}, albedo_t{};

public:
  DielectricMaterial(float ior, const glm::vec3 &albedo)
      : DielectricMaterial(ior, albedo, albedo) {}
  DielectricMaterial(float ior, const glm::vec3 &albedo_r, const glm::vec3 &albedo_t)
      : ior(ior), albedo_r(albedo_r), albedo_t(albedo_t) {}
  virtual glm::vec3 sampleBSDF(const glm::vec3 &hit_point, const glm::vec3 &view_direction, glm::vec3 &beta, const RNG &rng) const override;
};
