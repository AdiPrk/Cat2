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

    private:
        RenderingResource& renderingResource;
        uint32_t mCurrentImageIndex;
        uint32_t mCurrentFrameIndex;

        std::vector<VkCommandBuffer> mCommandBuffers;
        std::unique_ptr<PresentPass> mPresentPass;
    };
}
