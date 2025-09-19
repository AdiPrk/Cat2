#include <PCH/pch.h>
#include "PresentPass.h"

#include "../Pipeline/Pipeline.h"
#include "../Core/SwapChain.h"
#include "../Core/Synchronization.h"
namespace Dog
{
    PresentPass::PresentPass(Device& device, SwapChain& swapChain, Synchronizer& sync)
        : device(device)
        , swapChain(swapChain)
        , syncObjects(sync)
    {
        CreatePipeline();
    }

    PresentPass::~PresentPass()
    {
        Cleanup();
    }

    void PresentPass::CreatePipeline()
    {
        std::vector<Uniform*> unis{}; // temp empty
        pipeline = std::make_unique<Pipeline>(device, swapChain.GetImageFormat(), swapChain.GetDepthFormat(), unis, false, "basic_tri.vert", "basic_tri.frag");
    }

    void PresentPass::Execute(VkCommandBuffer cmd)
    {
        pipeline->Bind(cmd);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChain.GetSwapChainExtent().width);
        viewport.height = static_cast<float>(swapChain.GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        // Set the dynamic scissor
        VkRect2D scissor{ {0, 0}, swapChain.GetSwapChainExtent() };
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdDraw(cmd, 3, 1, 0, 0);
    }
    
    void PresentPass::Cleanup()
    {
    }
}
