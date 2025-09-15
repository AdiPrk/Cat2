#pragma once

namespace Dog
{
    struct RenderingResource;
    class RenderGraph;
    class Pipeline;

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
        friend RenderGraph;
        RenderingResource& renderingResource;

        std::vector<VkCommandBuffer> mCommandBuffers;
        uint32_t mCurrentImageIndex;
        uint32_t mCurrentFrameIndex;

        std::unique_ptr<RenderGraph> mRenderGraph;
        std::unique_ptr<Pipeline> mPipeline;

    // This stuff should be temporary
    private:
        void CreateSceneTexture();
        void CleanupSceneTexture();
        void RecreateSceneTexture();

        void CreateDepthBuffer();
        void CleanupDepthBuffer();
        void RecreateDepthBuffer();

        VkImage sceneImage{ VK_NULL_HANDLE };
        VmaAllocation sceneImageAllocation{ VK_NULL_HANDLE };
        VkImageView sceneImageView{ VK_NULL_HANDLE };
        VkSampler sceneSampler{ VK_NULL_HANDLE };

        VkImage mDepthImage;
        VmaAllocation mDepthImageAllocation;
        VkImageView mDepthImageView;

        VkDescriptorSet sceneTextureDescriptorSet{ VK_NULL_HANDLE };
    };
}
