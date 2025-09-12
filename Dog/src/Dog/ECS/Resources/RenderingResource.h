#pragma once

#include "IResource.h"

namespace Dog
{
    class Window;
    class Device;
    class SwapChain;
    class Synchronizer;
    class Pipeline;
    class Renderer;
    class Allocator;

    struct RenderingResource : public IResource
    {
        RenderingResource(Window& window);

        std::unique_ptr<Device> device;
        std::unique_ptr<SwapChain> swapChain;
        std::unique_ptr<Synchronizer> syncObjects;
        std::unique_ptr<Pipeline> basicTriPipeline;
        std::unique_ptr<Allocator> allocator;

        std::unique_ptr<Renderer> renderer;

    private:
        Window& window;

    private:
        friend class Renderer;
        void RecreateSwapChain();
    };
}
