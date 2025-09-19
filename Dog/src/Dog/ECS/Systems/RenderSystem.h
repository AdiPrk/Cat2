#pragma once

#include "ISystem.h"

namespace Dog
{
    class Pipeline;

    class RenderSystem : public ISystem
    {
    public:
        RenderSystem() : ISystem("RenderSystem") {};
        ~RenderSystem();

        void Init();
        void FrameStart();
        void Update(float dt);
        void FrameEnd();
        void Exit();

        void RenderScene(VkCommandBuffer cmd);

    private:

        std::unique_ptr<Pipeline> mPipeline;
    };
}

