#pragma once

#include "film.hpp"
#include "glm/fwd.hpp"
#include "ray.hpp"

class Camera {
private:
  Film &film;
  glm::vec3 pos;

  glm::mat4 camera_from_clip;
  glm::mat4 world_from_camera;

public:
  Camera(Film &film, const glm::vec3 &pos, const glm::vec3 &viewpoint,
         float fovy);

  Ray generateRay(const glm::ivec2 &pixel_coord,
                  const glm::vec2 &offset = {0.5f, 0.5f}) const;

  Film &getFilm() { return film; }
  const Film &getFilm() const { return film; }
};
