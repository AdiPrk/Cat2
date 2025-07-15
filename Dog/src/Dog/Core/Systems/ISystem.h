#pragma once

namespace Dog 
{
    class ISystem
    {
    public:
        ISystem() {}
        virtual ~ISystem() = default;

        virtual void Init() {};
        virtual void FrameStart() {};
        virtual void Update(float dt) {};
        virtual void FrameEnd() {};
        virtual void Exit() {};

        entt::registry& GetECS();
    };
}
