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

    protected:
        uint32_t m_ID;
    };

    class ActionManager
    {
    public:
        explicit ActionManager(size_t maxSize = 100);

        void AddAction(std::unique_ptr<Action> action);
        void Undo();
        void Redo();
        void Clear();
    private:
        uint32_t m_CurrentIndex = 0;
        std::vector<std::unique_ptr<Action>> m_Actions;
        size_t m_MaxSize;

        Events::Handle<Event::EntityMoved> m_EntityMovedHandle;
        void OnEntityMoved(const Event::EntityMoved& event);
    };

    class MoveEntityAction : public Action
    {
    public:
        MoveEntityAction(uint32_t entityID, float oldX, float oldY, float oldZ, float newX, float newY, float newZ);
        void Apply() override;
        void Undo() override;
        std::string Serialize() const override;

    private:
        uint32_t entityID;
        float oldX, oldY, oldZ, newX, newY, newZ;
    };

}
