#include <PCH/pch.h>
#include "RenderGraph.h"

namespace Dog
{
    void RGPassBuilder::writes(RGResourceHandle handle)
    {
        m_pass.writeTargets.push_back(handle);
    }

    void RGPassBuilder::reads(RGResourceHandle handle)
    {
        m_pass.readTargets.push_back(handle);
    }

    RGResourceHandle RenderGraph::import_backbuffer(const char* name, VkImage image, VkImageView view, VkExtent2D extent, VkFormat format)
    {
        RGResource backbuffer_resource;
        backbuffer_resource.name = name;
        backbuffer_resource.image = image;
        backbuffer_resource.imageView = view;
        backbuffer_resource.extent = extent;
        backbuffer_resource.format = format;
        backbuffer_resource.currentLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Swapchain images start as undefined.

        m_resources.push_back(backbuffer_resource);
        return { static_cast<uint32_t>(m_resources.size() - 1) };
    }

    RGResourceHandle RenderGraph::create_texture(const char* name, VkExtent2D extent, VkFormat format)
    {
        RGResource new_resource;
        new_resource.name = name;
        new_resource.extent = extent;
        new_resource.format = format;

        // Resource should be created in graph but we're temporarily making it in renderer once instead to get things running
        m_resources.push_back(new_resource);
        return { static_cast<uint32_t>(m_resources.size() - 1) };
    }

    void RenderGraph::add_pass(const char* name,
        std::function<void(RGPassBuilder&)>&& setup,
        std::function<void(VkCommandBuffer)>&& execute)
    {
        RGPass pass;
        pass.name = name;
        pass.setupCallback = std::move(setup);
        pass.executeCallback = std::move(execute);

        RGPassBuilder builder(pass);
        pass.setupCallback(builder);

        m_passes.push_back(pass);
    }

    void RenderGraph::execute(VkCommandBuffer cmd, VkDevice device)
    {
        for (const auto& pass : m_passes)
        {

            // --- 1. Automatic Barrier Insertion (Image Layout Transitions) ---

            for (const auto& handle : pass.readTargets)
            {
                RGResource& resource = m_resources[handle.index];
                if (resource.currentLayout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = resource.currentLayout;
                    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = resource.image;
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.layerCount = 1;

                    // From color attachment write to shader read
                    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                    vkCmdPipelineBarrier(cmd,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);

                    resource.currentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                }
            }


            for (const auto& handle : pass.writeTargets)
            {
                RGResource& resource = m_resources[handle.index];

                // Transition the backbuffer for rendering
                if (resource.currentLayout != VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.oldLayout = resource.currentLayout;
                    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                    barrier.image = resource.image;
                    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    barrier.subresourceRange.baseMipLevel = 0;
                    barrier.subresourceRange.levelCount = 1;
                    barrier.subresourceRange.baseArrayLayer = 0;
                    barrier.subresourceRange.layerCount = 1;

                    barrier.srcAccessMask = 0; // Or determine from previous usage
                    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                    vkCmdPipelineBarrier(cmd,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        0, 0, nullptr, 0, nullptr, 1, &barrier);

                    resource.currentLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
            }

            // --- 2. Begin Dynamic Rendering ---
            // For simplicity, we assume the first write target is the color attachment.
            // A more robust system would handle multiple attachments.
            if (!pass.writeTargets.empty())
            {
                RGResource& colorTarget = m_resources[pass.writeTargets[0].index];

                VkRenderingAttachmentInfo colorAttachment{};
                colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
                colorAttachment.imageView = colorTarget.imageView;
                colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

                VkRenderingInfo renderingInfo{};
                renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
                renderingInfo.renderArea = { {0, 0}, colorTarget.extent };
                renderingInfo.layerCount = 1;
                renderingInfo.colorAttachmentCount = 1;
                renderingInfo.pColorAttachments = &colorAttachment;
                // Note: Depth buffer management would also be added here
                renderingInfo.pDepthAttachment = nullptr;
                renderingInfo.pStencilAttachment = nullptr;

                vkCmdBeginRendering(cmd, &renderingInfo);

                pass.executeCallback(cmd);

                vkCmdEndRendering(cmd);
            }
        }

        // --- 3. Final Transition for Presentation ---
        // After all passes, transition the backbuffer for presenting.
        if (!m_resources.empty())
        {
            RGResource& backbuffer = m_resources[0]; // Assuming backbuffer is always the first imported resource
            if (backbuffer.currentLayout != VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = backbuffer.currentLayout;
                barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.image = backbuffer.image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                barrier.dstAccessMask = 0;

                vkCmdPipelineBarrier(cmd,
                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    0, 0, nullptr, 0, nullptr, 1, &barrier);

                backbuffer.currentLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
        }
    }

    void RenderGraph::clear()
    {
        m_passes.clear();
        m_resources.clear();
    }
}