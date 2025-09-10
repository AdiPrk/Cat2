#pragma once

namespace Dog
{
    struct IResource
    {
    public:
        IResource() = default;
        virtual ~IResource() = default;

        IResource(const IResource&) = delete;
        IResource& operator=(const IResource&) = delete;

    protected:
        friend class ECS;
        ECS* ecs = nullptr;
    };
}
