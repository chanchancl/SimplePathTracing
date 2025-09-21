#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "camera/camera.hpp"
#include "camera/ray.hpp"

Camera::Camera(Film &film, const glm::vec3 &pos, const glm::vec3 &viewpoint, float fovy)
    : film(film), pos(pos) {
  // 剪裁空间 到 相机空间, 相机位于 (0,0,0)
  camera_from_clip = glm::inverse(
    glm::perspective(glm::radians(fovy),
                     static_cast<float>(film.getWidth()) / static_cast<float>(film.getHeight()),
                     1.f,
                     2.f));
  // 相机空间到 世界空间
  world_from_camera = glm::inverse(glm::lookAt(pos, viewpoint, {0, 1, 0}));
}

Ray Camera::generateRay(const glm::ivec2 &pixel_coord,
                        const glm::vec2 &offset) const {
  glm::vec2 ndc = (glm::vec2(pixel_coord) + offset) / glm::vec2(film.getWidth(), film.getHeight());
  // ndc
  // x [ 0, width ] -> [ 0, 1]
  // y [ 0, height] -> [ 0, 1]
  // pc + offset, move pc with offset in the pixel
  // and then x / width, y / height

  ndc.y = 1.f - ndc.y;
  ndc = 2.f * ndc - 1.f;
  // reverse y
  // trnslate [0, 1] --> [-1, 1]

  glm::vec4 clip{ndc, 0, 1};

  glm::vec3 world = world_from_camera * camera_from_clip * clip;

  return Ray{pos, glm::normalize(world - pos)};
}