#pragma once

#include "ISystem.h"

namespace Dog
{
    class RenderSystem : public ISystem
    {
    public:
        RenderSystem() : ISystem("RenderSystem") {};
        ~RenderSystem() {}

        void Init();
        void FrameStart();
        void Update(float);
        void FrameEnd();
        void Exit();
    };
}

