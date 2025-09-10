#include <PCH/pch.h>
#include "Renderer.h"

#include "ECS/Resources/RenderingResource.h"
#include "Core/Synchronization.h"
#include "Core/SwapChain.h"
#include "Core/Device.h"

#include "RenderGraph.h"
#include "RenderPass/PresentPass.h"

namespace Dog
{
    Renderer::Renderer(RenderingResource& rr)
        : renderingResource{ rr }
        , mCurrentImageIndex{ 0 }
        , mCurrentFrameIndex{ 0 }
    {
        CreateCommandBuffers();
        mPresentPass = std::make_unique<PresentPass>(*renderingResource.device, *renderingResource.swapChain, *renderingResource.syncObjects);
    }

    Renderer::~Renderer()
    {
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

        RenderGraph graph;
        graph.add_present_pass("PresentToScreen", [&](VkCommandBuffer cmd) {
            mPresentPass->Execute(cmd);
        });

        graph.execute(
            commandBuffer,
            renderingResource.swapChain->GetImageView(mCurrentImageIndex),
            renderingResource.swapChain->GetDepthImageView(mCurrentImageIndex),
            renderingResource.swapChain->GetSwapChainExtent(),
            renderingResource.device->getDevice(),
            renderingResource.swapChain->GetImageFormat(),
            renderingResource.swapChain->FindDepthFormat()
        );

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
}
