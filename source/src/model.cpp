
#include "model.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ray.hpp"
#include "triangle.hpp"

Model::Model(const std::filesystem::path &filepath) {
  // v  p1 p2 p3
  // vn n1 n2 n3
  // f  p1/n1 p2/n2 p3/n3 (index)
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;

  std::ifstream file(filepath);
  if (!file.good()) {
    std::cout << "打开文件失败 : " << filepath << std::endl;
    return;
  }

  std::string line;
  char trash;
  while (!file.eof()) {
    std::getline(file, line);
    std::istringstream iss(line);

    if (line.compare(0, 2, "v ") == 0) {
      glm::vec3 pos{};
      iss >> trash >> pos.x >> pos.y >> pos.z;
      positions.push_back(pos);
    } else if (line.compare(0, 3, "vn ") == 0) {
      glm::vec3 normal{};
      iss >> trash >> trash >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    } else if (line.compare(0, 2, "f ") == 0) {
      glm::ivec3 idx_v, idx_vn;
      iss >> trash;
      iss >> idx_v.x >> trash >> trash >> idx_vn.x;
      iss >> idx_v.y >> trash >> trash >> idx_vn.y;
      iss >> idx_v.z >> trash >> trash >> idx_vn.z;
      triangles.push_back(
          Triangle(positions[idx_v.x - 1], positions[idx_v.y - 1],
                   positions[idx_v.z - 1], normals[idx_vn.x - 1],
                   normals[idx_vn.y - 1], normals[idx_vn.z - 1]));
    }
  }
}

std::optional<HitInfo> Model::intersect(const Ray &ray, float t_min,
                                        float t_max) const {
  std::optional<HitInfo> closest_hit_info{};
  for (const auto &triangle : triangles) {
    auto hit_info = triangle.intersect(ray, t_min, t_max);
    if (hit_info.has_value()) {
      t_max = hit_info->t;
      closest_hit_info = hit_info;
    }
  }
  return closest_hit_info;
};