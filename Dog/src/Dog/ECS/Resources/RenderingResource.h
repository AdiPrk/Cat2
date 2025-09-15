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
    class Uniform;
    class ModelLibrary;
    class TextureLibrary;

    struct RenderingResource : public IResource
    {
        RenderingResource(Window& window);

        std::unique_ptr<Device> device;
        std::unique_ptr<SwapChain> swapChain;
        std::unique_ptr<Synchronizer> syncObjects;
        std::unique_ptr<Allocator> allocator;

        std::unique_ptr<ModelLibrary> modelLibrary;
        std::unique_ptr<TextureLibrary> textureLibrary;

        std::unique_ptr<Renderer> renderer;

        std::unique_ptr<Uniform> cameraUniform;
        std::unique_ptr<Uniform> instanceUniform;

    private:
        Window& window;

    private:
        friend class Renderer;
        void RecreateSwapChain();
    };
}
