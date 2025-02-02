#include <PCH/pch.h>
#include "IndirectRenderer.h"
#include "Engine.h"
#include "Scene/SceneManager.h"
#include "Scene/Scene.h"
#include "Scene/Entity/Components.h"
#include "../Pipeline/Pipeline.h"

namespace Dog {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat4 normalMatrix{ 1.f };
        int textureIndex{ 0 };
    };

    IndirectRenderer::IndirectRenderer(
        Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, TextureLibrary& textureLibrary, ModelLibrary& modelLibrary)
        : device{ device }
        , textureLibrary{ textureLibrary }
        , modelLibrary{ modelLibrary }
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    IndirectRenderer::~IndirectRenderer() {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

    void IndirectRenderer::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void IndirectRenderer::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

        std::string shader = "old_simple_shader";

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
        lvePipeline = std::make_unique<Pipeline>(
            device,
            shader + ".vert",
            shader + ".frag",
            pipelineConfig);

        // Using base pipeline is supposed to make pipeline creation more efficient
        // It should also make switching between the two pipelines more efficient
        pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
        pipelineConfig.rasterizationInfo.lineWidth = 1.0f;
        pipelineConfig.basePipelineIndex = -1; // -1 forces it to use basePipelineHandle (which must be valid)
        pipelineConfig.basePipelineHandle = lvePipeline->getPipeline();
        pipelineConfig.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        lveWireframePipeline = std::make_unique<Pipeline>(
            device,
            shader + ".vert",
            shader + ".frag",
            pipelineConfig);
    }

    void IndirectRenderer::render(FrameInfo& frameInfo) {
        /*static float frame = 0;
        static bool wireframe = false;
        frame += frameInfo.frameTime;
        if (frame > 3.f) {
            frame = 0;
            wireframe = !wireframe;
        }

        if (wireframe) {
            lveWireframePipeline->bind(frameInfo.commandBuffer);
        }
        else {
            lvePipeline->bind(frameInfo.commandBuffer);
        }*/

        lvePipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        Scene* scene = SceneManager::GetCurrentScene();
        entt::registry& registry = scene->GetRegistry();

        registry.view<TransformComponent, ModelComponent>().each
        ([&](const auto& entity, const TransformComponent& transform, const ModelComponent& model)
            {
                if (model.ModelIndex == ModelLibrary::INVALID_MODEL_INDEX) return;

                Model* pModel = modelLibrary.GetModelByIndex(model.ModelIndex);

                for (int i = 0; i < pModel->meshes.size(); ++i) {
                    auto& mesh = pModel->meshes[i];

                    SimplePushConstantData push{};
                    push.modelMatrix = transform.mat4();
                    push.normalMatrix = transform.normalMatrix();

                    // push.textureIndex = mesh.materialComponent.AlbedoTexture;
                    push.textureIndex = model.MaterialOverrides[i].AlbedoTexture;
                    
                    vkCmdPushConstants(
                        frameInfo.commandBuffer,
                        pipelineLayout,
                        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,
                        sizeof(SimplePushConstantData),
                        &push);

                    mesh.bind(frameInfo.commandBuffer);
                    mesh.draw(frameInfo.commandBuffer);
                }
            });
    }

} // namespace Dog