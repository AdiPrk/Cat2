#pragma once

#include "ISystem.h"

namespace Dog
{
    class Pipeline;

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

        void RenderScene(VkCommandBuffer cmd);

    private:

        VkImage sceneImage{ VK_NULL_HANDLE };
        VmaAllocation sceneImageAllocation{ VK_NULL_HANDLE };
        VkImageView sceneImageView{ VK_NULL_HANDLE };
        VkSampler sceneSampler{ VK_NULL_HANDLE };

        VkImage mDepthImage;
        VmaAllocation mDepthImageAllocation;
        VkImageView mDepthImageView;

        std::unique_ptr<Pipeline> mPipeline;
    };
}

