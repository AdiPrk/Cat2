#pragma once

namespace Dog
{
    class Action
    {
    public:
        virtual void Apply() = 0;
        virtual void Undo() = 0;
        virtual std::string Serialize() const = 0; // Convert to a network-friendly format
        virtual ~Action() = default;
    };

    class ActionManager
    {
    public:
        explicit ActionManager(size_t maxSize = 10000);

        void AddAction(std::unique_ptr<Action> action);
        void Undo();
        void Redo();
        void Clear();

        uint32_t GetNumActions() const { return static_cast<uint32_t>(m_Actions.size()); }
        const Action* GetCurrentAction();
        const Action* GetLastAction();

    private:
        uint32_t m_CurrentIndex = 0;
        std::vector<std::unique_ptr<Action>> m_Actions;
        size_t m_MaxSize;

        Events::Handle<Event::EntityMoved> m_EntityMovedHandle;
        Events::Handle<Event::EntityRotated> m_EntityRotatedHandle;
        Events::Handle<Event::EntityScaled> m_EntityScaledHandle;
        Events::Handle<Event::EntityTransform> m_EntityTransformHandle;

        void OnEntityMoved(const Event::EntityMoved& event);
        void OnEntityRotated(const Event::EntityRotated& event);
        void OnEntityScaled(const Event::EntityScaled& event);
        void OnEntityTransform(const Event::EntityTransform& event);
    };

}
