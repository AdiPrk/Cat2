#include <PCH/pch.h>
#include "RenderSystem.h"

#include "../Resources/renderingResource.h"
#include "../Resources/EditorResource.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Vulkan/Core/SwapChain.h"
#include "Graphics/Vulkan/Model/ModelLibrary.h"
#include "Graphics/Vulkan/Uniform/ShaderTypes.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"
#include "Graphics/Vulkan/Model/Model.h"
#include "Graphics/Vulkan/Uniform/Uniform.h"
#include "Graphics/Vulkan/RenderGraph.h"
#include "Graphics/Vulkan/Model/Animation/AnimationLibrary.h"
#include "Graphics/Vulkan/Model/Animation/Animator.h"


#include "../ECS.h"
#include "ECS/Entities/Entity.h"
#include "ECS/Entities/Components.h"

namespace Dog
{
    RenderSystem::~RenderSystem()
    {
    }

    void RenderSystem::Init()
    {
        {
            ecs->AddEntity("Hii");

            Entity ent = ecs->GetEntity("Hii");
            if (ent)
            {
                ModelComponent& mc = ent.AddComponent<ModelComponent>();
                mc.ModelIndex = 0;

                TransformComponent& tc = ent.GetComponent<TransformComponent>();
                tc.Translation = glm::vec3(-0.0f, -0.8f, -2.0f);
                tc.Scale = glm::vec3(0.08f);

                CameraComponent& cc = ent.AddComponent<CameraComponent>();
            }
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
    
    void RenderSystem::Update(float dt)
    {
        auto renderingResource = ecs->GetResource<RenderingResource>();
        auto editorResource = ecs->GetResource<EditorResource>();
        auto& rg = renderingResource->renderGraph;

        // Set camera uniform!
        CameraUniforms camData{};
        camData.view = glm::mat4(1.0f);
        camData.projection = glm::perspective(glm::radians(45.0f), editorResource->sceneWindowWidth / editorResource->sceneWindowHeight, 0.1f, 100.0f);
        camData.projection[1][1] *= -1;
        camData.projectionView = camData.projection * camData.view;
        renderingResource->cameraUniform->SetUniformData(camData, 0, renderingResource->currentFrameIndex);
        
        auto& al = renderingResource->animationLibrary;
        al->GetAnimator(0)->UpdateAnimation(dt);
        
        // Add the scene render pass
        rg->AddPass(
            "ScenePass",
            [&](RGPassBuilder& builder) {
                builder.writes("SceneColor");
                builder.writes("SceneDepth");
            },
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

        // loop over entties with model and transform component
        std::vector<InstanceUniforms> instanceData{};

        auto& registry = ecs->GetRegistry();
        auto entityView = registry.view<ModelComponent, TransformComponent>();

        for (auto& entityHandle : entityView)
        {
            Entity entity(&registry, entityHandle);
            ModelComponent& mc = entity.GetComponent<ModelComponent>();
            TransformComponent& tc = entity.GetComponent<TransformComponent>();
            model = renderingResource->modelLibrary->GetModel(mc.ModelIndex);
            
            for (auto& mesh : model->mMeshes)
            {
                InstanceUniforms& data = instanceData.emplace_back();
                data.model = tc.GetTransform();
                data.textureIndex = mesh.diffuseTextureIndex;
            }
        }
        
        renderingResource->instanceUniform->SetUniformData(instanceData, 1, renderingResource->currentFrameIndex);

        // Update animations!
        auto& al = renderingResource->animationLibrary;
        auto& finalBones = al->GetAnimator(0)->GetFinalBoneMatrices();

        // put bones in vector
        std::vector<glm::mat4> finalBonesVec(finalBones.begin(), finalBones.end());

        renderingResource->instanceUniform->SetUniformData(finalBonesVec, 2, renderingResource->currentFrameIndex);

        uint32_t baseInstance = 0;
        for (auto& entityHandle : entityView)
        {
            ModelComponent& mc = registry.get<ModelComponent>(entityHandle);
            model = renderingResource->modelLibrary->GetModel(mc.ModelIndex);

            for (auto& mesh : model->mMeshes)
            {
                mesh.Bind(cmd);
                mesh.Draw(cmd, baseInstance++);
            }
        }
    }
}
