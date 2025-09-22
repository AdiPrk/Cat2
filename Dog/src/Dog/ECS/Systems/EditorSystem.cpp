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
        auto er = ecs->GetResource<EditorResource>();
        if (er->entityToDelete)
        {
            if (er->selectedEntity == er->entityToDelete)
            {
                er->selectedEntity = {};
            }

            ecs->RemoveEntity(er->entityToDelete);
            er->entityToDelete = {};
        }
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

        ImGui::Begin("Debug");
        ImGui::Checkbox("Wireframe", &ecs->GetResource<RenderingResource>()->renderWireframe);
        ImGui::End();

		// Rendering
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

    void EditorSystem::RenderMainMenuBar()
    {
        if (!ImGui::BeginMainMenuBar()) return;
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save Scene"))
                {
                    ecs->GetResource<SerializationResource>()->Serialize("assets/scenes/scene.json");
                }
                if (ImGui::MenuItem("Load Scene"))
                {
                    ecs->GetResource<SerializationResource>()->Deserialize("assets/scenes/scene.json");
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void EditorSystem::RenderSceneWindow()
	{
		// Create a window and display the scene texture
		VkDescriptorSet sceneTextureDescriptorSet = ecs->GetResource<RenderingResource>()->sceneTextureDescriptorSet;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		ImGui::Image(reinterpret_cast<void*>(sceneTextureDescriptorSet), viewportSize);
		ImGui::End();
        ImGui::PopStyleVar();

		auto editorResource = ecs->GetResource<EditorResource>();
		editorResource->sceneWindowWidth = viewportSize.x;
		editorResource->sceneWindowHeight = viewportSize.y;
	}

	void EditorSystem::RenderEntitiesWindow()
	{
        auto er = ecs->GetResource<EditorResource>();

		ImGui::Begin("Entities##window", nullptr, ImGuiWindowFlags_MenuBar);
		entt::registry& registry = ecs->GetRegistry();
		auto view = registry.view<TagComponent>();

		for (auto& entityHandle : view) 
		{
			Entity entity(&registry, entityHandle);
			TagComponent& tag = entity.GetComponent<TagComponent>();

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (er->selectedEntity == entity)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, flags, tag.Tag.c_str());
            if (ImGui::IsItemClicked())
            {
                er->selectedEntity = entity;
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Remove Entity"))
                {
                    // Mark for deletion, but don't delete yet!
                    er->entityToDelete = entity;
                }
                ImGui::EndPopup();
            }

            if (opened)
            {
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

        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
                                               | ImGuiTreeNodeFlags_Framed
                                               | ImGuiTreeNodeFlags_SpanAvailWidth
                                               | ImGuiTreeNodeFlags_AllowItemOverlap
                                               | ImGuiTreeNodeFlags_FramePadding;

        T& component = entity.GetComponent<T>();

        // Create a collapsible header for the component
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
        float lineHeight = ImGui::GetFrameHeight();
        ImGui::Separator();
        bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name);
        ImGui::PopStyleVar();

        // --- Component Settings Menu ---
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
        ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
        if (ImGui::Button("...", ImVec2{ lineHeight, lineHeight }))
        {
            ImGui::OpenPopup("ComponentSettings");
        }

        bool componentRemoved = false;
        if (ImGui::BeginPopup("ComponentSettings"))
        {
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

        // Remove component if needed
        if (componentRemoved)
        {
            // Avoid removal of Tag and Transform
            if constexpr (!std::is_same_v<T, TagComponent> && !std::is_same_v<T, TransformComponent>)
            {
                entity.RemoveComponent<T>();
            }
        }
    }

    // Helper for the "Add Component" dropdown
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
            ImGui::InputText("##Tag", &tag.Tag);
        }

        // --- Scrolling Component Region ---
        // We create a child window to house the components. This allows the component list to
        // scroll while the footer with the 'Add Component' and 'Add Entity' buttons remains fixed.
        // The negative height tells ImGui to use all available space minus the specified amount.
        const float footerHeight = ImGui::GetFrameHeightWithSpacing() * 2.2f;
        ImGui::BeginChild("ComponentsRegion", ImVec2(0, -footerHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

        DrawComponentUI<TransformComponent>("Transform", selectedEnt, [](auto& component)
        {
            // Displaying rotation in degrees
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

        ImGui::EndChild(); // End of ComponentsRegion

        // --- Fixed Footer Region ---
        ImGui::Separator();

        // Button to add a new component to the selected entity
        if (ImGui::Button("Add Component", ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("AddComponentPopup");
        }

        if (ImGui::BeginPopup("AddComponentPopup"))
        {
            DisplayAddComponentEntry<ModelComponent>("Model", selectedEnt);
            DisplayAddComponentEntry<AnimationComponent>("Animation", selectedEnt);
            // Add more component types here!
            ImGui::EndPopup();
        }

        // Button to add a new entity to the scene
        if (ImGui::Button("Add Entity", ImVec2(-1, 0)))
        {
            Entity newEntity = ecs->AddEntity("New Entity");
            // Automatically select the new entity for immediate editing
            ecs->GetResource<EditorResource>()->selectedEntity = newEntity;
        }

        ImGui::End(); // End of Inspector window
    }
}