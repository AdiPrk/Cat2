#include <PCH/pch.h>
#include "EditorSystem.h"
#include "../ECS.h"
#include "../Entities/Components.h"

#include "../Resources/RenderingResource.h"
#include "../Resources/WindowResource.h"
#include "../Resources/EditorResource.h"

#include "Graphics/Vulkan/Core/Device.h"
#include "Graphics/Vulkan/Core/SwapChain.h"
#include "Graphics/Vulkan/RenderGraph.h"

#include "Graphics/Window/Window.h"

namespace Dog
{
    void EditorSystem::Init()
    {
		InitImGui();

        auto rr = ecs->GetResource<RenderingResource>();
		rr->CreateSceneTexture();
		rr->CreateDepthBuffer();
    }

    void EditorSystem::FrameStart()
    {
    }

    void EditorSystem::Update(float dt)
    {
		auto rr = ecs->GetResource<RenderingResource>();
		if (!rr)
		{
			DOG_CRITICAL("No rendering resource in editor system");
			return;
		}
		
		rr->renderGraph->add_pass(
			"ImGuiPass",
			[&](RGPassBuilder& builder) {
				builder.reads("SceneColor");
				builder.writes("BackBuffer");
			},
			std::bind(&EditorSystem::RenderImGui, this, std::placeholders::_1)
		);
    }

    void EditorSystem::FrameEnd()
    {
    }

	void EditorSystem::InitImGui()
	{
		
	}

    void EditorSystem::Exit()
    {
        auto rr = ecs->GetResource<RenderingResource>();
        auto er = ecs->GetResource<EditorResource>();
        Device& device = *rr->device;

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorSetLayout(device, er->samplerSetLayout, nullptr);
		vkDestroyDescriptorPool(device, er->descriptorPool, nullptr);

		rr->CleanupSceneTexture();
        rr->CleanupDepthBuffer();
    }

	void EditorSystem::RenderImGui(VkCommandBuffer cmd)
	{
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		RenderSceneWindow();
		RenderEntitiesWindow(); 
		RenderInspectorWindow();

		// Rendering
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void EditorSystem::RenderSceneWindow()
	{
		// Create a window and display the scene texture
		VkDescriptorSet sceneTextureDescriptorSet = ecs->GetResource<RenderingResource>()->sceneTextureDescriptorSet;

		ImGui::Begin("Viewport");
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(sceneTextureDescriptorSet, viewportSize);
		ImGui::End();

		auto editorResource = ecs->GetResource<EditorResource>();
		editorResource->sceneWindowWidth = viewportSize.x;
		editorResource->sceneWindowHeight = viewportSize.y;
	}

	void EditorSystem::RenderEntitiesWindow()
	{
		ImGui::Begin("Entities##window", nullptr, ImGuiWindowFlags_MenuBar);

		// Iterate through all entities with a tag, and without a parent

		entt::registry& registry = ecs->GetRegistry();

		auto view = registry.view<TagComponent>();
		for (auto& entityHandle : view) 
		{
			Entity entity(&registry, entityHandle);
			TagComponent& tag = entity.GetComponent<TagComponent>();

			ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf;

			bool opened = false;
			opened = ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, leafFlags, tag.Tag.c_str());

			if (ImGui::IsItemClicked()) {
				DOG_INFO("Entity {0} Selected!", tag.Tag.c_str());
                ecs->GetResource<EditorResource>()->selectedEntity = entity;
			}

			// end treenodex
			if (opened) {
				ImGui::TreePop();
			}
		}

		ImGui::End(); // End of Entities
	}

	void EditorSystem::RenderInspectorWindow()
    {
        Entity selectedEnt = ecs->GetResource<EditorResource>()->selectedEntity;
		if (!selectedEnt)
		{
			return;
		}

        ImGui::Begin("Inspector");

        if (selectedEnt.HasComponent<TagComponent>())
        {
            TagComponent& tag = selectedEnt.GetComponent<TagComponent>();
			ImGui::InputText("Name##TagProp", &tag.Tag);
        }

		if (selectedEnt.HasComponent<TransformComponent>())
		{
			TransformComponent& transform = selectedEnt.GetComponent<TransformComponent>();
            ImGui::DragFloat3("Translation##TransformProp", glm::value_ptr(transform.Translation), 0.1f);
            ImGui::DragFloat3("Rotation##TransformProp", glm::value_ptr(transform.Rotation), 0.1f);
            ImGui::DragFloat3("Scale##TransformProp", glm::value_ptr(transform.Scale), 0.1f);
		}

        ImGui::End(); // End of Inspector
	}
}