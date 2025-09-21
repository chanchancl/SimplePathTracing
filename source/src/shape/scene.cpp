#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "camera/ray.hpp"
#include "shape/scene.hpp"

std::optional<HitInfo> Scene::intersect(const Ray &ray, float t_min, float t_max) const {
  return bvh.intersect(ray, t_min, t_max);
}

void Scene::addShape(Shape &shape, const Material *material, const glm::vec3 &pos, const glm::vec3 &scale, const glm::vec3 &rotate) {
  glm::mat4 world_from_obj =
    glm::translate(glm::mat4(1.f), pos) *
    glm::rotate(glm::mat4(1.f), glm::radians(rotate.z), {0, 0, 1}) *
    glm::rotate(glm::mat4(1.f), glm::radians(rotate.y), {0, 1, 0}) *
    glm::rotate(glm::mat4(1.f), glm::radians(rotate.x), {1, 0, 0}) *
    glm::scale(glm::mat4(1.f), scale);

  instances.push_back(ShapeInstance{shape, material, world_from_obj, glm::inverse(world_from_obj)});
}