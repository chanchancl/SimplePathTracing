#pragma once

#include <filesystem>

#include "accelerate/bounds.hpp"
#include "accelerate/bvh.hpp"
#include "shape/shape.hpp"
#include "shape/triangle.hpp"

class Model : public Shape {
  BVH bvh{};

public:
  Model(const std::vector<Triangle> &triangles) {
    auto copy = triangles;
    bvh.build(std::move(copy));
  }
  Model(const std::filesystem::path &filepath);

  virtual std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
  virtual Bounds getBounds() const override { return bvh.getBounds(); };
};