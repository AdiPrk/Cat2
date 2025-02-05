#pragma once

#include "ActionManager.h"

namespace Dog
{

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

    class RotateEntityAction : public Action
    {
    public:
        RotateEntityAction(uint32_t entityID, float oldX, float oldY, float oldZ, float newX, float newY, float newZ);
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
        ScaleEntityAction(uint32_t entityID, float oldX, float oldY, float oldZ, float newX, float newY, float newZ);
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
        TransformEntityAction(uint32_t entityID, float oPX, float oPY, float oPZ, float oRX, float oRY, float oRZ, float oSX, float oSY, float oSZ, float nPX, float nPY, float nPZ, float nRX, float nRY, float nRZ, float nSX, float nSY, float nSZ);
        void Apply() override;
        void Undo() override;
        std::string Serialize() const override;

    private:
        uint32_t entityID;
        float oPX, oPY, oPZ, oRX, oRY, oRZ, oSX, oSY, oSZ, nPX, nPY, nPZ, nRX, nRY, nRZ, nSX, nSY, nSZ;
    };

}
