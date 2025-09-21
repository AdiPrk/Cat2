#include <PCH/pch.h>
#include "EditorSystem.h"
#include "../ECS.h"
#include "../Entities/Components.h"

#include "../Resources/RenderingResource.h"
#include "../Resources/WindowResource.h"
#include "../Resources/EditorResource.h"
#include "../Resources/SerializationResource.h"

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
		rr->RecreateAllSceneTextures();
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
		
		rr->renderGraph->AddPass(
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

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save"))
				{
					// Trigger the save action here
					ecs->GetResource<SerializationResource>()->Serialize("assets/scenes/scene.json");
				}
                if (ImGui::MenuItem("Load"))
                {
                    // Trigger the load action here
                    ecs->GetResource<SerializationResource>()->Deserialize("assets/scenes/scene.json");
                }
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		RenderSceneWindow();
		RenderEntitiesWindow(); 
		RenderInspectorWindow();
        RenderSceneControls();

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
		ImGui::Image(reinterpret_cast<void*>(sceneTextureDescriptorSet), viewportSize);
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

    template<typename T, typename UIFunction>
    static void DrawComponentUI(const char* name, Entity entity, UIFunction uiFunction)
    {
        // Ensure the entity has the component before proceeding
        if (!entity.HasComponent<T>())
        {
            return;
        }

        // Use a consistent set of flags for a modern, clean look
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
            | ImGuiTreeNodeFlags_Framed
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_AllowItemOverlap
            | ImGuiTreeNodeFlags_FramePadding;

        auto& component = entity.GetComponent<T>();
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
        float lineHeight = ImGui::GetFrameHeight();
        ImGui::Separator();

        // Create a collapsible header for the component
        bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name);
        ImGui::PopStyleVar();

        // --- Component Settings Menu (the '...' button) ---
        // We add it on the same line, to the far right.
        ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
        if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
        {
            ImGui::OpenPopup("ComponentSettings");
        }

        bool componentRemoved = false;
        if (ImGui::BeginPopup("ComponentSettings"))
        {
            // Add a "Remove" option to the menu
            if (ImGui::MenuItem("Remove Component"))
            {
                componentRemoved = true;
            }
            ImGui::EndPopup();
        }

        // If the collapsible header is open, draw the component's properties
        if (open)
        {
            uiFunction(component);
            ImGui::TreePop();
        }

        // Defer removal until after drawing to avoid issues
        if (componentRemoved)
        {
            // We prevent the removal of essential components like Tag or Transform
            if constexpr (!std::is_same_v<T, TagComponent> && !std::is_same_v<T, TransformComponent>)
            {
                entity.RemoveComponent<T>();
            }
        }
    }

    // Helper for the "Add Component" dropdown to avoid repetition
    template<typename T>
    void DisplayAddComponentEntry(const char* name, Entity entity)
    {
        if (!entity.HasComponent<T>())
        {
            if (ImGui::MenuItem(name))
            {
                entity.AddComponent<T>();
                ImGui::CloseCurrentPopup();
            }
        }
    }

    // --- Main Inspector Window Function ---
    void EditorSystem::RenderInspectorWindow()
    {
        ImGui::Begin("Inspector");

        Entity selectedEnt = ecs->GetResource<EditorResource>()->selectedEntity;

        // Handle the case where no entity is selected
        if (!selectedEnt)
        {
            ImGui::Text("No entity selected.");
            ImGui::End();
            return;
        }

        // --- Tag Component (Special case, always at the top) ---
        if (selectedEnt.HasComponent<TagComponent>())
        {
            auto& tag = selectedEnt.GetComponent<TagComponent>();
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), tag.Tag.c_str());

            // A larger font for the name makes it feel like a title
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Assumes a larger font is at index 0
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag.Tag = std::string(buffer);
            }
            ImGui::PopFont();
        }

        // --- Draw Each Component using the Helper ---
        // This is now much cleaner and easier to read.
        DrawComponentUI<TransformComponent>("Transform", selectedEnt, [](auto& component)
            {
                // Displaying rotation in degrees is much more user-friendly!
                glm::vec3 rotationDegrees = glm::degrees(component.Rotation);

                ImGui::DragFloat3("Translation", glm::value_ptr(component.Translation), 0.1f);
                if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotationDegrees), 0.1f))
                {
                    component.Rotation = glm::radians(rotationDegrees);
                }
                ImGui::DragFloat3("Scale", glm::value_ptr(component.Scale), 0.1f);
            });

        DrawComponentUI<ModelComponent>("Model", selectedEnt, [](auto& component)
            {
                ImGui::InputInt("Model Index", (int*)&component.ModelIndex);
            });

        DrawComponentUI<AnimationComponent>("Animation", selectedEnt, [](auto& component)
            {
                ImGui::Checkbox("Is Playing", &component.IsPlaying);
                ImGui::InputInt("Animation Index", (int*)&component.AnimationIndex);
                ImGui::DragFloat("Animation Time", &component.AnimationTime, 0.05f, 0.0f, FLT_MAX);
            });

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- Add Component Dropdown ---
        // Center the button for better visual appeal
        float buttonWidth = ImGui::GetContentRegionAvail().x * 0.8f;
        float offset = (ImGui::GetContentRegionAvail().x - buttonWidth) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

        if (ImGui::Button("Add Component", ImVec2(buttonWidth, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            DisplayAddComponentEntry<ModelComponent>("Model", selectedEnt);
            DisplayAddComponentEntry<AnimationComponent>("Animation", selectedEnt);
            // Add more component types here as you create them!

            ImGui::EndPopup();
        }

        ImGui::End(); // End of Inspector window
    }

    // This function could be part of your SceneHierarchyPanel or a general editor UI function
    void EditorSystem::RenderSceneControls()
    {
        ImGui::Begin("Scene Controls"); // Or whatever window you prefer

        // --- Add Entity Button ---
        if (ImGui::Button("Add Entity", ImVec2(-1, 0))) // -1 width makes it fill the space
        {
            Entity newEntity = ecs->AddEntity("New Entity");
            // Optionally, select the newly created entity immediately
            ecs->GetResource<EditorResource>()->selectedEntity = newEntity;
        }

        ImGui::End();
    }
}