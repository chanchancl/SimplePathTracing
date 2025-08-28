
#include <array>
#include <iostream>

#include "accelerate/bounds.hpp"
#include "accelerate/bvh.hpp"
#include "utils/debug_marco.hpp"

void BVH::build(std::vector<Triangle> &&triangles) {
  auto *root = allocator.allocator();
  root->triangles = std::move(triangles);
  root->updateBounds();
  root->depth = 1;
  BVHState state{};
  size_t triangle_count = root->triangles.size();

  recusiveSplit(root, state);

  std::cout << "Total node count : " << state.total_node_count << std::endl;
  std::cout << "Leaf node count : " << state.leaf_node_count << std::endl;
  std::cout << "Triangle count: : " << triangle_count << std::endl;
  std::cout << "Mean leaf Node Triangle count : " << static_cast<float>(triangle_count) / static_cast<float>(state.leaf_node_count) << std::endl;
  std::cout << "Max leaf Node triangle count : " << state.max_leaf_node_triangle_count << std::endl;
  std::cout << "Max leaf Node Depth : " << state.max_leaf_node_depth << std::endl;

  nodes.reserve(state.total_node_count + 1);
  ordered_triangles.reserve(triangle_count);
  recusiveFlatten(root);
}

void BVH::recusiveSplit(BVHTreeNode *node, BVHState &state) {
  state.total_node_count++;
  if (node->triangles.size() == 1 || node->depth >= 32) {
    state.addLeafNode(node);
    return;
  }

  auto diag = node->bounds.diagonal();
  float min_cost = std::numeric_limits<float>::infinity();
  size_t min_split_bucket = 0;
  Bounds min_child0_bounds{}, min_child1_bounds{};
  size_t min_child0_triangle_count{}, min_child1_triangle_count{};

  constexpr size_t bucket_count = 12;
  std::vector<size_t> triangle_indices_buckets[3][bucket_count] = {};

  for (size_t axis = 0; axis < 3; axis++) {
    Bounds bounds_buckets[bucket_count] = {};
    size_t triangle_count[bucket_count] = {};
    size_t triangle_idx = 0;
    for (const auto &triangle : node->triangles) {
      // 将node->bounds 分为12份，找出 triangle 在 axis 轴上算在哪一份
      // (center - min) / (max - min) * 12
      auto triangle_center = triangle.centerOnAxis(axis);
      size_t bucket_idx = glm::clamp<size_t>(
        glm::floor((triangle_center - node->bounds.b_min[axis]) * bucket_count / diag[axis]),
        0, bucket_count - 1);
      bounds_buckets[bucket_idx].expand(triangle.p0);
      bounds_buckets[bucket_idx].expand(triangle.p1);
      bounds_buckets[bucket_idx].expand(triangle.p2);
      triangle_count[bucket_idx]++;
      // 在 axis 轴上将 triangle 划分到 bucket_idx 这个桶里
      triangle_indices_buckets[axis][bucket_idx].push_back(triangle_idx);
      triangle_idx++;
    }

    Bounds left_bounds = bounds_buckets[0];
    size_t left_triangle_count = triangle_count[0];
    // 将 12个bucket，依次分为 1,11 . 2, 10, 3, 9 ... 11, 1
    // 左右两份，计算这个划分下的cost，并记录最小cost的划分
    for (size_t i = 1; i < bucket_count; i++) {
      Bounds right_bounds{};
      size_t right_triangle_count = 0;
      for (size_t j = bucket_count - 1; j >= i; j--) {
        right_bounds.expand(bounds_buckets[j]);
        right_triangle_count += triangle_count[j];
      }
      if (right_triangle_count == 0) {
        break;
      }
      if (left_triangle_count != 0) {
        float cost = left_bounds.area() * left_triangle_count + right_bounds.area() * right_triangle_count;
        if (cost < min_cost) {
          min_cost = cost;
          node->split_axis = axis;
          min_split_bucket = i;
          min_child0_bounds = left_bounds;
          min_child1_bounds = right_bounds;
          min_child0_triangle_count = left_triangle_count;
          min_child1_triangle_count = right_triangle_count;
        }
      }
      left_bounds.expand(bounds_buckets[i]);
      left_triangle_count += triangle_count[i];
    }
  }

  if (min_split_bucket == 0) {
    state.addLeafNode(node);
    return;
  }

  auto *child0 = allocator.allocator();
  auto *child1 = allocator.allocator();

  node->children[0] = child0;
  node->children[1] = child1;

  child0->triangles.reserve(min_child0_triangle_count + 1);
  child1->triangles.reserve(min_child1_triangle_count + 1);
  for (size_t i = 0; i < min_split_bucket; i++) {
    for (size_t idx : triangle_indices_buckets[node->split_axis][i]) {
      child0->triangles.push_back(node->triangles[idx]);
    }
  }
  for (size_t i = min_split_bucket; i < bucket_count; i++) {
    for (size_t idx : triangle_indices_buckets[node->split_axis][i]) {
      child1->triangles.push_back(node->triangles[idx]);
    }
  }

  child0->depth = child1->depth = node->depth + 1;

  node->triangles.clear();
  node->triangles.shrink_to_fit();

  child0->bounds = min_child0_bounds;
  child1->bounds = min_child1_bounds;

  recusiveSplit(child0, state);
  recusiveSplit(child1, state);
}

size_t BVH::recusiveFlatten(BVHTreeNode *node) {
  BVHNode bvh_node{
    node->bounds,
    {0},
    static_cast<uint16_t>(node->triangles.size()),
    static_cast<uint8_t>(node->split_axis)};
  auto idx = nodes.size();
  nodes.push_back(bvh_node);
  if (bvh_node.triangle_count == 0) {
    recusiveFlatten(node->children[0]);
    nodes[idx].child1_index = recusiveFlatten(node->children[1]);
  } else {
    nodes[idx].triangle_index = ordered_triangles.size();
    // TODO
    for (const auto &triangle : node->triangles) {
      ordered_triangles.push_back(triangle);
    }
  }
  return idx;
}

std::optional<HitInfo> BVH::intersect(const Ray &ray, float t_min, float t_max) const {
  std::optional<HitInfo> cloest_hit_info;

  DEBUG_LINE(size_t bounds_test_count = 0, triangle_test_count = 0)

  glm::vec3 inv_direction = 1.f / ray.direction;

  glm::bvec3 dir_is_neg = {
    ray.direction.x < 0,
    ray.direction.y < 0,
    ray.direction.z < 0,
  };

  std::array<int, 32> stack;
  auto ptr = stack.begin();
  size_t current_node_index = 0;

  while (true) {
    auto &node = nodes[current_node_index];

    DEBUG_LINE(bounds_test_count++)

    if (!node.bounds.hasIntersection(ray, inv_direction, t_min, t_max)) {
      if (ptr == stack.begin())
        break;
      current_node_index = *(--ptr);
      continue;
    }

    if (node.triangle_count == 0) {
      if (dir_is_neg[node.split_axis]) {
        *(ptr++) = current_node_index + 1;
        current_node_index = node.child1_index;
      } else {
        current_node_index += 1;
        *(ptr++) = node.child1_index;
      }
    } else {
      auto triangle_it = ordered_triangles.begin() + node.triangle_index;

      DEBUG_LINE(triangle_test_count++)

      for (size_t i = 0; i < node.triangle_count; i++) {
        auto hit_info = triangle_it->intersect(ray, t_min, t_max);
        ++triangle_it;
        if (hit_info.has_value()) {
          t_max = hit_info->t;
          cloest_hit_info = hit_info;
        }
      }
      if (ptr == stack.begin())
        break;
      current_node_index = *(--ptr);
    }
  }

  DEBUG_LINE(ray.bounds_test_count = bounds_test_count)
  DEBUG_LINE(ray.triangle_test_count = triangle_test_count)

  return cloest_hit_info;
}
