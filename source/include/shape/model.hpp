#pragma once

#include <filesystem>

#include "shape/shape.hpp"
#include "shape/triangle.hpp"

class Model : public Shape {
  std::vector<Triangle> triangles;

public:
  Model(const std::vector<Triangle> &triangles) : triangles(triangles) {}
  Model(const std::filesystem::path &filepath);

  virtual std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
};