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

        // Template helper to subscribe to an event and associate it with an Action
        template<typename EventT, typename ActionT>
        void SubscribeAction() {
            auto handle = Events::Subscribe<EventT>([this](const EventT& event) {
                this->AddAction(std::make_unique<ActionT>(event));
            });

            m_SubscriptionHandles.push_back(
                std::make_unique<EventHandleWrapper<EventT>>(std::move(handle))
            );
        }

    private:
        uint32_t m_CurrentIndex = 0;
        std::vector<std::unique_ptr<Action>> m_Actions;
        size_t m_MaxSize;

        std::vector<std::unique_ptr<IEventHandle>> m_SubscriptionHandles;
    };

}
