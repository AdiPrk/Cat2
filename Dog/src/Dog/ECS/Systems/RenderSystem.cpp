#include <PCH/pch.h>
#include "RenderSystem.h"

#include "../Resources/renderingResource.h"
#include "../Resources/EditorResource.h"
#include "../Resources/DebugDrawResource.h"

#include "InputSystem.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Vulkan/Core/SwapChain.h"
#include "Graphics/Vulkan/Model/ModelLibrary.h"
#include "Graphics/Vulkan/Uniform/ShaderTypes.h"
#include "Graphics/Vulkan/Pipeline/Pipeline.h"
#include "Graphics/Vulkan/Model/Model.h"
#include "Graphics/Vulkan/Uniform/Uniform.h"
#include "Graphics/Vulkan/RenderGraph.h"
#include "Graphics/Vulkan/Model/Animation/AnimationLibrary.h"
#include "Graphics/Vulkan/Texture/TextureLibrary.h"
#include "Graphics/Vulkan/Texture/Texture.h"
#include "Graphics/Vulkan/Uniform/Descriptors.h"

#include "../ECS.h"
#include "ECS/Entities/Entity.h"
#include "ECS/Entities/Components.h"

namespace Dog
{
    RenderSystem::RenderSystem() : ISystem("RenderSystem") {}
    RenderSystem::~RenderSystem() {}

    void RenderSystem::Init()
    {
        auto rr = ecs->GetResource<RenderingResource>();

        std::vector<Uniform*> unis{
            rr->cameraUniform.get(),
            rr->instanceUniform.get()
        };

        mPipeline = std::make_unique<Pipeline>(
            *rr->device,
            rr->swapChain->GetImageFormat(), rr->swapChain->FindDepthFormat(),
            unis,
            false,
            "basic_model.vert", "basic_model.frag"
        );

        mWireframePipeline = std::make_unique<Pipeline>(
            *rr->device,
            rr->swapChain->GetImageFormat(), rr->swapChain->FindDepthFormat(),
            unis,
            true,
            "basic_model.vert", "basic_model.frag"
        );
    }
    
    void RenderSystem::FrameStart()
    {
        auto rr = ecs->GetResource<RenderingResource>();
        auto& ml = rr->modelLibrary;
        if (ml->NeedsTextureUpdate())
        {
            ml->LoadTextures();

            auto& tl = rr->textureLibrary;

            size_t textureCount = tl->GetTextureCount();
            std::vector<VkDescriptorImageInfo> imageInfos(TextureLibrary::MAX_TEXTURE_COUNT);

            VkSampler defaultSampler = tl->GetSampler();

            bool hasTex = textureCount > 0;
            for (size_t j = 0; j < TextureLibrary::MAX_TEXTURE_COUNT; ++j) {
                imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos[j].sampler = defaultSampler;
                imageInfos[j].imageView = hasTex ? tl->GetTextureByIndex(static_cast<uint32_t>(std::min(j, textureCount - 1))).GetImageView() : 0;
            }

            for (int frameIndex = 0; frameIndex < SwapChain::MAX_FRAMES_IN_FLIGHT; ++frameIndex) {
                DescriptorWriter writer(*rr->instanceUniform->GetDescriptorLayout(), *rr->instanceUniform->GetDescriptorPool());
                writer.WriteImage(0, imageInfos.data(), static_cast<uint32_t>(imageInfos.size()));
                writer.Overwrite(rr->instanceUniform->GetDescriptorSets()[frameIndex]);
            }
        }

        DebugDrawResource::Clear();
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
                
                // Get the position directly
                glm::vec3 cameraPos = tc.Translation;

                // Calculate direction vectors from the rotation quaternion
                glm::vec3 forwardDir = glm::normalize(cc.Forward);
                glm::vec3 upDir = glm::normalize(cc.Up);

                // Calculate the target and call lookAt
                glm::vec3 cameraTarget = cameraPos + forwardDir;
                camData.view = glm::lookAt(cameraPos, cameraTarget, upDir);
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
        mWireframePipeline.reset();
    }

    void RenderSystem::RenderScene(VkCommandBuffer cmd)
    {
        auto rr = ecs->GetResource<RenderingResource>();

        rr->renderWireframe ? mWireframePipeline->Bind(cmd) : mPipeline->Bind(cmd);
        VkPipelineLayout pipelineLayout = rr->renderWireframe ? mWireframePipeline->GetLayout() : mPipeline->GetLayout();

        rr->cameraUniform->Bind(cmd, pipelineLayout, rr->currentFrameIndex);
        rr->instanceUniform->Bind(cmd, pipelineLayout, rr->currentFrameIndex);

        VkViewport viewport{};
        viewport.width = static_cast<float>(rr->swapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(rr->swapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);

        VkRect2D scissor{ {0, 0}, rr->swapChain->GetSwapChainExtent() };
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        // loop over entties with model and transform component
        std::vector<InstanceUniforms> instanceData{};

        auto& registry = ecs->GetRegistry();
        auto entityView = registry.view<ModelComponent, TransformComponent>();

        AnimationLibrary* al = rr->animationLibrary.get();
        for (auto& entityHandle : entityView)
        {
            Entity entity(&registry, entityHandle);
            ModelComponent& mc = entity.GetComponent<ModelComponent>();
            TransformComponent& tc = entity.GetComponent<TransformComponent>();
            AnimationComponent* ac = entity.HasComponent<AnimationComponent>() ? &entity.GetComponent<AnimationComponent>() : nullptr;
            Model* model = rr->modelLibrary->GetModel(mc.ModelPath);
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
                    data.model = tc.GetTransform() * model->mNormalizationMatrix;
                }
                else
                {
                    data.model = tc.GetTransform();
                }

                data.tint = mc.tintColor;
                data.textureIndex = mesh.diffuseTextureIndex;
                data.boneOffset = boneOffset;
            }
        }

        // RenderSkeleton();

        auto instDatas = DebugDrawResource::GetInstanceData();

        if (InputSystem::isKeyDown(Key::G))
        {
            instanceData.clear();
        }

        instanceData.insert(instanceData.begin(), instDatas.begin(), instDatas.end());
        
        rr->instanceUniform->SetUniformData(instanceData, 1, rr->currentFrameIndex);

        uint32_t baseInstance = 0;

        for (auto& data : instDatas)
        {
            Model* model = rr->modelLibrary->GetModel(1);

            for (auto& mesh : model->mMeshes)
            {
                mesh.Bind(cmd);
                mesh.Draw(cmd, baseInstance++);
            }
        }

        if (!InputSystem::isKeyDown(Key::G))
        {
            for (auto& entityHandle : entityView)
            {
                ModelComponent& mc = registry.get<ModelComponent>(entityHandle);
                Model* model = rr->modelLibrary->GetModel(mc.ModelPath);
                if (!model) continue;

                uint32_t numVerts = 0;
                for (auto& mesh : model->mMeshes)
                {
                    mesh.Bind(cmd);
                    mesh.Draw(cmd, baseInstance++);
                    numVerts += mesh.mVertexCount;
                }

                //printf("Num verts for %s is %i\n", model->GetName(), numVerts);
            }
        }

        
    }

    void RenderSystem::RenderSkeleton()
    {
        // loop over entities with models
        auto& registry = ecs->GetRegistry();
        auto entityView = registry.view<ModelComponent, TransformComponent>();

        for (auto& entityHandle : entityView)
        {
            Entity entity(&registry, entityHandle);
            ModelComponent& mc = entity.GetComponent<ModelComponent>();
            TransformComponent& tc = entity.GetComponent<TransformComponent>();
            //AnimationComponent* ac = entity.HasComponent<AnimationComponent>() ? &entity.GetComponent<AnimationComponent>() : nullptr;
            Model* model = ecs->GetResource<RenderingResource>()->modelLibrary->GetModel(mc.ModelPath);
            if (!model) continue;

            glm::mat4 normalizationMatrix = model->mNormalizationMatrix;
            //if (!ac) normalizationMatrix = glm::mat4(1.f);

            const aiScene* scene = model->mScene;
            RecursiveNodeDraw(tc.GetTransform() * normalizationMatrix, scene->mRootNode);
        }
    }

    void RenderSystem::RecursiveNodeDraw(const glm::mat4& parentWorldTransform, const aiNode* node)
    {
        glm::mat4 localTr = aiMatToGlm(node->mTransformation);
        glm::mat4 worldTr = parentWorldTransform * localTr;

        glm::vec3 startPos = glm::vec3(parentWorldTransform[3]);
        glm::vec3 endPos = glm::vec3(worldTr[3]);

        if (node->mParent && node->mParent->mParent != nullptr)
        {
            DebugDrawResource::DrawLine(startPos, endPos, glm::vec4(1.f, 0.f, 1.f, 1.f));
            DebugDrawResource::DrawCube(endPos, glm::vec3(0.01f), glm::vec4(0.f, 1.f, 1.f, 0.4f));
        }


        // Recurse :3
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            RecursiveNodeDraw(worldTr, node->mChildren[i]);
        }
    }
}
