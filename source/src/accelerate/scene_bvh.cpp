
#include <array>
#include <iostream>

#include "accelerate/bounds.hpp"
#include "accelerate/scene_bvh.hpp"
#include "utils/debug_marco.hpp"

void SceneBVH::build(std::vector<ShapeInstance> &&instances) {
  auto *root = allocator.allocator();
  auto temp_instances = std::move(instances);
  std::cout << "Count of instances : " << temp_instances.size() << std::endl;
  for (auto &instance : temp_instances) {
    if (instance.shape.getBounds().isValid()) {
      instance.updateBounds();
      root->instances.push_back(instance);
    } else {
      infinity_instances.push_back(instance);
    }
  }
  root->updateBounds();
  root->depth = 1;
  SceneBVHState state{};
  size_t instance_count = root->instances.size();

  recusiveSplit(root, state);

  std::cout << "Total node count : " << state.total_node_count << std::endl;
  std::cout << "Leaf node count : " << state.leaf_node_count << std::endl;
  std::cout << "ShapeInstance count: : " << instance_count << std::endl;
  std::cout << "Mean leaf Node ShapeInstance count : " << static_cast<float>(instance_count) / static_cast<float>(state.leaf_node_count) << std::endl;
  std::cout << "Max leaf Node instance count : " << state.max_leaf_node_shape_count << std::endl;
  std::cout << "Max leaf Node Depth : " << state.max_leaf_node_depth << std::endl;

  nodes.reserve(state.total_node_count + 1);
  ordered_instances.reserve(instance_count);
  recusiveFlatten(root);
}

void SceneBVH::recusiveSplit(SceneBVHTreeNode *node, SceneBVHState &state) {
  state.total_node_count++;
  if (node->instances.size() == 1 || node->depth >= 32) {
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
    for (const auto &instance : node->instances) {
      size_t bucket_idx = glm::clamp<size_t>(
        glm::floor((instance.center[axis] - node->bounds.b_min[axis]) * bucket_count / diag[axis]),
        0, bucket_count - 1);
      bounds_buckets[bucket_idx].expand(instance.bounds);
      triangle_count[bucket_idx]++;
      triangle_indices_buckets[axis][bucket_idx].push_back(triangle_idx);
      triangle_idx++;
    }

    Bounds left_bounds = bounds_buckets[0];
    size_t left_triangle_count = triangle_count[0];
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

  child0->instances.reserve(min_child0_triangle_count + 1);
  child1->instances.reserve(min_child1_triangle_count + 1);
  for (size_t i = 0; i < min_split_bucket; i++) {
    for (size_t idx : triangle_indices_buckets[node->split_axis][i]) {
      child0->instances.push_back(node->instances[idx]);
    }
  }
  for (size_t i = min_split_bucket; i < bucket_count; i++) {
    for (size_t idx : triangle_indices_buckets[node->split_axis][i]) {
      child1->instances.push_back(node->instances[idx]);
    }
  }

  child0->depth = child1->depth = node->depth + 1;

  node->instances.clear();
  node->instances.shrink_to_fit();

  child0->bounds = min_child0_bounds;
  child1->bounds = min_child1_bounds;

  recusiveSplit(child0, state);
  recusiveSplit(child1, state);
}

size_t SceneBVH::recusiveFlatten(SceneBVHTreeNode *node) {
  SceneBVHNode bvh_node{
    node->bounds,
    {0},
    static_cast<uint16_t>(node->instances.size()),
    static_cast<uint8_t>(node->split_axis)};
  auto idx = nodes.size();
  nodes.push_back(bvh_node);
  if (bvh_node.instance_count == 0) {
    recusiveFlatten(node->children[0]);
    nodes[idx].child1_index = recusiveFlatten(node->children[1]);
  } else {
    nodes[idx].instance_index = ordered_instances.size();
    // TODO
    for (const auto &instance : node->instances) {
      ordered_instances.push_back(instance);
    }
  }
  return idx;
}

std::optional<HitInfo> SceneBVH::intersect(const Ray &ray, float t_min, float t_max) const {
  std::optional<HitInfo> closest_hit_info;
  const ShapeInstance *closest_instance = nullptr;

  DEBUG_LINE(size_t bounds_test_count = 0)

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

    if (node.instance_count == 0) {
      if (dir_is_neg[node.split_axis]) {
        *(ptr++) = current_node_index + 1;
        current_node_index = node.child1_index;
      } else {
        current_node_index += 1;
        *(ptr++) = node.child1_index;
      }
    } else {
      auto instance_it = ordered_instances.begin() + node.instance_index;

      for (size_t i = 0; i < node.instance_count; i++) {
        auto ray_object = ray.objectFromWorld(instance_it->object_from_world);
        auto hit_info = instance_it->shape.intersect(ray_object, t_min, t_max);
        DEBUG_LINE(ray.bounds_test_count += ray_object.bounds_test_count)
        DEBUG_LINE(ray.triangle_test_count += ray_object.triangle_test_count)
        if (hit_info.has_value()) {
          t_max = hit_info->t;
          closest_hit_info = hit_info;
          closest_instance = &(*instance_it);
        }
        ++instance_it;
      }
      if (ptr == stack.begin())
        break;
      current_node_index = *(--ptr);
    }
  }

  for (const auto &infinity_instance : infinity_instances) {
    auto ray_object = ray.objectFromWorld(infinity_instance.object_from_world);
    auto hit_info = infinity_instance.shape.intersect(ray_object, t_min, t_max);
    DEBUG_LINE(ray.bounds_test_count += ray_object.bounds_test_count)
    DEBUG_LINE(ray.triangle_test_count += ray_object.triangle_test_count)
    if (hit_info.has_value()) {
      t_max = hit_info->t;
      closest_hit_info = hit_info;
      closest_instance = &infinity_instance;
    }
  }

  if (closest_instance) {
    closest_hit_info->hit_point = closest_instance->world_from_object * glm::vec4(closest_hit_info->hit_point, 1);
    // Mn = [M^-1]T
    // M    = world_from_object
    // M^-1 = object_from_world
    // Mn   = [object_from_world]T
    closest_hit_info->normal = glm::normalize(
      glm::vec3(glm::transpose(closest_instance->object_from_world) * glm::vec4(closest_hit_info->normal, 0)));
    closest_hit_info->material = closest_instance->material;
  }

  DEBUG_LINE(ray.bounds_test_count = bounds_test_count)

  return closest_hit_info;
}
