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
            ecs->AddEntity("Camera");

            Entity ent = ecs->GetEntity("Camera");
            if (ent)
            {
                TransformComponent& tc = ent.GetComponent<TransformComponent>();
                tc.Translation = glm::vec3(-0.5f, -0.8f, -2.0f);

                CameraComponent& cc = ent.AddComponent<CameraComponent>();
            }
        }
        {
            ecs->AddEntity("Hii");

            Entity ent = ecs->GetEntity("Hii");
            if (ent)
            {
                ModelComponent& mc = ent.AddComponent<ModelComponent>();
                mc.ModelIndex = 0;

                AnimationComponent& ac = ent.AddComponent<AnimationComponent>();
                ac.AnimationIndex = 0;

                TransformComponent& tc = ent.GetComponent<TransformComponent>();
                tc.Translation = glm::vec3(-0.5f, -0.8f, -2.0f);
            }
        }
        {
            ecs->AddEntity("Hii2");

            Entity ent = ecs->GetEntity("Hii2");
            if (ent)
            {
                ModelComponent& mc = ent.AddComponent<ModelComponent>();
                mc.ModelIndex = 0;

                TransformComponent& tc = ent.GetComponent<TransformComponent>();
                tc.Translation = glm::vec3(0.5f, -0.8f, -2.0f);
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
        {
            CameraUniforms camData{};

            // get camera entity
            Entity cameraEntity = ecs->GetEntity("Camera");
            if (cameraEntity) 
            {
                TransformComponent& tc = cameraEntity.GetComponent<TransformComponent>();
                CameraComponent& cc = cameraEntity.GetComponent<CameraComponent>();
                glm::vec3 forward = cc.Forward;
                glm::vec3 up = cc.Up;
                glm::vec3 position = tc.Translation;
                camData.view = glm::lookAt(position, position + forward, up);
            }
            else
            {
                camData.view = glm::mat4(1.0f);
            }

            camData.projection = glm::perspective(glm::radians(45.0f), editorResource->sceneWindowWidth / editorResource->sceneWindowHeight, 0.1f, 100.0f);
            camData.projection[1][1] *= -1;
            camData.projectionView = camData.projection * camData.view;
            renderingResource->cameraUniform->SetUniformData(camData, 0, renderingResource->currentFrameIndex);

        }
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

        // loop over entties with model and transform component
        std::vector<InstanceUniforms> instanceData{};

        auto& registry = ecs->GetRegistry();
        auto entityView = registry.view<ModelComponent, TransformComponent>();

        AnimationLibrary* al = renderingResource->animationLibrary.get();
        for (auto& entityHandle : entityView)
        {
            Entity entity(&registry, entityHandle);
            ModelComponent& mc = entity.GetComponent<ModelComponent>();
            TransformComponent& tc = entity.GetComponent<TransformComponent>();
            AnimationComponent* ac = entity.HasComponent<AnimationComponent>() ? &entity.GetComponent<AnimationComponent>() : nullptr;
            Model* model = renderingResource->modelLibrary->GetModel(mc.ModelIndex);
            if (!model) continue;
            
            uint32_t boneOffset = AnimationLibrary::INVALID_ANIMATION_INDEX;
            if (ac && al->GetAnimation(ac->AnimationIndex) && al->GetAnimator(ac->AnimationIndex))
            {
                boneOffset = ac->BoneOffset;
            }

            for (auto& mesh : model->mMeshes)
            {
                InstanceUniforms& data = instanceData.emplace_back();
                if (boneOffset == AnimationLibrary::INVALID_ANIMATION_INDEX)
                {
                    data.model = model->mNormalizationMatrix * tc.GetTransform();
                }
                else
                {
                    data.model = tc.GetTransform();
                }
                    
                data.textureIndex = mesh.diffuseTextureIndex;
                data.boneOffset = boneOffset;
            }
        }
        
        renderingResource->instanceUniform->SetUniformData(instanceData, 1, renderingResource->currentFrameIndex);

        uint32_t baseInstance = 0;
        for (auto& entityHandle : entityView)
        {
            ModelComponent& mc = registry.get<ModelComponent>(entityHandle);
            Model* model = renderingResource->modelLibrary->GetModel(mc.ModelIndex);
            if (!model) continue;

            for (auto& mesh : model->mMeshes)
            {
                mesh.Bind(cmd);
                mesh.Draw(cmd, baseInstance++);
            }
        }
    }
}
