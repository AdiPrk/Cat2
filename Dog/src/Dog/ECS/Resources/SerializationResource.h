#pragma once

#include "IResource.h"

namespace Dog
{
    struct SerializationResource : public IResource
    {
        void Serialize(const std::string& filepath);
        void Deserialize(const std::string& filepath);
    };
}
