#include <PCH/pch.h>
#include "RenderingResource.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Vulkan/Core/SwapChain.h"
#include "Graphics/Vulkan/Core/Synchronization.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"
#include "Graphics/Vulkan/RenderGraph.h"

#include "Graphics/Window/Window.h"

#include "Graphics/Vulkan/Uniform/Uniform.h"
#include "Graphics/Vulkan/Uniform/UniformData.h"

#include "Graphics/Vulkan/Model/ModelLibrary.h"
#include "Graphics/Vulkan/Texture/TextureLibrary.h"
#include "Graphics/Vulkan/Model/Animation/AnimationLibrary.h"
#include "Graphics/Vulkan/Model/Model.h"
#include "Graphics/Vulkan/Model/Animation/Animator.h"

namespace Dog
{
    RenderingResource::RenderingResource(Window& window)
        : device(std::make_unique<Device>(window))
        , window{ window }
        , allocator(std::make_unique<Allocator>(*device))
    {
        RecreateSwapChain();

        VkFormat srgbFormat = swapChain->GetImageFormat();
        VkFormat linearFormat = ToLinearFormat(srgbFormat);
        device->SetFormats(linearFormat, srgbFormat);

        syncObjects = std::make_unique<Synchronizer>(device->getDevice(), swapChain->ImageCount());

        textureLibrary = std::make_unique<TextureLibrary>(*device);

        modelLibrary = std::make_unique<ModelLibrary>(*device, *textureLibrary);
        modelLibrary->AddModel("Assets/models/jack_samba.glb");
        //modelLibrary->AddModel("Assets/models/okayu.pmx");
        //modelLibrary->AddModel("Assets/models/AlisaMikhailovna.fbx");

        animationLibrary = std::make_unique<AnimationLibrary>();
        animationLibrary->AddAnimation("Assets/models/jack_samba.glb", modelLibrary->GetModel(0));
        animationLibrary->GetAnimator(0)->PlayAnimation(animationLibrary->GetAnimation(0));

        modelLibrary->LoadTextures();

        CreateCommandBuffers();
        renderGraph = std::make_unique<RenderGraph>();

        cameraUniform = std::make_unique<Uniform>(*device, *this, cameraUniformSettings);
        instanceUniform = std::make_unique<Uniform>(*device, *this, instanceUniformSettings);

    }

    RenderingResource::~RenderingResource()
    {
        vkFreeCommandBuffers(
            device->getDevice(),
            device->getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data()
        );
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
    void RenderingResource::CreateCommandBuffers()
    {
        //Resize command buffer to match number of possible frames in flight
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        //Create struct for holding allocation info for this command buffer
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;							//Set what info this is holding to command buffer allocate info
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;														//Set level to primary (Primary buffers can be submitted for execution but cant be called by other buffers)
        allocInfo.commandPool = device->getCommandPool();															//Simular to a string pool, this is pre-allocated memorey for commands so we dont have to allocate memory a lot
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size()); //Tell the allocation info how many command buffers we are using

        //Allocate the command buffers
        if (vkAllocateCommandBuffers(device->getDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            DOG_CRITICAL("Failed to allocate command buffers");
        }
    }

    

    void RenderingResource::CreateSceneTexture()
    {
        VkExtent2D extent = swapChain->GetSwapChainExtent();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = device->GetLinearFormat();
        imageInfo.extent.width = extent.width;
        imageInfo.extent.height = extent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VkResult result = vmaCreateImage(allocator->GetAllocator(), &imageInfo, &allocInfo, &sceneImage, &sceneImageAllocation, nullptr);
        if (result != VK_SUCCESS)
        {
            DOG_CRITICAL("VMA failed to create scene image");
        }

        VkImageViewCreateInfo sampledViewInfo{};
        sampledViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        sampledViewInfo.image = sceneImage;
        sampledViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        sampledViewInfo.format = device->GetLinearFormat();
        sampledViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        sampledViewInfo.subresourceRange.baseMipLevel = 0;
        sampledViewInfo.subresourceRange.levelCount = 1;
        sampledViewInfo.subresourceRange.baseArrayLayer = 0;
        sampledViewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device->getDevice(), &sampledViewInfo, nullptr, &sceneImageView) != VK_SUCCESS)
        {
            DOG_CRITICAL("Failed to create scene image view");
        }

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

        if (vkCreateSampler(device->getDevice(), &samplerInfo, nullptr, &sceneSampler) != VK_SUCCESS)
        {
            DOG_CRITICAL("Failed to create scene sampler");
        }
        
        sceneTextureDescriptorSet = ImGui_ImplVulkan_AddTexture(sceneSampler, sceneImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void RenderingResource::CleanupSceneTexture()
    {
        sceneTextureDescriptorSet = VK_NULL_HANDLE;

        if (sceneSampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device->getDevice(), sceneSampler, nullptr);
            sceneSampler = VK_NULL_HANDLE;
        }

        if (sceneImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device->getDevice(), sceneImageView, nullptr);
            sceneImageView = VK_NULL_HANDLE;
        }

        if (sceneImage != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator->GetAllocator(), sceneImage, sceneImageAllocation);
            sceneImage = VK_NULL_HANDLE;
            sceneImageAllocation = VK_NULL_HANDLE;
        }
    }

    void RenderingResource::RecreateSceneTexture()
    {
        CleanupSceneTexture();
        CreateSceneTexture();
    }

    void RenderingResource::CreateDepthBuffer()
    {
        VkExtent2D extent = swapChain->GetSwapChainExtent();

        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = depthFormat;
        imageInfo.extent = { extent.width, extent.height, 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        vmaCreateImage(allocator->GetAllocator(), &imageInfo, &allocInfo, &mDepthImage, &mDepthImageAllocation, nullptr);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = mDepthImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(device->getDevice(), &viewInfo, nullptr, &mDepthImageView);
    }

    void RenderingResource::CleanupDepthBuffer()
    {
        if (mDepthImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device->getDevice(), mDepthImageView, nullptr);
            mDepthImageView = VK_NULL_HANDLE;
        }
        if (mDepthImage != VK_NULL_HANDLE)
        {
            vmaDestroyImage(allocator->GetAllocator(), mDepthImage, mDepthImageAllocation);
            mDepthImage = VK_NULL_HANDLE;
            mDepthImageAllocation = VK_NULL_HANDLE;
        }
    }

    void RenderingResource::RecreateDepthBuffer()
    {
        CleanupDepthBuffer();
        CreateDepthBuffer();
    }

    void RenderingResource::RecreateAllSceneTextures()
    {
        RecreateSceneTexture();
        RecreateDepthBuffer();
    }

    VkFormat RenderingResource::ToLinearFormat(VkFormat format)
    {
        if (format == VK_FORMAT_R8G8B8A8_SRGB) {
            return VK_FORMAT_R8G8B8A8_UNORM;
        }
        if (format == VK_FORMAT_B8G8R8A8_SRGB) {
            return VK_FORMAT_B8G8R8A8_UNORM;
        }

        DOG_CRITICAL("Unsupported format for sRGB conversion");
        return format;
    }

    void RenderingResource::LoadAnimations()
    {
        /*for (int i = 0; i < modelLibrary->GetModelCount(); i++)
        {
            Model* model = modelLibrary->GetModel(i);
            if (model->HasAnimations())
            {
                
            }
        }*/
    }
}
