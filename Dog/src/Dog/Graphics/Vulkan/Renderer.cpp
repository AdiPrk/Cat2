#include <PCH/pch.h>
#include "Renderer.h"
#include "Systems/IndirectRenderer.h"
#include "Descriptors/Descriptors.h"
#include "Texture/TextureLibrary.h"
#include "Models/ModelLibrary.h"
#include "Core/SwapChain.h"
#include "Input/KeyboardController.h"
#include "Input/input.h"

#include "Engine.h"

#include "Graphics/Editor/Editor.h"

#include "Scene/Scene.h"
#include "Scene/Entity/Components.h"
#include "Scene/Entity/Entity.h"

//anim
#include "Animation/Animation.h"

namespace Dog {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
        int textureIndex{ 0 };
    };

    Renderer::Renderer(Window& window, Device& device)
        : m_Window{ window }
        , device{ device }
        , globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        , uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT)
        , bonesUboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT)
    {
        recreateSwapChain();
        createCommandBuffers();

        globalPool =
            DescriptorPool::Builder(device)
            .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, TextureLibrary::MAX_TEXTURE_COUNT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
            .Build();

        glslang::InitializeProcess();

        Input::Init(m_Window.getGLFWwindow());
    }

    Renderer::~Renderer() {
        freeCommandBuffers();
        glslang::FinalizeProcess();
    }

    void Renderer::Init()
    {
        auto& textureLibrary = Engine::Get().GetTextureLibrary();
        auto& modelLibrary = Engine::Get().GetModelLibrary();

        textureLibrary.AddTexture("assets/textures/square.png");
        textureLibrary.AddTexture("assets/textures/texture.jpg");
        textureLibrary.AddTexture("assets/textures/dog.png");
        textureLibrary.AddTexture("assets/textures/viking_room.png");
        textureLibrary.AddTexture("assets/models/ModelTextures/Book.png");

        modelLibrary.AddModel("assets/models/quad.obj");
        // modelLibrary.AddModel("assets/models/Adi_Dancing.fbx");
        
        //modelLibrary.AddModel("assets/models/charles.glb");
        //modelLibrary.AddModel("assets/models/dragon.obj");
        //modelLibrary.AddModel("assets/models/AlisaMikhailovna.fbx");
        //modelLibrary.AddModel("assets/models/Mon_BlackDragon31_Skeleton.FBX");
        //modelLibrary.AddModel("assets/models/Book.fbx");
        //modelLibrary.AddModel("assets/models/smooth_vase.obj");
        //modelLibrary.AddModel("assets/models/viking_room.obj");

        for (size_t i = 0; i < uboBuffers.size(); i++) {
            uboBuffers[i] = std::make_unique<Buffer>(
                device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY);
            uboBuffers[i]->map();
        }

        for (size_t i = 0; i < bonesUboBuffers.size(); i++) {
            bonesUboBuffers[i] = std::make_unique<Buffer>(
                device,
                sizeof(BonesUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY);
            bonesUboBuffers[i]->map();
        }

        auto globalSetLayout =
            DescriptorSetLayout::Builder(device)
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, TextureLibrary::MAX_TEXTURE_COUNT)
            .AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .Build();

        // Atleast 1 texture must be added by this point, or uh-oh.
        // Set up image infos for the descriptor set
        std::vector<VkDescriptorImageInfo> imageInfos(TextureLibrary::MAX_TEXTURE_COUNT);
        for (size_t j = 0; j < TextureLibrary::MAX_TEXTURE_COUNT; j++) {
            if (j < textureLibrary.getTextureCount()) {
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[j].imageView = textureLibrary.getTextureByIndex(j).getImageView();
                imageInfos[j].sampler = textureLibrary.GetTextureSampler();
            }
            else {
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[j].imageView = textureLibrary.getTextureByIndex(0).getImageView();
                imageInfos[j].sampler = textureLibrary.GetTextureSampler();
            }
        }

        // Create descriptor sets
        for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            auto boneBufferInfo = bonesUboBuffers[i]->descriptorInfo();

            DescriptorWriter(*globalSetLayout, *globalPool)
                .WriteBuffer(0, &bufferInfo)
                .WriteImage(1, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()))
                .WriteBuffer(2, &boneBufferInfo)
                .Build(globalDescriptorSets[i]);
        }

        simpleRenderSystem = std::make_unique<IndirectRenderer>(
			device,
			getSwapChainRenderPass(),
			globalSetLayout->GetDescriptorSetLayout(),
			textureLibrary,
			modelLibrary);

        // Temporary camera controller
        cameraController = std::make_unique<KeyboardMovementController>();
    }

    void Renderer::Render(float dt)
    {
        auto& textureLibrary = Engine::Get().GetTextureLibrary();
        auto& modelLibrary = Engine::Get().GetModelLibrary();
        textureLibrary.UpdateTextures();

        //animator->UpdateAnimation(dt);

        // get entity with camera component
        Scene* scene = SceneManager::GetCurrentScene();
        entt::registry& registry = scene->GetRegistry();

        GlobalUbo ubo{};

        registry.view<TransformComponent, CameraComponent>().each
        ([&](const auto& entity, TransformComponent& transform, CameraComponent& camera)
        {
            ubo.projection = camera.GetProjection();
            ubo.view = camera.GetView();
            ubo.inverseView = camera.GetInverseView();

            cameraController->moveInPlaneXZ(m_Window.getGLFWwindow(), dt, transform);
            camera.SetViewYXZ(transform.Translation, transform.Rotation);
            
            // Set the camera's projection
            float aspect = getAspectRatio();
            camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 8000.f);
        });

        // Start the frame
        if (auto commandBuffer = beginFrame()) {
            Engine::Get().GetEditor().BeginFrame();

            int frameIndex = getFrameIndex();
            FrameInfo frameInfo{
                frameIndex,
                dt,
                commandBuffer,
                globalDescriptorSets[frameIndex]};

            // update
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            // render
            beginSwapChainRenderPass(commandBuffer);
            simpleRenderSystem->render(frameInfo);
           
            Engine::Get().GetEditor().EndFrame(commandBuffer);

            endSwapChainRenderPass(commandBuffer);
            endFrame();
        }
    }

    void Renderer::Exit()
    {
        // Wait until the device is idle before cleaning up resources
		vkDeviceWaitIdle(device);
    }

    void Renderer::recreateSwapChain() {
        auto extent = m_Window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = m_Window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device);

        if (m_SwapChain == nullptr) {
            m_SwapChain = std::make_unique<SwapChain>(device, extent);
        }
        else {
            std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain);
            m_SwapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*m_SwapChain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void Renderer::createCommandBuffers() {
        commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
            device,
            device.getCommandPool(),
            static_cast<uint32_t>(commandBuffers.size()),
            commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer Renderer::beginFrame() {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        auto result = m_SwapChain->acquireNextImage(&currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return commandBuffer;
    }

    void Renderer::endFrame() {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");

        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = m_SwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            m_Window.wasWindowResized()) {
            m_Window.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(
            commandBuffer == getCurrentCommandBuffer() &&
            "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_SwapChain->getRenderPass();
        renderPassInfo.framebuffer = m_SwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{ {0, 0}, m_SwapChain->getSwapChainExtent() };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }

} // namespace Dog
