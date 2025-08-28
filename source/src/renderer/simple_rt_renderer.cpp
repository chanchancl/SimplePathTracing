

#include <mutex>

#include "renderer/sample_rt_renderer.hpp"
#include "utils/frame.hpp"

std::mutex logm;

glm::vec3 SimpleRTRenderer::renderPixel(const glm::ivec2 &pixel_coord) {
  auto ray = camera.generateRay(pixel_coord, {rng.uniform(), rng.uniform()});
  glm::vec3 beta = {1, 1, 1};
  glm::vec3 color = {0, 0, 0};
  size_t max_bounce_count = 32;

  while (max_bounce_count--) {
    auto hit_info = scene.intersect(ray);
    if (hit_info.has_value()) {
      color += beta * hit_info->material->emissive;
      beta *= hit_info->material->albedo;

      ray.origin = hit_info->hit_point;

      Frame frame(hit_info->normal);
      glm::vec3 light_direction;
      if (hit_info->material->is_specular) {
        glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
        light_direction = {-view_direction.x, view_direction.y, -view_direction.z};
      } else {
        do {
          light_direction = {rng.uniform(), rng.uniform(), rng.uniform()};
          light_direction = light_direction * 2.f - 1.f;
        } while (glm::length(light_direction) > 1.f);
        if (light_direction.y < 0) {
          light_direction.y = -light_direction.y;
        }
      }
      ray.direction = frame.worldFromLocal(light_direction);
    } else {
      break;
    }
  }

  // if ((pixel_coord.x + pixel_coord.y) % 255 == 0)
  //   std::cout << pixel_coord.x << ", " << pixel_coord.y
  //             << " -> " << color.r << ", " << color.g << ", " << color.b << std::endl;

  return color;
}