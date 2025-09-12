#pragma once

namespace Dog
{
    struct RenderingResource;
    class PresentPass;

    class Renderer
    {
    public:
        Renderer(RenderingResource& rr);
        ~Renderer();

        void drawFrame();

        void CreateCommandBuffers();

        uint32_t GetCurrentFrameIndex() const { return mCurrentFrameIndex; }
        uint32_t GetCurrentImageIndex() const { return mCurrentImageIndex; }

    private:
        friend class RenderGraph;
        RenderingResource& renderingResource;
        uint32_t mCurrentImageIndex;
        uint32_t mCurrentFrameIndex;

        std::vector<VkCommandBuffer> mCommandBuffers;
        std::unique_ptr<PresentPass> mPresentPass;
    };
}
