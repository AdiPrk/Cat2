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
        std::unique_ptr<Pipeline> mTrianglePipeline;

    // This stuff should be temporary
    private:
        void CreateSceneTexture();
        void CleanupSceneTexture();
        void RecreateSceneTexture();

        VkImage sceneImage{ VK_NULL_HANDLE };
        VmaAllocation sceneImageAllocation{ VK_NULL_HANDLE }; // Changed from VkDeviceMemory
        VkImageView sceneImageView{ VK_NULL_HANDLE };
        VkSampler sceneSampler{ VK_NULL_HANDLE };

        VkDescriptorSet sceneTextureDescriptorSet{ VK_NULL_HANDLE };
    };
}
