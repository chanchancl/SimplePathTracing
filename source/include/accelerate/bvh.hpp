#pragma once

#include "bounds.hpp"
#include "shape/triangle.hpp"

struct BVHTreeNode {
  Bounds bounds;
  std::vector<Triangle> triangles;

  BVHTreeNode *children[2];
  size_t depth;
  size_t split_axis;

  void updateBounds() {
    bounds = {};
    for (const auto &triangle : triangles) {
      bounds.expand(triangle.getBounds());
    }
  }
};

struct alignas(32) BVHNode {
  Bounds bounds{};
  union {
    int child1_index;
    int triangle_index;
  };
  uint16_t triangle_count;
  uint8_t split_axis;
};

struct BVHTreeNodeAllocator {
  BVHTreeNodeAllocator() : ptr(4096) {}
  ~BVHTreeNodeAllocator() {
    for (auto &nodes : nodes_list) {
      delete[] nodes;
    }
    nodes_list.clear();
  }

  BVHTreeNode *allocator() {
    if (ptr == 4096) {
      nodes_list.push_back(new BVHTreeNode[4096]);
      ptr = 0;
    }
    return &nodes_list.back()[ptr++];
  }

private:
  size_t ptr;
  std::vector<BVHTreeNode *> nodes_list;
};
struct BVHState {
  size_t total_node_count{};
  size_t leaf_node_count{};
  size_t max_leaf_node_triangle_count{};
  size_t max_leaf_node_depth{};

  void addLeafNode(BVHTreeNode *node) {
    leaf_node_count++;
    max_leaf_node_triangle_count = glm::max(max_leaf_node_triangle_count, node->triangles.size());
    max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);
  }
};

class BVH : public Shape {
  std::vector<BVHNode> nodes;
  std::vector<Triangle> ordered_triangles;
  BVHTreeNodeAllocator allocator;

public:
  void build(std::vector<Triangle> &&triangles);

  virtual std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;

  Bounds getBounds() const override { return nodes[0].bounds; }

private:
  void recusiveSplit(BVHTreeNode *node, BVHState &state);
  size_t recusiveFlatten(BVHTreeNode *node);
};