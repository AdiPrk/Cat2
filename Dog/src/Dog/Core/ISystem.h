#pragma once

namespace Dog 
{
    class ISystem
    {
    public:
        ISystem(const std::string& name) : m_DebugName(name) {}
        virtual ~ISystem() = default;

        virtual void Init() {};
        virtual void FrameStart() {};
        virtual void Update(float dt) {};
        virtual void FrameEnd() {};
        virtual void Exit() {};

        entt::registry& GetECS();
        const std::string& GetDebugName() const { return m_DebugName; }

    private:
        std::string m_DebugName;
    };
}
