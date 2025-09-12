#pragma once

#include "ECS/Resources/RenderingResource.h"
#include "Core/SwapChain.h"
#include "Renderer.h"

namespace Dog
{
    // Logical description of a render pass
    struct RGPass {
        std::string name;
        std::function<void(VkCommandBuffer)> executeCallback;
    };

    // The main orchestrator class
    class RenderGraph {
    public:
        RenderGraph(RenderingResource& rr) : renderingResource(rr) {}
        // --- Declaration API ---
        // In this minimal example, we only have one pass that outputs to the screen.
        void add_present_pass(const char* name, std::function<void(VkCommandBuffer)>&& callback) {
            RGPass pass;
            pass.name = name;
            pass.executeCallback = std::move(callback);

            // For now, we just store the single pass.
            m_passes.push_back(pass);
        }

        // --- Backend Execution ---
        void execute(VkCommandBuffer cmd,
            VkImageView colorTargetView, VkImageView depthTargetView,
            VkExtent2D targetExtent, VkDevice device,
            VkFormat colorFormat, VkFormat depthFormat) 
        {
            // 1. Define the color attachment on-the-fly
            VkRenderingAttachmentInfoKHR colorAttachment{};
            colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            colorAttachment.imageView = colorTargetView;
            colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout during rendering
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

            // 2. Define the depth attachment on-the-fly
            VkRenderingAttachmentInfoKHR depthAttachment{};
            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            depthAttachment.imageView = depthTargetView;
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

            // 3. Define the overall rendering info
            VkRenderingInfoKHR renderingInfo{};
            renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
            renderingInfo.renderArea = { {0, 0}, targetExtent };
            renderingInfo.layerCount = 1;
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachments = &colorAttachment;
            renderingInfo.pDepthAttachment = &depthAttachment;
            renderingInfo.pStencilAttachment = nullptr; // Or set to &depthAttachment if using stencil

            // --- 4. BEGIN DYNAMIC RENDERING --- 
            vkCmdBeginRendering(cmd, &renderingInfo);

            // Execute the pass's draw commands (this part is the same)
            m_passes[0].executeCallback(cmd);

            // --- 5. END DYNAMIC RENDERING ---
            vkCmdEndRendering(cmd);
        }

    private:
        std::vector<RGPass> m_passes;
        RenderingResource& renderingResource;
    };
}
