#include <PCH/pch.h>
#include "RenderingResource.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Vulkan/Core/SwapChain.h"
#include "Graphics/Vulkan/Core/Synchronization.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"
#include "Graphics/Vulkan/Renderer.h"

#include "Graphics/Window/Window.h"

#include "Graphics/Vulkan/Uniform/Uniform.h"
#include "Graphics/Vulkan/Uniform/UniformData.h"    
#include "Graphics/Vulkan/Uniform/Descriptors.h"

#include "Graphics/Vulkan/Model/ModelLibrary.h"
#include "Graphics/Vulkan/Texture/TextureLibrary.h"

namespace Dog
{
    RenderingResource::RenderingResource(Window& window)
        : device(std::make_unique<Device>(window))
        , window{ window }
        , allocator(std::make_unique<Allocator>(*device))
    {
        RecreateSwapChain();
        syncObjects = std::make_unique<Synchronizer>(device->getDevice(), swapChain->ImageCount());

        textureLibrary = std::make_unique<TextureLibrary>(*device);
        modelLibrary = std::make_unique<ModelLibrary>(*device, *textureLibrary);
        modelLibrary->AddModel("Assets/models/AlisaMikhailovna.fbx");

        modelLibrary->LoadTextures();

        cameraUniform = std::make_unique<Uniform>(*device, *this, cameraUniformSettings);
        instanceUniform = std::make_unique<Uniform>(*device, *this, instanceUniformSettings);

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
