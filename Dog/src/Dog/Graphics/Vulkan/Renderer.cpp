#include <PCH/pch.h>
#include "Renderer.h"

#include "ECS/Resources/RenderingResource.h"
#include "Core/Synchronization.h"
#include "Core/SwapChain.h"
#include "Core/Device.h"

#include "Pipeline/Pipeline.h"
#include "RenderGraph.h"

namespace Dog
{
    Renderer::Renderer(RenderingResource& rr)
        : renderingResource{ rr }
        , mCurrentImageIndex{ 0 }
        , mCurrentFrameIndex{ 0 }
    {
        CreateCommandBuffers();

        mRenderGraph = std::make_unique<RenderGraph>();

        std::vector<Uniform*> unis{}; // temp empty
        mTrianglePipeline = std::make_unique<Pipeline>(
            *renderingResource.device,
            renderingResource.swapChain->GetImageFormat(),
            renderingResource.swapChain->FindDepthFormat(),
            unis,
            false,
            "basic_tri.vert",
            "basic_tri.frag"
        );
    }

    Renderer::~Renderer()
    {
        vkDeviceWaitIdle(renderingResource.device->getDevice());
        CleanupSceneTexture();
    }

    void Renderer::drawFrame()
    {
        // Wait on the current frame's fence (ensures the previous frame's GPU work is done).
        renderingResource.syncObjects->WaitForCommandBuffers();

        // Aquire next image from swapchain
        VkResult result = renderingResource.swapChain->AcquireNextImage(&mCurrentImageIndex, *renderingResource.syncObjects);

        // Recreate if needed
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            renderingResource.RecreateSwapChain();
            RecreateSceneTexture();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            DOG_CRITICAL("Failed to present swap chain image");
        }

        // If image is in flight, wait for its fence
        if (renderingResource.syncObjects->GetImageInFlightFence(mCurrentImageIndex) != VK_NULL_HANDLE) {
            vkWaitForFences(renderingResource.device->getDevice(), 1, &renderingResource.syncObjects->GetImageInFlightFence(mCurrentImageIndex), VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now in use by this frame:
        renderingResource.syncObjects->GetImageInFlightFence(mCurrentFrameIndex) = renderingResource.syncObjects->GetCommandBufferInFlightFence();


        // Get the command buffer for the current frame and reset it
        VkCommandBuffer commandBuffer = mCommandBuffers[mCurrentFrameIndex];
        vkResetCommandBuffer(commandBuffer, 0);

        // --- Begin Recording with Render Graph ---
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // --- Render Graph Setup and Execution ---

        // Temporary;
        if (!sceneTextureDescriptorSet)
        {
            CreateSceneTexture();
        }

        // 1. Clear graph from previous frame.
        mRenderGraph->clear();

        // 2. Import the current swapchain image into the graph.
        RGResourceHandle sceneColorHandle = mRenderGraph->import_backbuffer(
            "SceneColor",
            sceneImage,
            sceneImageView,
            renderingResource.swapChain->GetSwapChainExtent(),
            renderingResource.swapChain->GetImageFormat()
        );

        RGResourceHandle backbufferHandle = mRenderGraph->import_backbuffer(
            "Backbuffer",
            renderingResource.swapChain->GetImage(),
            renderingResource.swapChain->GetImageView(),
            renderingResource.swapChain->GetSwapChainExtent(),
            renderingResource.swapChain->GetImageFormat()
        );

        // 3. PASS 1: Render the triangle scene to the scene texture.
        mRenderGraph->add_pass(
            "ScenePass",
            // Setup: This pass WRITES to the scene texture.
            [&](RGPassBuilder& builder) {
                builder.writes(sceneColorHandle);
            },
            // Execute: The same triangle drawing logic as before.
            [&](VkCommandBuffer cmd) {
                mTrianglePipeline->Bind(cmd);

                VkViewport viewport{};
                viewport.width = static_cast<float>(renderingResource.swapChain->GetSwapChainExtent().width);
                viewport.height = static_cast<float>(renderingResource.swapChain->GetSwapChainExtent().height);
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(cmd, 0, 1, &viewport);

                VkRect2D scissor{ {0, 0}, renderingResource.swapChain->GetSwapChainExtent() };
                vkCmdSetScissor(cmd, 0, 1, &scissor);

                vkCmdDraw(cmd, 3, 1, 0, 0);
            }
        );

        // 4. PASS 2: Render ImGui to the final swapchain backbuffer.
        mRenderGraph->add_pass(
            "ImGuiPass",
            // Setup: This pass READS the scene texture and WRITES to the backbuffer.
            [&](RGPassBuilder& builder) {
                builder.reads(sceneColorHandle);
                builder.writes(backbufferHandle);
            },
            // Execute: All the ImGui drawing commands.
            [&](VkCommandBuffer cmd) {
                // Start the Dear ImGui frame
                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                // Create a window and display the scene texture
                ImGui::Begin("Viewport");
                ImVec2 viewportSize = ImGui::GetContentRegionAvail();
                ImGui::Image(sceneTextureDescriptorSet, viewportSize);
                ImGui::End();

                // Rendering
                ImGui::Render();
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
            }
        );

        // 5. Execute the graph
        mRenderGraph->execute(commandBuffer, renderingResource.device->getDevice());

        // --- End Graph ---

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        // --- Submit the Command Buffer ---
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { renderingResource.syncObjects->GetImageAvailableSemaphore() };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = { renderingResource.syncObjects->GetRenderFinishedSemaphore(mCurrentImageIndex) };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(renderingResource.device->getDevice(), 1, &renderingResource.syncObjects->GetCommandBufferInFlightFence());
        if (vkQueueSubmit(renderingResource.device->getGraphicsQueue(), 1, &submitInfo, renderingResource.syncObjects->GetCommandBufferInFlightFence()) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // --- Presentation ---
        renderingResource.swapChain->PresentImage(&mCurrentImageIndex, *renderingResource.syncObjects);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            renderingResource.RecreateSwapChain();
            renderingResource.syncObjects->ClearImageFences();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        mCurrentFrameIndex = (mCurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
        renderingResource.syncObjects->NextFrame();
    }

    void Renderer::CreateCommandBuffers()
    {
        //Resize command buffer to match number of possible frames in flight
        mCommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        //Create struct for holding allocation info for this command buffer
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;							//Set what info this is holding to command buffer allocate info
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;														//Set level to primary (Primary buffers can be submitted for execution but cant be called by other buffers)
        allocInfo.commandPool = renderingResource.device->getCommandPool();															//Simular to a string pool, this is pre-allocated memorey for commands so we dont have to allocate memory a lot
        allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size()); //Tell the allocation info how many command buffers we are using

        //Allocate the command buffers
        if (vkAllocateCommandBuffers(renderingResource.device->getDevice(), &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
        {
            //If failed throw error
            DOG_CRITICAL("Failed to allocate command buffers");
        }
    }

    void Renderer::CreateSceneTexture()
    {
        VkDevice device = renderingResource.device->getDevice();
        // Get the VMA allocator instance from your device or resource manager
        VmaAllocator allocator = renderingResource.allocator->GetAllocator();
        VkExtent2D extent = renderingResource.swapChain->GetSwapChainExtent();
        VkFormat format = renderingResource.swapChain->GetImageFormat();

        // 1. Create VkImageCreateInfo
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = format;
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        // This image will be used as a color attachment and sampled in a shader
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // 2. Set up VMA allocation info
        VmaAllocationCreateInfo allocInfo{};
        // VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE tells VMA to place the image in the most optimal
        // memory, which is usually fast device-local VRAM.
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        // 3. Create the image and allocate its memory with VMA
        // This single function call replaces vkCreateImage, vkAllocateMemory, and vkBindImageMemory.
        VkResult result = vmaCreateImage(allocator, &imageInfo, &allocInfo, &sceneImage, &sceneImageAllocation, nullptr);
        if (result != VK_SUCCESS)
        {
            DOG_CRITICAL("VMA failed to create scene image");
        }

        // 4. Create the VkImageView
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = sceneImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &sceneImageView) != VK_SUCCESS)
        {
            DOG_CRITICAL("Failed to create scene image view");
        }

        // 5. Create a Sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &sceneSampler) != VK_SUCCESS)
        {
            DOG_CRITICAL("Failed to create scene sampler");
        }

        // 6. Create the Descriptor Set for ImGui to use
        // This call registers the texture with ImGui's Vulkan backend and returns a handle
        // that can be used with ImGui::Image().
        sceneTextureDescriptorSet = ImGui_ImplVulkan_AddTexture(sceneSampler, sceneImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void Renderer::CleanupSceneTexture()
    {
        VkDevice device = renderingResource.device->getDevice();
        VmaAllocator allocator = renderingResource.allocator->GetAllocator();

        // The ImGui descriptor set is allocated from ImGui's own pool, which is
        // cleaned up when ImGui_ImplVulkan_Shutdown() is called. We don't need to
        // free it here, but we should null the handle.
        sceneTextureDescriptorSet = VK_NULL_HANDLE;

        // Destroy the Vulkan objects in reverse order of creation
        if (sceneSampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device, sceneSampler, nullptr);
            sceneSampler = VK_NULL_HANDLE;
        }

        if (sceneImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, sceneImageView, nullptr);
            sceneImageView = VK_NULL_HANDLE;
        }

        // Use vmaDestroyImage to free the memory and destroy the image object
        if (sceneImage != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator, sceneImage, sceneImageAllocation);
            sceneImage = VK_NULL_HANDLE;
            sceneImageAllocation = VK_NULL_HANDLE;
        }
    }

    void Renderer::RecreateSceneTexture()
    {
        CleanupSceneTexture();
        CreateSceneTexture();
    }
}
