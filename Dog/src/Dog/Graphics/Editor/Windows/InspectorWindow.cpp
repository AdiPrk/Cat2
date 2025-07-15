#include <PCH/pch.h>

#ifndef DOG_SHIP

#include "InspectorWindow.h"
#include "EntitiesWindow.h"
#include "Dog/Scene/sceneManager.h"
#include "Dog/Scene/scene.h"
#include "Dog/Scene/Entity/entity.h"
#include "Dog/Scene/Entity/components.h"
#include "Input/input.h"
#include "Engine.h"
#include "Graphics/Vulkan/Models/ModelLibrary.h"
#include "Graphics/Vulkan/Models/Model.h"
#include "../Editor.h"

namespace Dog {

	void DisplayComponents(Entity entity);

	template <typename T>
	void DisplayAddComponent(Entity entity, const std::string& name);

	template <typename EventType>
	void DragFloat3WithEvent(const char* label, glm::vec3& value, float speed, uint32_t entityID) {
		static glm::vec3 oldValue;
		static bool isDragging = false;

		ImGui::DragFloat3(label, &value.x, speed);

		if (ImGui::IsItemActivated() && !isDragging) {
			oldValue = value;
			isDragging = true;
		}

		if (isDragging && ImGui::IsItemDeactivatedAfterEdit()) {
			isDragging = false;

			if (oldValue != value) {
				PUBLISH_EVENT(EventType, entityID, oldValue.x, oldValue.y, oldValue.z, value.x, value.y, value.z);
			}
		}
	}


	void UpdateInspectorWindow() {
		ImGui::Begin("Inspector");

		// lock input if this window is focused
		// log is window focused
		Engine::Get().GetEditor().CaptureInput(ImGui::IsWindowFocused());

		static Entity lastSelectedEntity;
		Entity selectedEntity = GetSelectedEntity();

		if (!selectedEntity) {
			ImGui::Text("No entity selected.");
			ImGui::End(); // Inspector
			return;
		}

		ImGui::PushID(selectedEntity);

		// Display components
		DisplayComponents(selectedEntity);

		// Add components button
		if (ImGui::Button("Add Component")) {
			ImGui::OpenPopup("AddComponentPopup");
		}

		if (ImGui::BeginPopup("AddComponentPopup")) {
			DisplayAddComponent<TagComponent>(selectedEntity, "Tag");
			DisplayAddComponent<TransformComponent>(selectedEntity, "Transform");
			DisplayAddComponent<ModelComponent>(selectedEntity, "Model");
			DisplayAddComponent<MaterialComponent>(selectedEntity, "Material");
			// DisplayAddComponent<SpriteComponent>(selectedEntity, "Sprite");
			// DisplayAddComponent<ShaderComponent>(selectedEntity, "Shader");
			// DisplayAddComponent<CameraComponent>(selectedEntity, "Camera");

			ImGui::EndPopup();
		}

		ImGui::PopID();

		ImGui::End(); // Inspector
	}

	template <typename T>
	void DisplayAddComponent(Entity entity, const std::string& name) {
		if (!entity.HasComponent<T>() && ImGui::MenuItem(name.c_str())) {
			entity.AddComponent<T>();
		}
	}

	void RenderTagComponent(Entity entity, TagComponent& tagComponent) {
		std::string& tag = tagComponent.Tag;

		char buffer[256];

		strncpy_s(buffer, tag.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';

		if (ImGui::InputText("Name##TagProp", buffer, sizeof(buffer))) {
			tag = std::string(buffer);
		}

		/*bool imguiDrawCursor = false;
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("The entity's name.");
			imguiDrawCursor = true;
		}

		ImGui::GetIO().MouseDrawCursor = imguiDrawCursor;*/

		// get mouse pos
		//ImGuiIO& io = ImGui::GetIO();
		//ImVec2 mousePos = io.MousePos;

		//printf("x, y: %f, %f\n", mousePos.x, mousePos.y);
	}

	void RenderTransformComponent(Entity entity, TransformComponent& transform) {
		ImGui::SetNextItemOpen(false, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Transform##header")) 
		{
			DragFloat3WithEvent<Event::EntityMoved>("Position##TransformProp", transform.Translation, 0.1f, entity);
			DragFloat3WithEvent<Event::EntityRotated>("Rotation##TransformProp", transform.Rotation, 0.1f, entity);
			DragFloat3WithEvent<Event::EntityScaled>("Scale##TransformProp", transform.Scale, 0.1f, entity);
		}
	}

	void RenderModelComponent(Entity entity, ModelComponent& model) {
		ImGui::SetNextItemOpen(false, ImGuiCond_FirstUseEver);

		if (ImGui::CollapsingHeader("Model##header")) {
			auto& ml = Engine::Get().GetModelLibrary();
			uint32_t modelCount = ml.GetModelCount();

			// Begin combo box for model selection
			std::vector<std::string> modelNames;
			std::vector<std::string> viewNames;
			modelNames.reserve(modelCount);
			viewNames.reserve(modelCount);

			// Collect model paths and names
			for (uint32_t i = 0; i < modelCount; i++) {
				auto& str = modelNames.emplace_back(ml.GetModelByIndex(i)->GetPath());
				// Extract file name without path for displaying in the dropdown
				viewNames.emplace_back(str.substr(str.find_last_of('/') + 1));
			}

			// Find the current model's index
			int currentModelIndex = -1;
			for (uint32_t i = 0; i < modelCount; i++) {
				if (model.ModelPath == modelNames[i]) {
					currentModelIndex = i;
					break;
				}
			}

			// Display the selected model name or "None" if no model is selected
			const char* modelPreviewValue = currentModelIndex == -1 ? "None" : viewNames[currentModelIndex].c_str();

			// Model selection dropdown
			if (ImGui::BeginCombo("Model##ModelProp", modelPreviewValue)) {
				for (uint32_t i = 0; i < modelCount; i++) {
					const bool isSelected = currentModelIndex == i;

					if (ImGui::Selectable(viewNames[i].c_str(), isSelected)) {
						currentModelIndex = i;
						model.SetModel(modelNames[i]);
					}

					// Ensure default focus is on the currently selected item
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			// Display material overrides with improved mesh headers
			if (model.MaterialOverrides.size() == 1) {
				// Single material override, no need for a header
				ImGui::InputInt("Albedo Texture##ModelProp", &model.MaterialOverrides[0].AlbedoTexture);
				ImGui::InputInt("Normal Texture##ModelProp", &model.MaterialOverrides[0].NormalTexture);
			}
			else {
				// Multiple material overrides, use collapsible headers for each mesh
				ImGui::Indent();  // Indent for child elements to make them visually distinct

				for (int i = 0; i < model.MaterialOverrides.size(); i++) {
					auto& mat = model.MaterialOverrides[i];
					ImGui::PushID(i);

					// Create a collapsible header for each mesh
					std::string meshHeader = "Mesh " + std::to_string(i) + "##header";
					if (ImGui::CollapsingHeader(meshHeader.c_str(), ImGuiTreeNodeFlags_Bullet)) {
						ImGui::Indent();  // Further indent inside each mesh header

						// Display albedo and normal texture fields for this mesh
						ImGui::PushItemWidth(100.0f);  // Adjust the width as necessary
						ImGui::InputInt("Albedo Texture##ModelProp", &mat.AlbedoTexture);
						ImGui::InputInt("Normal Texture##ModelProp", &mat.NormalTexture);
						ImGui::PopItemWidth();

						ImGui::Unindent();  // Unindent inside each mesh header
					}

					ImGui::PopID();
				}

				ImGui::Unindent();  // End indentation for child elements
			}
		}
	}

	void DisplayComponents(Entity entity) {
		// get all components from the entity
		entt::registry& registry = entity.GetScene()->GetRegistry();

		if (entity.HasComponent<TagComponent>())
			RenderTagComponent(entity, entity.GetComponent<TagComponent>());

		if (entity.HasComponent<TransformComponent>())
			RenderTransformComponent(entity, entity.GetComponent<TransformComponent>());

		if (entity.HasComponent<ModelComponent>())
			RenderModelComponent(entity, entity.GetComponent<ModelComponent>());

		/*if (entity.HasComponent<SpriteComponent>())
			RenderSpriteComponent(entity.GetComponent<SpriteComponent>());

		if (entity.HasComponent<ShaderComponent>())
			RenderShaderComponent(entity.GetComponent<ShaderComponent>());

		if (entity.HasComponent<CameraComponent>())
			RenderCameraComponent(entity.GetComponent<CameraComponent>());*/
	}

}

#endif