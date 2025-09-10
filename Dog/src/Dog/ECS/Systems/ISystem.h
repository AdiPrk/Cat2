#pragma once

namespace Dog 
{
    class ISystem
    {
    public:
        ISystem(const std::string& name) : m_DebugName(name), ecs(nullptr) {}
        virtual ~ISystem() = default;

        virtual void Init() {};
        virtual void FrameStart() {};
        virtual void Update(float dt) {};
        virtual void FrameEnd() {};
        virtual void Exit() {};

        const std::string& GetDebugName() const { return m_DebugName; }

    protected:
        friend class ECS;
        ECS* ecs;

    private:
        std::string m_DebugName;
    };
}
