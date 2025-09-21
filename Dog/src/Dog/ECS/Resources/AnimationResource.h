#pragma once

#include "IResource.h"

namespace Dog
{
    struct AnimationResource : public IResource
    {
        AnimationResource();

        std::vector<glm::mat4> bonesMatrices;
    };
}
