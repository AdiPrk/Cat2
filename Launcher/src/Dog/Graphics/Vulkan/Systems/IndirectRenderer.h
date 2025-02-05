#pragma once

#include "../Core/Device.h"
#include "../FrameInfo.h"
#include "../Pipeline/Pipeline.h"
#include "../Texture/TextureLibrary.h"
#include "../Models/ModelLibrary.h"

namespace Dog {

    class IndirectRenderer {
    public:
        IndirectRenderer(
            Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, TextureLibrary& textureLibrary, ModelLibrary& modelLibrary);
        ~IndirectRenderer();

        IndirectRenderer(const IndirectRenderer&) = delete;
        IndirectRenderer& operator=(const IndirectRenderer&) = delete;

        void render(FrameInfo& frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Device& device;
        TextureLibrary& textureLibrary;
        ModelLibrary& modelLibrary;

        std::unique_ptr<Pipeline> lvePipeline;
        std::unique_ptr<Pipeline> lveWireframePipeline;
        VkPipelineLayout pipelineLayout;
    };

} // namespace Dog
