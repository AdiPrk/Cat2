#include <PCH/pch.h>
#include "GltfLoader.h"

#include "stb_image.h"

namespace Dog
{
    bool GltfLoader::LoadFromFile(ModelLoadData& outData, const std::string& filePath)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        std::string ext = std::filesystem::path(filePath).extension().string();
        bool success = (ext == ".glb") ?
            loader.LoadBinaryFromFile(&model, &err, &warn, filePath) :
            loader.LoadASCIIFromFile(&model, &err, &warn, filePath);

        if (!warn.empty()) {
            DOG_WARN("glTF Warning: {}", warn);
        }
        if (!err.empty()) {
            DOG_CRITICAL("glTF Error: {}", err);
        }
        if (!success) {
            DOG_CRITICAL("Failed to load glTF file: {}", filePath);
            return false;
        }

        outData.hasAnimations = !model.animations.empty();
        std::string baseDir = std::filesystem::path(filePath).parent_path().string();

        // Process all scenes and their nodes
        for (const auto& scene : model.scenes) {
            for (int nodeIndex : scene.nodes) {
                ProcessNode(outData, model, model.nodes[nodeIndex], baseDir);
            }
        }

        return true;
    }

    void GltfLoader::ProcessNode(ModelLoadData& outData, const tinygltf::Model& model, const tinygltf::Node& node, const std::string& baseDir)
    {
        // Process the mesh in the current node
        if (node.mesh > -1) {
            ProcessMesh(outData, model, model.meshes[node.mesh], node.skin, baseDir);
        }

        // Recursively process all children of this node
        for (int childIndex : node.children) {
            ProcessNode(outData, model, model.nodes[childIndex], baseDir);
        }
    }

    void GltfLoader::ProcessMesh(ModelLoadData& outData, const tinygltf::Model& model, const tinygltf::Mesh& mesh, int skinIndex, const std::string& baseDir)
    {
        for (const auto& primitive : mesh.primitives)
        {
            MeshData meshData;
            size_t vertexCount = 0;

            // --- Vertices ---
            if (primitive.attributes.count("POSITION")) {
                auto [positions, count] = GetAccessorData<glm::vec3>(model, primitive.attributes.at("POSITION"));
                vertexCount = count;
                meshData.vertices.resize(vertexCount);
                for (size_t i = 0; i < vertexCount; ++i) {
                    meshData.vertices[i].position = positions[i];
                    outData.aabbMin = glm::min(outData.aabbMin, positions[i]);
                    outData.aabbMax = glm::max(outData.aabbMax, positions[i]);
                }
            }

            // --- Normals ---
            if (primitive.attributes.count("NORMAL")) {
                auto [normals, count] = GetAccessorData<glm::vec3>(model, primitive.attributes.at("NORMAL"));
                for (size_t i = 0; i < count; ++i) meshData.vertices[i].normal = normals[i];
            }

            // --- UVs ---
            if (primitive.attributes.count("TEXCOORD_0")) {
                auto [uvs, count] = GetAccessorData<glm::vec2>(model, primitive.attributes.at("TEXCOORD_0"));
                for (size_t i = 0; i < count; ++i) meshData.vertices[i].uv = uvs[i];
            }

            // --- Colors ---
            if (primitive.attributes.count("COLOR_0")) {
                auto [colors, count] = GetAccessorData<glm::vec3>(model, primitive.attributes.at("COLOR_0"));
                for (size_t i = 0; i < count; ++i) meshData.vertices[i].color = colors[i];
            }

            // --- Indices ---
            if (primitive.indices >= 0)
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const uint8_t* data = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                meshData.indices.resize(accessor.count);

                switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                    const auto* buf = reinterpret_cast<const uint32_t*>(data);
                    for (size_t i = 0; i < accessor.count; ++i) meshData.indices[i] = buf[i];
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                    const auto* buf = reinterpret_cast<const uint16_t*>(data);
                    for (size_t i = 0; i < accessor.count; ++i) meshData.indices[i] = buf[i];
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                    const auto* buf = reinterpret_cast<const uint8_t*>(data);
                    for (size_t i = 0; i < accessor.count; ++i) meshData.indices[i] = buf[i];
                    break;
                }
                }
            }

            if (primitive.material >= 0) {
                ProcessMaterial(meshData.material, model, model.materials[primitive.material], baseDir);
            }
            // TODO: Process Materials and Skinning here, populating meshData.

            outData.meshes.emplace_back(std::move(meshData));
        }
    }

    void GltfLoader::ProcessMaterial(MaterialData& outMaterial, const tinygltf::Model& model, const tinygltf::Material& material, const std::string& baseDir)
    {
        const auto& pbr = material.pbrMetallicRoughness;

        if (pbr.baseColorTexture.index < 0) return;

        const tinygltf::Texture& texture = model.textures[pbr.baseColorTexture.index];
        if (texture.source < 0) return;

        const tinygltf::Image& image = model.images[texture.source];

        if (!image.uri.empty())
        {
            // Texture is an external file
            outMaterial.diffuseTexturePath = baseDir + "/" + image.uri;
        }
        else if (!image.image.empty())
        {
            size_t expected_size = image.width * image.height * image.component;

            if (image.image.size() == expected_size)
            {
                // The data is already decoded raw pixel data. No need for stbi_load.
                outMaterial.diffuseTextureData = image.image;
                outMaterial.diffuseWidth = image.width;
                outMaterial.diffuseHeight = image.height;
                outMaterial.diffuseTextureLoaded = true;
            }
            else
            {
                outMaterial.diffuseTextureData = image.image;
                outMaterial.diffuseWidth = image.width;
                outMaterial.diffuseHeight = image.height;
            }
        }
    }


    // Template helper to safely get a typed pointer and count from a glTF accessor.
    template<typename T>
    std::pair<const T*, size_t> GltfLoader::GetAccessorData(const tinygltf::Model& model, int accessorIndex)
    {
        const auto& accessor = model.accessors[accessorIndex];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];
        const T* data = reinterpret_cast<const T*>(buffer.data.data() + bufferView.byteOffset + accessor.byteOffset);
        return { data, accessor.count };
    }
}