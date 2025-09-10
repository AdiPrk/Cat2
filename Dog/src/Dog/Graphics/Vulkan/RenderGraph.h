#pragma once

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
        // --- Declaration API ---
        // In this minimal example, we only have one pass that outputs to the screen.
        void add_present_pass(const char* name, std::function<void(VkCommandBuffer)>&& callback) {
            RGPass pass;
            pass.name = name;
            pass.executeCallback = std::move(callback);

            // In a real graph, we'd track resource dependencies here.
            // For now, we just store the single pass.
            m_passes.push_back(pass);
        }

        // --- Backend Execution ---
        void execute(VkCommandBuffer cmd,
            VkImageView colorTargetView, VkImageView depthTargetView,
            VkExtent2D targetExtent, VkDevice device,
            VkFormat colorFormat, VkFormat depthFormat) 
        {
            // 1. Create a transient VkRenderPass compatible with your setup
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = colorFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = depthFormat;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            // --- FIX #1: Add the dependency to match the pipeline's render pass ---
            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1; // Set count to 1
            renderPassInfo.pDependencies = &dependency; // Point to the dependency

            VkRenderPass renderPass;
            if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                throw std::runtime_error("RG: failed to create transient render pass!");
            }

            // 2. Create a transient VkFramebuffer with both attachments
            std::array<VkImageView, 2> fbAttachments = { colorTargetView, depthTargetView };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(fbAttachments.size());
            framebufferInfo.pAttachments = fbAttachments.data();
            framebufferInfo.width = targetExtent.width;
            framebufferInfo.height = targetExtent.height;
            framebufferInfo.layers = 1;

            VkFramebuffer framebuffer;
            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
                throw std::runtime_error("RG: failed to create transient framebuffer!");
            }

            // 3. Execute the recorded passes
            VkRenderPassBeginInfo passBeginInfo{};
            passBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            passBeginInfo.renderPass = renderPass;
            passBeginInfo.framebuffer = framebuffer;
            passBeginInfo.renderArea.offset = { 0, 0 };
            passBeginInfo.renderArea.extent = targetExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clearValues[1].depthStencil = { 1.0f, 0 };
            passBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            passBeginInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(cmd, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            //Set up dynamic viewport
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(targetExtent.width);
            viewport.height = static_cast<float>(targetExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(cmd, 0, 1, &viewport);

            //Set up dynamic scissor
            VkRect2D scissor{ {0, 0}, targetExtent };
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            m_passes[0].executeCallback(cmd); // Assuming one pass for this example
            vkCmdEndRenderPass(cmd);

            // 4. Clean up transient objects
            // vkDestroyFramebuffer(device, framebuffer, nullptr);
            // vkDestroyRenderPass(device, renderPass, nullptr);
        }

    private:
        std::vector<RGPass> m_passes;
    };
}
