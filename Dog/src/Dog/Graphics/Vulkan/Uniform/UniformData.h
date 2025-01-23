/*********************************************************************
 * file:   UniformData.hpp
 * author: aditya.prakash (aditya.prakash@digipen.edu) and evan.gray (evan.gray@digipen.edu)
 * date:   October 10, 2024
 * Copyright � 2024 DigiPen (USA) Corporation. 
 * 
 * brief:  Handles cosntant data about uniforms
 *********************************************************************/
#pragma once

#include "UniformSettings.h"
#include "../Font/Font.h"
#include "../Texture/TextureLibrary.h"
#include "../Models/ModelLibrary.h"
#include "../Animation/Animator.h"
#include "InstanceData.h"

namespace Dog
{
  // Forward declarations
  class FontLibrary;

  void MatrixUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary);
  void InstancedUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary);
  void SharedUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary);
  void ComputeUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary);
  void HeightmapUniformInit(Uniform& uniform, TextureLibrary& textureLibrary, FontLibrary& fontLibrary, ModelLibrary& modelLibrary);

  // MatrixUniformData holds camera and light info
  struct MatrixUniformData {
    glm::mat4 projectionView{ 1.0f };
    glm::mat4 projection{ 1.0f };
    glm::mat4 view{ 1.0f };
    glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.0f, -3.0f, -1.0f });
  };

  // Uniform set 0 
  const UniformSettings matrixUniformSettings = UniformSettings(MatrixUniformInit)
    .AddUBBinding(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, sizeof(MatrixUniformData));

  // Uniform set 1 (for vert/frag/compute)
  const UniformSettings instancedUniformSettings = UniformSettings(InstancedUniformInit)
    .AddSSBOBinding(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, sizeof(InstancedUniformData), InstancedUniformData::MAX_INSTANCE_COUNT)
    .AddSSBOBinding(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT, sizeof(MeshData), InstancedUniformData::MAX_INSTANCE_COUNT);

  // Uniform set 2
  const UniformSettings sharedUniformSettings = UniformSettings(SharedUniformInit)
    .AddSSBOBinding(VK_SHADER_STAGE_VERTEX_BIT, sizeof(GlyphData), GlyphDataBuffer::MAX_DRAW_COUNT)
    .AddISBinding(VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, sizeof(VkDescriptorImageInfo), TextureLibrary::MAX_TEXTURE_COUNT)
    .AddSSBOBinding(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), Animation::MAX_BONES * ModelLibrary::MAX_MODEL_COUNT);

  struct HeightmapUniformData 
  {
    glm::mat4 modelMatrix{ 1.0f };
    glm::mat4 viewMatrix{ 1.0f };
    glm::vec4 cameraPos;
    unsigned tessellationFactor;
    uint32_t heightmapTextureIndex;
  };

  // Uniform set 3
  const UniformSettings heightmapUniformSettings = UniformSettings(HeightmapUniformInit)
    .AddUBBinding(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(HeightmapUniformData));

  struct ComputeInstanceGroup
  {
    uint32_t instanceCount;
    uint32_t wireframe; // 0 or 1
    uint32_t firstInstance;
    uint32_t indexCount;
    uint32_t firstIndex;
    int32_t  vertexOffset;
    // no padding required since every element is 4 bytes
  };

  // Compute Uniform set 0
  const UniformSettings computeUniformSettings = UniformSettings(ComputeUniformInit)
    .AddSSBOIndirectBinding(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(VkDrawIndexedIndirectCommand), InstancedUniformData::MAX_INSTANCE_COUNT)
    .AddSSBOIndirectBinding(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(VkDrawIndexedIndirectCommand), InstancedUniformData::MAX_INSTANCE_COUNT)
    .AddSSBOIndirectBinding(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(uint32_t), 3);
}