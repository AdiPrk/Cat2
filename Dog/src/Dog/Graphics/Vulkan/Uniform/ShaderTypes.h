#pragma once

namespace Dog
{
    struct CameraUniforms 
    {
        glm::mat4 projectionView;
        glm::mat4 projection;
        glm::mat4 view;
    };

    struct alignas(16) InstanceUniforms {
        glm::mat4 model;
        uint32_t textureIndex;
        uint32_t boneOffset = 10001;

        const static uint32_t MAX_INSTANCES = 10000;
    };

    struct AnimationUniforms
    {
        glm::mat4 boneMatrix;

        const static uint32_t MAX_BONES = 10000;
    };
}
