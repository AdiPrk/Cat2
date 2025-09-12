#pragma once

#include "ISystem.h"

namespace Dog
{
    class EditorSystem : public ISystem
    {
    public:
        EditorSystem() : ISystem("EditorSystem") {};
        ~EditorSystem() {}

        void Init();
        void FrameStart();
        void Update(float);
        void FrameEnd();
        void Exit();
    };
}