#pragma once

#include "ActionManager.h"

namespace Dog
{

    class MoveEntityAction : public Action
    {
    public:
        MoveEntityAction(Event::EntityMoved e);
        void Apply() override;
        void Undo() override;
        std::string Serialize() const override;

    private:
        uint32_t entityID;
        float oldX, oldY, oldZ, newX, newY, newZ;
    };

    class RotateEntityAction : public Action
    {
    public:
        RotateEntityAction(Event::EntityRotated e);
        void Apply() override;
        void Undo() override;
        std::string Serialize() const override;

    private:
        uint32_t entityID;
        float oldX, oldY, oldZ, newX, newY, newZ;
    };

    class ScaleEntityAction : public Action
    {
    public:
        ScaleEntityAction(Event::EntityScaled e);
        void Apply() override;
        void Undo() override;
        std::string Serialize() const override;

    private:
        uint32_t entityID;
        float oldX, oldY, oldZ, newX, newY, newZ;
    };

    class TransformEntityAction : public Action
    {
    public:
        TransformEntityAction(Event::EntityTransform e);
        void Apply() override;
        void Undo() override;
        std::string Serialize() const override;

    private:
        uint32_t entityID;
        float oPX, oPY, oPZ, oRX, oRY, oRZ, oSX, oSY, oSZ, nPX, nPY, nPZ, nRX, nRY, nRZ, nSX, nSY, nSZ;
    };

}
