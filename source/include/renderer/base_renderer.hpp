
#pragma once

#include "camera/camera.hpp"
#include "glm/fwd.hpp"
#include "shape/scene.hpp"
#include "thread/threadpool.hpp"
#include "utils/rng.hpp"
#include <filesystem>

#define DEFINE_RENDERER(Name)                                                           \
  class Name##Renderer : public BaseRenderer {                                          \
  public:                                                                               \
    Name##Renderer(Camera &camera, const Scene &scene) : BaseRenderer(camera, scene) {} \
                                                                                        \
  private:                                                                              \
    virtual glm::vec3 renderPixel(const glm::ivec2 &pixel_coord) override;              \
  };

class BaseRenderer {
protected:
  Camera &camera;
  const Scene &scene;
  RNG rng{};

public:
  BaseRenderer(Camera &camera, const Scene &scene) : camera(camera), scene(scene) {}

  void render(size_t spp, const std::filesystem::path &filename);

private:
  virtual glm::vec3 renderPixel(const glm::ivec2 &pixel_coord) = 0;
};