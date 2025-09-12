 #pragma once

namespace Dog
{
    // Opaque handle to a resource managed by the render graph.
    // This abstracts away the underlying VkImage/VkBuffer.
    struct RGResourceHandle {
        uint32_t index;
        bool operator==(const RGResourceHandle& other) const { return index == other.index; }
    };

    // Internal representation of a resource's state.
    struct RGResource
    {
        std::string name;
        VkImage image{ VK_NULL_HANDLE };
        VkImageView imageView{ VK_NULL_HANDLE };
        VkExtent2D extent;
        VkFormat format;

        // State tracking for automatic barriers
        VkImageLayout currentLayout{ VK_IMAGE_LAYOUT_UNDEFINED };
    };

    struct RGPass; // Forward declaration

    // A transient helper object passed to the pass setup lambda.
    // It provides a clean API for declaring what a pass reads from and writes to.
    class RGPassBuilder
    {
    public:
        RGPassBuilder(RGPass& pass) : m_pass(pass) {}

        // Declare that this pass writes to a resource.
        void writes(RGResourceHandle handle);

        // Declare that this pass reads from a resource.
        void reads(RGResourceHandle handle);


    private:
        RGPass& m_pass;
    };

    // Logical description of a render pass and its resource usage.
    struct RGPass {
        std::string name;
        std::function<void(RGPassBuilder&)> setupCallback;
        std::function<void(VkCommandBuffer)> executeCallback;

        std::vector<RGResourceHandle> writeTargets;
        std::vector<RGResourceHandle> readTargets;
    };

    // The main orchestrator class
    class RenderGraph {
    public:
        RenderGraph() = default;

        // Imports an existing, externally managed image (like the swapchain) into the graph.
        RGResourceHandle import_backbuffer(const char* name, VkImage image, VkImageView view, VkExtent2D extent, VkFormat format);

        // Create a transient texture owned by the graph
        RGResourceHandle create_texture(const char* name, VkExtent2D extent, VkFormat format);

        // Adds a new pass to the graph. The setup callback declares dependencies.
        void add_pass(const char* name,
            std::function<void(RGPassBuilder&)>&& setup,
            std::function<void(VkCommandBuffer)>&& execute);

        // Compiles and executes the graph, recording commands into the provided buffer.
        void execute(VkCommandBuffer cmd, VkDevice device);

        // Clears all passes and resources for the next frame.
        void clear();

    private:
        std::vector<RGResource> m_resources;
        std::vector<RGPass> m_passes;
    };
}
