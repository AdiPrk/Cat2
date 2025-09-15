#include <PCH/pch.h>
#include "RenderSystem.h"

#include "../Resources/renderingResource.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Vulkan/Core/SwapChain.h"
#include "Graphics/Vulkan/Model/ModelLibrary.h"
#include "Graphics/Vulkan/Uniform/ShaderTypes.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"
#include "Graphics/Vulkan/Model/Model.h"
#include "Graphics/Vulkan/Uniform/Uniform.h"
#include "Graphics/Vulkan/RenderGraph.h"

#include "../ECS.h"
#include "ECS/Entities/Entity.h"
#include "ECS/Entities/Components.h"

namespace Dog
{
    void RenderSystem::Init()
    {
        ecs->AddEntity("Hii");

        Entity ent = ecs->GetEntity("Hii");
        if (ent)
        {
            ModelComponent& mc = ent.AddComponent<ModelComponent>();
            mc.ModelIndex = 0;
        }

        auto renderingResource = ecs->GetResource<RenderingResource>();

        std::vector<Uniform*> unis{
            renderingResource->cameraUniform.get(),
            renderingResource->instanceUniform.get()
        };

        mPipeline = std::make_unique<Pipeline>(
            *renderingResource->device,
            renderingResource->swapChain->GetImageFormat(),
            renderingResource->swapChain->FindDepthFormat(),
            unis,
            false,
            "basic_model.vert",
            "basic_model.frag"
        );
    }
    
    void RenderSystem::FrameStart()
    {
    }
    
    void RenderSystem::Update(float)
    {
        auto renderingResource = ecs->GetResource<RenderingResource>();
        auto& rg = renderingResource->mRenderGraph;

        // Set camera uniform!
        CameraUniforms camData{};
        camData.view = glm::mat4(1.0f);
        camData.projection = glm::perspective(glm::radians(45.0f), renderingResource->swapChain->GetSwapChainExtent().width / (float)renderingResource->swapChain->GetSwapChainExtent().height, 0.1f, 10.0f);
        camData.projection[1][1] *= -1;
        camData.projectionView = camData.projection * camData.view;
        renderingResource->cameraUniform->SetUniformData(camData, 0, renderingResource->currentFrameIndex);

        // Add the scene render pass
        rg->add_pass(
            "ScenePass",
            // Setup: This pass WRITES to the scene texture.
            [&](RGPassBuilder& builder) {
                builder.writes("SceneColor");
                builder.writes("SceneDepth");
            },
            // Execute:
            std::bind(&RenderSystem::RenderScene, this, std::placeholders::_1)
        );
    }
    
    void RenderSystem::FrameEnd()
    {
    }

    void RenderSystem::Exit()
    {
        mPipeline.reset();
    }

    void RenderSystem::RenderScene(VkCommandBuffer cmd)
    {
        auto renderingResource = ecs->GetResource<RenderingResource>();

        mPipeline->Bind(cmd);
        renderingResource->cameraUniform->Bind(cmd, mPipeline->GetLayout(), renderingResource->currentFrameIndex);
        renderingResource->instanceUniform->Bind(cmd, mPipeline->GetLayout(), renderingResource->currentFrameIndex);

        VkViewport viewport{};
        viewport.width = static_cast<float>(renderingResource->swapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(renderingResource->swapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{ {0, 0}, renderingResource->swapChain->GetSwapChainExtent() };
        vkCmdSetScissor(cmd, 0, 1, &scissor);


        Model* model = renderingResource->modelLibrary->GetModel(0);

        std::vector<InstanceUniforms> instanceData{};
        for (auto& mesh : model->mMeshes)
        {
            InstanceUniforms& data = instanceData.emplace_back();
            data.model = glm::mat4(1.0f);
            data.model = glm::translate(data.model, glm::vec3(0.0f, -0.75f, -2.0f));
            data.model = glm::scale(data.model, glm::vec3(0.1f));
            data.model = glm::rotate(data.model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            data.textureIndex = mesh.diffuseTextureIndex;
        }
        renderingResource->instanceUniform->SetUniformData(instanceData, 1, renderingResource->currentFrameIndex);

        uint32_t baseInstance = 0;
        for (auto& mesh : model->mMeshes)
        {
            mesh.Bind(cmd);
            mesh.Draw(cmd, baseInstance++);
        }
    }
}
