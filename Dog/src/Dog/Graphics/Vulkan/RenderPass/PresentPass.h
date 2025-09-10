#pragma once

namespace Dog
{
    class Pipeline;
    class Device;
    class SwapChain;
    class Synchronizer;

    class PresentPass
    {
    public:
        PresentPass(Device& device, SwapChain& swapChain, Synchronizer& sync);
        ~PresentPass();

        void CreateRenderPass();
        void CreatePipeline();
        void Execute(VkCommandBuffer cmd);
        void Cleanup();

    private:
        Device& device;
        SwapChain& swapChain;

        std::unique_ptr<Pipeline> pipeline;
        VkRenderPass mRenderPass;
        Synchronizer& syncObjects;
    };
}
