#pragma once

#include "IResource.h"

namespace Dog
{
    struct EditorResource : public IResource
    {
        EditorResource();

        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout samplerSetLayout;
    };
}
