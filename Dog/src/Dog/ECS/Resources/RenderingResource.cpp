#include <PCH/pch.h>
#include "RenderingResource.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Vulkan/Core/SwapChain.h"
#include "Graphics/Vulkan/Core/Synchronization.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"
#include "Graphics/Vulkan/Renderer.h"

#include "Graphics/Window/Window.h"

namespace Dog
{
    RenderingResource::RenderingResource(Window& window)
        : device(std::make_unique<Device>(window))
        , window{ window }
    {
        RecreateSwapChain();
        syncObjects = std::make_unique<Synchronizer>(device->getDevice(), swapChain->ImageCount());

        renderer = std::make_unique<Renderer>(*this);
    }

    void RenderingResource::RecreateSwapChain()
    {
        auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(*device);

        if (swapChain == nullptr) {
            swapChain = std::make_unique<SwapChain>(*device, extent);
        }
        else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
            swapChain = std::make_unique<SwapChain>(*device, extent, oldSwapChain);

            if (!oldSwapChain->CompareSwapFormats(*swapChain.get())) {
                DOG_ERROR("Swap chain image(or depth) format has changed!");
            }
        }
    }
}
