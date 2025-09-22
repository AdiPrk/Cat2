#pragma once

#include "IResource.h"
#include "ECS/Entities/Entity.h"

namespace Dog
{
    class Device;
    class SwapChain;
    class Window;

    struct EditorResource : public IResource
    {
        EditorResource(Device& device, SwapChain& swapChain, Window& window);

        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout samplerSetLayout;

        float sceneWindowWidth = 1.f;
        float sceneWindowHeight = 1.f;

        Entity selectedEntity;
        Entity entityToDelete;
    };
}
