#include <PCH/pch.h>
#include "ObjLoader.h"

#include <map>
#include <filesystem>

#include <tiny_obj_loader.h>

#include "stb_image.h"

namespace Dog
{
    // Custom comparator for std::map, as tinyobj::index_t doesn't have a default operator<.
    struct IndexComparator {
        bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const {
            if (a.vertex_index != b.vertex_index) return a.vertex_index < b.vertex_index;
            if (a.normal_index != b.normal_index) return a.normal_index < b.normal_index;
            if (a.texcoord_index != b.texcoord_index) return a.texcoord_index < b.texcoord_index;
            return false;
        }
    };

    bool ObjLoader::LoadFromFile(ModelLoadData& outData, const std::string& filePath)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string baseDir = std::filesystem::path(filePath).parent_path().string();

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str(), baseDir.c_str())) {
            DOG_CRITICAL("tinyobjloader error: {}", err);
            return false;
        }

        if (!warn.empty()) {
            DOG_WARN("tinyobjloader warning: {}", warn);
        }

        // Process materials first
        std::vector<MaterialData> processedMaterials;
        for (const auto& mat : materials) {
            MaterialData materialData;
            ProcessMaterial(materialData, mat, baseDir);
            processedMaterials.push_back(materialData);
        }
        // Add a default material if none exist
        if (processedMaterials.empty()) {
            processedMaterials.push_back(MaterialData{});
        }


        // Loop over shapes (each shape is a mesh)
        for (const auto& shape : shapes) {
            MeshData meshData;
            std::map<tinyobj::index_t, uint32_t, IndexComparator> uniqueVertices{};

            // Loop over faces (triangles)
            for (const auto& index : shape.mesh.indices) {
                // If this combination of vertex/normal/uv is new, create a new Vertex.
                if (uniqueVertices.count(index) == 0) {
                    uniqueVertices[index] = static_cast<uint32_t>(meshData.vertices.size());

                    Vertex vertex{};

                    // Position
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    // Update AABB
                    outData.aabbMin = glm::min(outData.aabbMin, vertex.position);
                    outData.aabbMax = glm::max(outData.aabbMax, vertex.position);

                    // Normal (if present)
                    if (index.normal_index >= 0) {
                        vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        };
                    }

                    // UV (if present)
                    if (index.texcoord_index >= 0) {
                        vertex.uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            1.0f - attrib.texcoords[2 * index.texcoord_index + 1] // Flip V coordinate for Vulkan/DX
                        };
                    }

                    // Color (if present)
                    if (attrib.colors.size() > 3 * index.vertex_index) {
                        vertex.color = {
                           attrib.colors[3 * index.vertex_index + 0],
                           attrib.colors[3 * index.vertex_index + 1],
                           attrib.colors[3 * index.vertex_index + 2]
                        };
                    }

                    meshData.vertices.push_back(vertex);
                }
                meshData.indices.push_back(uniqueVertices[index]);
            }

            // Assign material. OBJ format is a bit loose here. We'll just use the first material ID.
            if (!shape.mesh.material_ids.empty() && !materials.empty()) {
                int materialId = shape.mesh.material_ids[0];
                if (materialId >= 0 && materialId < processedMaterials.size()) {
                    meshData.material = processedMaterials[materialId];
                }
                else {
                    meshData.material = processedMaterials[0]; // Default material
                }
            }
            else {
                meshData.material = processedMaterials[0]; // Default material
            }

            outData.meshes.emplace_back(std::move(meshData));
        }

        return true;
    }

    void ObjLoader::ProcessMaterial(MaterialData& outMaterial, const tinyobj::material_t& material, const std::string& baseDir)
    {
        if (!material.diffuse_texname.empty()) {
            outMaterial.diffuseTexturePath = baseDir + "/" + material.diffuse_texname;

            // Normalize path separators
            std::replace(outMaterial.diffuseTexturePath.begin(), outMaterial.diffuseTexturePath.end(), '\\', '/');
        }
    }
}
