#pragma once

namespace Dog
{
    struct CameraUniforms {
        glm::mat4 projectionView;
        glm::mat4 projection;
        glm::mat4 view;
    };

    struct alignas(16) InstanceUniforms {
        glm::mat4 model;
        uint32_t textureIndex;

        const static uint32_t MAX_INSTANCES = 10000;
    };
}
