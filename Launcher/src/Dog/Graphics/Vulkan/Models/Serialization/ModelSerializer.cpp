/*****************************************************************//**
 * \file   ModelSerializer.cpp
 * \author Adi (aditya.prakash@digipen.edu)
 * \date   October 27 2024
 * \Copyright @ 2024 Digipen (USA) Corporation * 
 
 * \brief  Serializes model data to our custom model file format
 *  *********************************************************************/

/*********************************************************************
 * A little note: I don't have a computer that's big endian. I cannot test if that stuff really works or not :)
 *********************************************************************/

#include <PCH/pch.h>
#include "ModelSerializer.hpp"
#include "../../Animation/Animation.h"
#include "../../Animation/Animator.h"

namespace Dog {

  const std::string ModelSerializer::NL_MODEL_FILE_PATH = "assets/models/nlm/";
  const std::string ModelSerializer::NL_MODEL_EXTENTION = ".nlm";

  constexpr uint32_t ModelSerializer::swapEndian(uint32_t val) {
    return ((val >> 24) & 0xff) 
      | ((val << 8) & 0xff0000) 
      | ((val >> 8) & 0xff00) 
      | ((val << 24) & 0xff000000);
  }

  // Utility to swap endianness of a float
  float ModelSerializer::swapEndianFloat(float val) {
    if constexpr (switchEndian) {
      uint32_t temp;
      std::memcpy(&temp, &val, sizeof(float));
      temp = swapEndian(temp);
      std::memcpy(&val, &temp, sizeof(float));
    }
    return val;
  }

  // Utility to return a glm::vec2 with swapped endianness
  glm::vec2 ModelSerializer::swapEndianVec2(const glm::vec2& vec) {
    return glm::vec2(swapEndianFloat(vec.x), swapEndianFloat(vec.y));
  }

  // Utility to return a glm::vec3 with swapped endianness
  glm::vec3 ModelSerializer::swapEndianVec3(const glm::vec3& vec) {
    return glm::vec3(swapEndianFloat(vec.x), swapEndianFloat(vec.y), swapEndianFloat(vec.z));
  }

  // Utility to return a glm::mat4 with swapped endianness
  glm::mat4 ModelSerializer::swapEndianMat4(const glm::mat4& mat) {
    glm::mat4 result;
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) {
        result[i][j] = swapEndianFloat(mat[i][j]);
      }
    }
    return result;
  }

  bool ModelSerializer::validateHeader(std::ifstream& file) {
    uint32_t hash, magic, version;
    file.read(reinterpret_cast<char*>(&hash), sizeof(hash));
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    // Swap endianness if the system is big-endian
    if (switchEndian) {
      magic = swapEndian(magic);
      version = swapEndian(version);
    }

    if (magic != MAGIC_NUMBER || version != VERSION) {
      //NL_ERROR("Invalid file format or version.");
      return false;
    }

    return true;
  }

  void ModelSerializer::save(const Model& model, const std::string& filename, uint32_t hash) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) 
    {
      //NL_CRITICAL("Could not open file for writing.");
    }

    uint32_t magic = MAGIC_NUMBER;
    uint32_t version = VERSION;
    uint32_t checksum = 0; // Placeholder for future checksum implementation
    uint32_t hasAnimation = model.HasAnimations() ? 1 : 0;

    if constexpr (switchEndian) {
      magic = swapEndian(magic);
      version = swapEndian(version);
      checksum = swapEndian(checksum);
      hasAnimation = swapEndian(hasAnimation);
      hash = swapEndian(hash);
    }

    // Write header in little-endian format
    file.write(reinterpret_cast<const char*>(&hash), sizeof(hash));
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    // file.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));
    file.write(reinterpret_cast<const char*>(&hasAnimation), sizeof(hasAnimation));

    // Write AABB
    if constexpr (switchEndian) {
      glm::vec3 min = swapEndianVec3(model.mAABBmin);
      glm::vec3 max = swapEndianVec3(model.mAABBmax);
      file.write(reinterpret_cast<const char*>(&min), sizeof(min));
      file.write(reinterpret_cast<const char*>(&max), sizeof(max));
    }
    else {
      file.write(reinterpret_cast<const char*>(&model.mAABBmin), sizeof(model.mAABBmin));
      file.write(reinterpret_cast<const char*>(&model.mAABBmax), sizeof(model.mAABBmax));
    }

    // Write mesh count
    uint32_t meshCount = static_cast<uint32_t>(model.meshes.size());
    if constexpr (switchEndian) meshCount = swapEndian(meshCount);
    file.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));

    // Write each mesh
    uint32_t index = 0;
    for (const auto& mesh : model.meshes) {
      uint32_t vertexCount = static_cast<uint32_t>(mesh.vertices.size());
      uint32_t indexCount = static_cast<uint32_t>(mesh.indices.size());
      if constexpr (switchEndian) {
        vertexCount = swapEndian(vertexCount);
        indexCount = swapEndian(indexCount);
      }

      file.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
      file.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));

      // Write vertices
      for (const auto& vertex : mesh.vertices) {
        if constexpr (switchEndian)
        {
          glm::vec3 position = swapEndianVec3(vertex.position);
          glm::vec3 color = swapEndianVec3(vertex.color);
          glm::vec3 normal = swapEndianVec3(vertex.normal);
          glm::vec2 uv = swapEndianVec2(vertex.uv);
          glm::ivec4 boneIDs = glm::ivec4(
            swapEndian(vertex.boneIDs[0]), swapEndian(vertex.boneIDs[1]),
            swapEndian(vertex.boneIDs[2]), swapEndian(vertex.boneIDs[3]));
          glm::vec4 weights = glm::vec4(
            swapEndianFloat(vertex.weights[0]), swapEndianFloat(vertex.weights[1]),
            swapEndianFloat(vertex.weights[2]), swapEndianFloat(vertex.weights[3]));

          file.write(reinterpret_cast<const char*>(&position), sizeof(position));
          file.write(reinterpret_cast<const char*>(&color), sizeof(color));
          file.write(reinterpret_cast<const char*>(&normal), sizeof(normal));
          file.write(reinterpret_cast<const char*>(&uv), sizeof(uv));
          file.write(reinterpret_cast<const char*>(&boneIDs), sizeof(boneIDs));
          file.write(reinterpret_cast<const char*>(&weights), sizeof(weights));
        }
        else 
        {
          file.write(reinterpret_cast<const char*>(&vertex.position), sizeof(vertex.position));
          file.write(reinterpret_cast<const char*>(&vertex.color), sizeof(vertex.color));
          file.write(reinterpret_cast<const char*>(&vertex.normal), sizeof(vertex.normal));
          file.write(reinterpret_cast<const char*>(&vertex.uv), sizeof(vertex.uv));
          file.write(reinterpret_cast<const char*>(vertex.boneIDs.data()), sizeof(vertex.boneIDs));
          file.write(reinterpret_cast<const char*>(vertex.weights.data()), sizeof(vertex.weights));
        }
      }

      // Write indices as a contiguous block
      if constexpr (switchEndian) 
      {
        for (const auto& idx : mesh.indices) {
          uint32_t indexToWrite = swapEndian(idx);
          file.write(reinterpret_cast<const char*>(&indexToWrite), sizeof(indexToWrite));
        }
      }
      else
      {
        file.write(reinterpret_cast<const char*>(mesh.indices.data()), indexCount * sizeof(uint32_t));
      }

      if (!model.mSerializeData.embeddedTextures.empty())
      {
        auto& embedData = model.mSerializeData.embeddedTextures[index];
        uint32_t size = embedData.first;
        if constexpr (switchEndian) size = swapEndian(size);

        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(reinterpret_cast<const char*>(embedData.second.get()), size);
      }
      else
      {
        uint32_t size = 0;
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
      }

      ++index;
    }

    // Serialize animation data if present
    if (hasAnimation) {
      saveAnimationData(model, file);
    }

    file.close();
  }

  void ModelSerializer::saveAnimationData(const Model& model, std::ofstream& file)
  {
    const std::unique_ptr<Animation>& animation = model.GetAnimation();
    const std::unique_ptr<Animator>& animator = model.GetAnimator();

    // Write animation data
    float duration = animation->GetDuration();
    int ticksPerSecond = static_cast<int>(animation->GetTicksPerSecond());

    if constexpr (switchEndian) {
      duration = swapEndianFloat(duration);
      ticksPerSecond = swapEndian(ticksPerSecond);
    }

    file.write(reinterpret_cast<const char*>(&duration), sizeof(duration));
    file.write(reinterpret_cast<const char*>(&ticksPerSecond), sizeof(ticksPerSecond));

    // Write bone information map
    const auto& boneInfoMap = animation->GetBoneIDMap();
    uint32_t boneCount = static_cast<uint32_t>(boneInfoMap.size());
    if constexpr (switchEndian) boneCount = swapEndian(boneCount);
    file.write(reinterpret_cast<const char*>(&boneCount), sizeof(boneCount));

    for (const auto& [boneID, boneInfo] : boneInfoMap) {
      int id = boneID;
      glm::mat4 offset = boneInfo.offset;
      int boneInfoId = boneInfo.id;

      if constexpr (switchEndian) {
        id = static_cast<int>(swapEndian(static_cast<uint32_t>(id)));
        offset = swapEndianMat4(offset);
        boneInfoId = static_cast<int>(swapEndian(static_cast<uint32_t>(boneInfoId)));
      }

      file.write(reinterpret_cast<const char*>(&id), sizeof(id));
      file.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
      file.write(reinterpret_cast<const char*>(&boneInfoId), sizeof(boneInfoId));
    }

    // Write bone map (Animation keyframes)
    const auto& boneMap = animation->GetBoneMap();
    uint32_t boneMapSize = static_cast<uint32_t>(boneMap.size());
    if constexpr (switchEndian) boneMapSize = swapEndian(boneMapSize);
    file.write(reinterpret_cast<const char*>(&boneMapSize), sizeof(boneMapSize));

    for (const auto& [nodeID, bone] : boneMap) 
    {
      int nodeId = nodeID;
      int boneId = bone.GetBoneID();

      uint32_t posKeyCount = static_cast<uint32_t>(bone.GetNumPositionKeys());
      uint32_t rotKeyCount = static_cast<uint32_t>(bone.GetNumRotationKeys());
      uint32_t scaleKeyCount = static_cast<uint32_t>(bone.GetNumScalingKeys());

      if constexpr (switchEndian) {
        nodeId = static_cast<int>(swapEndian(static_cast<uint32_t>(nodeId)));
        boneId = static_cast<int>(swapEndian(static_cast<uint32_t>(boneId)));
        posKeyCount = swapEndian(posKeyCount);
        rotKeyCount = swapEndian(rotKeyCount);
        scaleKeyCount = swapEndian(scaleKeyCount);
      }

      file.write(reinterpret_cast<const char*>(&nodeId), sizeof(nodeId));
      file.write(reinterpret_cast<const char*>(&boneId), sizeof(boneId));
      file.write(reinterpret_cast<const char*>(&posKeyCount), sizeof(posKeyCount));
      file.write(reinterpret_cast<const char*>(&rotKeyCount), sizeof(rotKeyCount));
      file.write(reinterpret_cast<const char*>(&scaleKeyCount), sizeof(scaleKeyCount));

      // Write Position Keyframes
      for (const auto& key : bone.GetPositionKeys()) {
        float time = key.time;
        glm::vec3 position = key.position;
        if constexpr (switchEndian) {
          time = swapEndianFloat(time);
          position = swapEndianVec3(position);
        }
        file.write(reinterpret_cast<const char*>(&time), sizeof(time));
        file.write(reinterpret_cast<const char*>(&position), sizeof(position));
      }

      // Write Rotation Keyframes
      for (const auto& key : bone.GetRotationKeys()) {
        float time = key.time;
        glm::quat rotation = key.orientation;
        if constexpr (switchEndian) {
          time = swapEndianFloat(time);
          rotation = glm::quat(
            swapEndianFloat(rotation.w), swapEndianFloat(rotation.x),
            swapEndianFloat(rotation.y), swapEndianFloat(rotation.z));
        }
        file.write(reinterpret_cast<const char*>(&time), sizeof(time));
        file.write(reinterpret_cast<const char*>(&rotation), sizeof(rotation));
      }

      // Write Scaling Keyframes
      for (const auto& key : bone.GetScalingKeys()) {
        float time = key.time;
        glm::vec3 scale = key.scale;
        if constexpr (switchEndian) {
          time = swapEndianFloat(time);
          scale = swapEndianVec3(scale);
        }
        file.write(reinterpret_cast<const char*>(&time), sizeof(time));
        file.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
      }
    }

    // Animation Heirarchy
    const auto& nodes = animation->GetNodes();
    uint32_t nodeCount = static_cast<uint32_t>(nodes.size());
    if constexpr (switchEndian) nodeCount = swapEndian(nodeCount);
    file.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount));

    for (const auto& node : nodes) {
      int id = node.id;
      glm::mat4 transformation = node.transformation;
      uint32_t childCount = static_cast<uint32_t>(node.childIndices.size());
      std::vector<int> children = node.childIndices;

      if constexpr (switchEndian) {
        id = static_cast<int>(swapEndian(static_cast<uint32_t>(id)));
        transformation = swapEndianMat4(transformation);
        childCount = swapEndian(childCount);
        for (auto& child : children) {
          child = static_cast<int>(swapEndian(static_cast<uint32_t>(child)));
        }
      }

      file.write(reinterpret_cast<const char*>(&id), sizeof(id));
      file.write(reinterpret_cast<const char*>(&transformation), sizeof(transformation));
      file.write(reinterpret_cast<const char*>(&childCount), sizeof(childCount));
      file.write(reinterpret_cast<const char*>(children.data()), childCount * sizeof(int));
    }
  }

  bool ModelSerializer::load(Model& model, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) 
    {
      //NL_CRITICAL("Could not open file for reading.");
    }

    // Validate the file header and version
    if (!validateHeader(file))
    {
      file.close();
      return false;
    }

    uint32_t hasAnimation;
    file.read(reinterpret_cast<char*>(&hasAnimation), sizeof(hasAnimation));
    if constexpr (switchEndian) hasAnimation = swapEndian(hasAnimation);

    // Clear meshes
    model.meshes.clear();

    // Read AABB
    file.read(reinterpret_cast<char*>(&model.mAABBmin), sizeof(model.mAABBmin));
    file.read(reinterpret_cast<char*>(&model.mAABBmax), sizeof(model.mAABBmax));

    if constexpr (switchEndian) {
      model.mAABBmin = swapEndianVec3(model.mAABBmin);
      model.mAABBmax = swapEndianVec3(model.mAABBmax);
    }

    // Read mesh count
    uint32_t meshCount;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
    if constexpr (switchEndian) meshCount = swapEndian(meshCount);

    model.meshes.resize(meshCount);

    // Read each mesh
    for (auto& mesh : model.meshes) {
      uint32_t vertexCount, indexCount;

      // Read vertex and index counts
      file.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
      file.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
      if constexpr (switchEndian) {
        vertexCount = swapEndian(vertexCount);
        indexCount = swapEndian(indexCount);
      }

      mesh.vertices.resize(vertexCount);
      mesh.indices.resize(indexCount);

      // Read vertices
      for (auto& vertex : mesh.vertices) {
        file.read(reinterpret_cast<char*>(&vertex.position), sizeof(vertex.position));
        file.read(reinterpret_cast<char*>(&vertex.color), sizeof(vertex.color));
        file.read(reinterpret_cast<char*>(&vertex.normal), sizeof(vertex.normal));
        file.read(reinterpret_cast<char*>(&vertex.uv), sizeof(vertex.uv));
        file.read(reinterpret_cast<char*>(vertex.boneIDs.data()), sizeof(vertex.boneIDs));
        file.read(reinterpret_cast<char*>(vertex.weights.data()), sizeof(vertex.weights));

        if constexpr (switchEndian) {
          vertex.position = swapEndianVec3(vertex.position);
          vertex.color = swapEndianVec3(vertex.color);
          vertex.normal = swapEndianVec3(vertex.normal);
          vertex.uv = swapEndianVec2(vertex.uv);
          for (auto& id : vertex.boneIDs) {
            id = static_cast<int>(swapEndian(static_cast<uint32_t>(id)));
          }
          for (auto& weight : vertex.weights) {
            weight = swapEndianFloat(weight);
          }
        }
      }

      // Read indices
      file.read(reinterpret_cast<char*>(mesh.indices.data()), indexCount * sizeof(uint32_t));

      // Swap endianness of indices if necessary
      if constexpr (switchEndian) {
        for (auto& index : mesh.indices) {
          index = swapEndian(index);
        }
      }

      // Read embedded texture data
      uint32_t size;
      file.read(reinterpret_cast<char*>(&size), sizeof(size));
      if constexpr (switchEndian) size = swapEndian(size);

      if (size > 0) {
        auto data = std::make_unique<unsigned char[]>(size);
        file.read(reinterpret_cast<char*>(data.get()), size);
        model.mSerializeData.embeddedTextures.push_back({ size, std::move(data) });
      }
      else {
        model.mSerializeData.embeddedTextures.push_back({ 0, nullptr });
      }
    }

    if (hasAnimation)
    {
      loadAnimationData(model, file);
    }

    file.close();

    return true;
  }

  void ModelSerializer::loadAnimationData(Model& model, std::ifstream& file)
  {
    // Read animation data
    float duration;
    int ticksPerSecond;

    file.read(reinterpret_cast<char*>(&duration), sizeof(duration));
    file.read(reinterpret_cast<char*>(&ticksPerSecond), sizeof(ticksPerSecond));

    if constexpr (switchEndian) {
      duration = swapEndianFloat(duration);
      ticksPerSecond = swapEndian(ticksPerSecond);
    }

    // Create animation
    model.mAnimation = std::make_unique<Animation>();
    auto& animation = model.mAnimation;

    animation->mDuration = duration;
    animation->mTicksPerSecond = ticksPerSecond;

    // Read bone information map
    uint32_t boneCount;
    file.read(reinterpret_cast<char*>(&boneCount), sizeof(boneCount));
    if constexpr (switchEndian) boneCount = swapEndian(boneCount);

    for (uint32_t i = 0; i < boneCount; ++i) {
      int id;
      glm::mat4 offset;
      int boneInfoId;

      file.read(reinterpret_cast<char*>(&id), sizeof(id));
      file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
      file.read(reinterpret_cast<char*>(&boneInfoId), sizeof(boneInfoId));

      if constexpr (switchEndian) {
        id = static_cast<int>(swapEndian(static_cast<uint32_t>(id)));
        offset = swapEndianMat4(offset);
        boneInfoId = static_cast<int>(swapEndian(static_cast<uint32_t>(boneInfoId)));
      }

      animation->mBoneInfoMap[id] = { boneInfoId, offset };
    }

    // Read bone map (Animation keyframes)
    uint32_t boneMapSize;
    file.read(reinterpret_cast<char*>(&boneMapSize), sizeof(boneMapSize));
    if constexpr (switchEndian) boneMapSize = swapEndian(boneMapSize);

    // Read bone keyframes
    for (uint32_t i = 0; i < boneMapSize; ++i) {
      int nodeId, boneId;
      uint32_t posKeyCount, rotKeyCount, scaleKeyCount;

      file.read(reinterpret_cast<char*>(&nodeId), sizeof(nodeId));
      file.read(reinterpret_cast<char*>(&boneId), sizeof(boneId));
      file.read(reinterpret_cast<char*>(&posKeyCount), sizeof(posKeyCount));
      file.read(reinterpret_cast<char*>(&rotKeyCount), sizeof(rotKeyCount));
      file.read(reinterpret_cast<char*>(&scaleKeyCount), sizeof(scaleKeyCount));

      if constexpr (switchEndian) {
        nodeId = static_cast<int>(swapEndian(static_cast<uint32_t>(nodeId)));
        boneId = static_cast<int>(swapEndian(static_cast<uint32_t>(boneId)));
        posKeyCount = swapEndian(posKeyCount);
        rotKeyCount = swapEndian(rotKeyCount);
        scaleKeyCount = swapEndian(scaleKeyCount);
      }

      Bone bone(boneId);
      bone.mNumPositions = posKeyCount;
      bone.mNumRotations = rotKeyCount;
      bone.mNumScalings = scaleKeyCount;
      bone.mPositions.resize(posKeyCount);
      bone.mRotations.resize(rotKeyCount);
      bone.mScales.resize(scaleKeyCount);

      // Read Position Keyframes
      for (uint32_t j = 0; j < posKeyCount; ++j) {
        float time;
        glm::vec3 position;

        file.read(reinterpret_cast<char*>(&time), sizeof(time));
        file.read(reinterpret_cast<char*>(&position), sizeof(position));

        if constexpr (switchEndian) {
          time = swapEndianFloat(time);
          position = swapEndianVec3(position);
        }

        bone.mPositions[j] = { position, time };
      }

      // Read Rotation Keyframes
      for (uint32_t j = 0; j < rotKeyCount; ++j) {
        float time;
        glm::quat rotation;

        file.read(reinterpret_cast<char*>(&time), sizeof(time));
        file.read(reinterpret_cast<char*>(&rotation), sizeof(rotation));

        if constexpr (switchEndian) {
          time = swapEndianFloat(time);
          rotation = glm::quat(
            swapEndianFloat(rotation.w), swapEndianFloat(rotation.x),
                       swapEndianFloat(rotation.y), swapEndianFloat(rotation.z));
        }

        bone.mRotations[j] = { rotation, time };
      }

      // Read Scaling Keyframes
      for (uint32_t j = 0; j < scaleKeyCount; ++j) {
        float time;
        glm::vec3 scale;

        file.read(reinterpret_cast<char*>(&time), sizeof(time));
        file.read(reinterpret_cast<char*>(&scale), sizeof(scale));

        if constexpr (switchEndian) {
          time = swapEndianFloat(time);
          scale = swapEndianVec3(scale);
        }

        bone.mScales[j] = { scale, time };
      }

      animation->mBoneMap[nodeId] = bone;
    }

    // Read Animation Heirarchy
    uint32_t nodeCount;
    file.read(reinterpret_cast<char*>(&nodeCount), sizeof(nodeCount));
    if constexpr (switchEndian) nodeCount = swapEndian(nodeCount);

    for (uint32_t i = 0; i < nodeCount; ++i) {
      int id;
      glm::mat4 transformation;
      uint32_t childCount;
      std::vector<int> children;

      file.read(reinterpret_cast<char*>(&id), sizeof(id));
      file.read(reinterpret_cast<char*>(&transformation), sizeof(transformation));
      file.read(reinterpret_cast<char*>(&childCount), sizeof(childCount));
      children.resize(childCount);
      file.read(reinterpret_cast<char*>(children.data()), childCount * sizeof(int));

      if constexpr (switchEndian) {
        id = static_cast<int>(swapEndian(static_cast<uint32_t>(id)));
        transformation = swapEndianMat4(transformation);
        childCount = swapEndian(childCount);
        for (auto& child : children) {
          child = static_cast<int>(swapEndian(static_cast<uint32_t>(child)));
        }
      }

      animation->mNodes.push_back({ transformation, id, children });
    }

    model.mAnimator = std::make_unique<Animator>(animation.get());
    model.hasAnimations = true;
  }

} // namespace Rendering
