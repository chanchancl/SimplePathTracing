#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "fast_obj.h"
// #include "tiny_obj_loader.h"

#include "camera/ray.hpp"
#include "shape/model.hpp"
#include "shape/triangle.hpp"
#include "utils/profile.hpp"

Model::Model(const std::filesystem::path &filepath) {
  PROFILE("Loading model " + filepath.string());
  // v  p1 p2 p3
  // vn n1 n2 n3
  // f  p1/n1 p2/n2 p3/n3 (index)

  // // manual
  // std::vector<glm::vec3> positions;
  // std::vector<glm::vec3> normals;

  // std::ifstream file(filepath);
  // if (!file.good()) {
  //   std::cout << "打开文件失败 : " << filepath << std::endl;
  //   return;
  // }

  // std::string line;
  // char trash;
  // while (!file.eof()) {
  //   std::getline(file, line);
  //   std::istringstream iss(line);

  //   if (line.compare(0, 2, "v ") == 0) {
  //     glm::vec3 pos{};
  //     iss >> trash >> pos.x >> pos.y >> pos.z;
  //     positions.push_back(pos);
  //   } else if (line.compare(0, 3, "vn ") == 0) {
  //     glm::vec3 normal{};
  //     iss >> trash >> trash >> normal.x >> normal.y >> normal.z;
  //     normals.push_back(normal);
  //   } else if (line.compare(0, 2, "f ") == 0) {
  //     glm::ivec3 idx_v, idx_vn;
  //     iss >> trash;
  //     iss >> idx_v.x >> trash >> trash >> idx_vn.x;
  //     iss >> idx_v.y >> trash >> trash >> idx_vn.y;
  //     iss >> idx_v.z >> trash >> trash >> idx_vn.z;
  //     triangles.push_back(
  //       Triangle(positions[idx_v.x - 1], positions[idx_v.y - 1],
  //                positions[idx_v.z - 1], normals[idx_vn.x - 1],
  //                normals[idx_vn.y - 1], normals[idx_vn.z - 1]));
  //   }
  // }

  // // tinyobjloader
  // tinyobj::attrib_t attrib;
  // std::vector<tinyobj::shape_t> shapes;
  // std::vector<tinyobj::material_t> materials;

  // std::string warn;
  // std::string err;

  // bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.string().c_str());
  // if (!warn.empty()) {
  //   std::cout << warn << std::endl;
  // }

  // if (!err.empty()) {
  //   std::cerr << err << std::endl;
  // }

  // if (!ret) {
  //   std::cout << "打开文件失败 : " << filepath << std::endl;
  //   return;
  // }

  // std::vector<Triangle> triangles;
  // for (const auto &shape : shapes) {
  //   size_t index_offset = 0;
  //   for (size_t num_face_vectex : shape.mesh.num_face_vertices) {
  //     if (num_face_vectex == 3) {
  //       auto &positions = attrib.vertices;
  //       auto index = shape.mesh.indices[index_offset];
  //       glm::vec3 p0{
  //         positions[index.vertex_index * 3 + 0],
  //         positions[index.vertex_index * 3 + 1],
  //         positions[index.vertex_index * 3 + 2]};

  //       index = shape.mesh.indices[index_offset + 1];
  //       glm::vec3 p1{
  //         positions[index.vertex_index * 3 + 0],
  //         positions[index.vertex_index * 3 + 1],
  //         positions[index.vertex_index * 3 + 2]};

  //       index = shape.mesh.indices[index_offset + 2];
  //       glm::vec3 p2{
  //         positions[index.vertex_index * 3 + 0],
  //         positions[index.vertex_index * 3 + 1],
  //         positions[index.vertex_index * 3 + 2]};

  //       if (index.normal_index >= 0) {
  //         auto &normals = attrib.normals;
  //         index = shape.mesh.indices[index_offset];
  //         glm::vec3 n0{
  //           normals[index.normal_index * 3 + 0],
  //           normals[index.normal_index * 3 + 1],
  //           normals[index.normal_index * 3 + 2]};

  //         index = shape.mesh.indices[index_offset + 1];
  //         glm::vec3 n1{
  //           normals[index.normal_index * 3 + 0],
  //           normals[index.normal_index * 3 + 1],
  //           normals[index.normal_index * 3 + 2]};

  //         index = shape.mesh.indices[index_offset + 2];
  //         glm::vec3 n2{
  //           normals[index.normal_index * 3 + 0],
  //           normals[index.normal_index * 3 + 1],
  //           normals[index.normal_index * 3 + 2]};
  //         triangles.push_back(Triangle{p0, p1, p2, n0, n1, n2});
  //       } else {
  //         triangles.push_back(Triangle{p0, p1, p2});
  //       }
  //     }
  //     index_offset += num_face_vectex;
  //   }
  // }

  // fast_obj
  std::vector<Triangle> triangles;
  fastObjMesh *mesh = fast_obj_read(filepath.string().c_str());

  for (size_t ig = 0; ig < mesh->group_count; ig++) {
    const auto &group = mesh->groups[ig];

    int idx = 0;
    for (size_t ifc = 0; ifc < group.face_count; ifc++) {
      size_t fc = mesh->face_vertices[group.face_offset + ifc];
      if (fc == 3) {
        auto objIndex = mesh->indices[group.index_offset + idx];

        glm::vec3 p0{
          mesh->positions[3 * objIndex.p + 0],
          mesh->positions[3 * objIndex.p + 1],
          mesh->positions[3 * objIndex.p + 2],
        };
        objIndex = mesh->indices[group.index_offset + idx + 1];
        glm::vec3 p1{
          mesh->positions[3 * objIndex.p + 0],
          mesh->positions[3 * objIndex.p + 1],
          mesh->positions[3 * objIndex.p + 2],
        };
        objIndex = mesh->indices[group.index_offset + idx + 2];
        glm::vec3 p2{
          mesh->positions[3 * objIndex.p + 0],
          mesh->positions[3 * objIndex.p + 1],
          mesh->positions[3 * objIndex.p + 2],
        };
        if (objIndex.n) {
          auto objIndex = mesh->indices[group.index_offset + idx];
          glm::vec3 n0{
            mesh->normals[3 * objIndex.n + 0],
            mesh->normals[3 * objIndex.n + 1],
            mesh->normals[3 * objIndex.n + 2]};
          objIndex = mesh->indices[group.index_offset + idx + 1];
          glm::vec3 n1{
            mesh->normals[3 * objIndex.n + 0],
            mesh->normals[3 * objIndex.n + 1],
            mesh->normals[3 * objIndex.n + 2]};
          objIndex = mesh->indices[group.index_offset + idx + 2];
          glm::vec3 n2{
            mesh->normals[3 * objIndex.n + 0],
            mesh->normals[3 * objIndex.n + 1],
            mesh->normals[3 * objIndex.n + 2]};
          triangles.push_back(Triangle{p0, p1, p2, n0, n1, n2});
        } else {
          triangles.push_back(Triangle{p0, p1, p2});
        }
      }

      idx += fc;
    }
  }

  fast_obj_destroy(mesh);

  // // rapidobj
  // auto result = rapidobj::ParseFile(filepath, rapidobj::MaterialLibrary::Ignore());

  // for (const auto &shape : result.shapes) {
  //   size_t index_offset = 0;
  //   for (size_t num_face_vectex : shape.mesh.num_face_vertices) {
  //     if (num_face_vectex == 3) {
  //       auto &positions = result.attributes.positions;
  //       auto index = shape.mesh.indices[index_offset];
  //       glm::vec3 p0{
  //         positions[index.position_index * 3 + 0],
  //         positions[index.position_index * 3 + 1],
  //         positions[index.position_index * 3 + 2]};

  //       index = shape.mesh.indices[index_offset + 1];
  //       glm::vec3 p1{
  //         positions[index.position_index * 3 + 0],
  //         positions[index.position_index * 3 + 1],
  //         positions[index.position_index * 3 + 2]};

  //       index = shape.mesh.indices[index_offset + 2];
  //       glm::vec3 p2{
  //         positions[index.position_index * 3 + 0],
  //         positions[index.position_index * 3 + 1],
  //         positions[index.position_index * 3 + 2]};

  //       if (index.normal_index >= 0) {
  //         auto &normals = result.attributes.normals;
  //         index = shape.mesh.indices[index_offset];
  //         glm::vec3 n0{
  //           normals[index.normal_index * 3 + 0],
  //           normals[index.normal_index * 3 + 1],
  //           normals[index.normal_index * 3 + 2]};

  //         index = shape.mesh.indices[index_offset + 1];
  //         glm::vec3 n1{
  //           normals[index.normal_index * 3 + 0],
  //           normals[index.normal_index * 3 + 1],
  //           normals[index.normal_index * 3 + 2]};

  //         index = shape.mesh.indices[index_offset + 2];
  //         glm::vec3 n2{
  //           normals[index.normal_index * 3 + 0],
  //           normals[index.normal_index * 3 + 1],
  //           normals[index.normal_index * 3 + 2]};
  //         triangles.push_back(Triangle{p0, p1, p2, n0, n1, n2});
  //       }
  //     }
  //     index_offset += num_face_vectex;
  //   }
  // }

  bvh.build(std::move(triangles));
}

std::optional<HitInfo> Model::intersect(const Ray &ray, float t_min, float t_max) const {
  return bvh.intersect(ray, t_min, t_max);
};
