#include <PCH/pch.h>

#ifndef DOG_SHIP

#include "entitiesWindow.h"
#include "Dog/Scene/sceneManager.h"
#include "Dog/Scene/scene.h"
#include "Dog/Scene/Entity/entity.h"
#include "Dog/Scene/Entity/components.h"

namespace Dog {

	Entity selectedEntity;

	Entity GetSelectedEntity() {
		return selectedEntity;
	}

	void DrawNewItemTree(Scene* scene, bool fromMenuBar) {
		if (fromMenuBar) {
			ImGui::BeginMenuBar();
		}

		bool beganMenuBarAdd = false;
		if (fromMenuBar) {
			beganMenuBarAdd = ImGui::BeginMenu("Add");
		}

		if (!fromMenuBar || beganMenuBarAdd) {
			if (ImGui::BeginMenu("2D Objects")) {

				if (ImGui::BeginMenu("Sprites")) {

					if (ImGui::MenuItem("Square")) {
						Entity entity = scene->CreateEntity("Square");
						selectedEntity = entity;
					}
					/*if (ImGui::MenuItem("Circle")) {
						Entity entity = scene->CreateEntity("Circle");
						entity.GetComponent<SpriteComponent>().texturePath = "circle.png";
						selectedEntity = entity;
					}*/

					ImGui::EndMenu(); // Sprites
				}

				ImGui::EndMenu(); // 2D Objects
			}
		}

		if (beganMenuBarAdd) {
			ImGui::EndMenu();
		}

		if (fromMenuBar) {
			ImGui::EndMenuBar();
		}
	}

	void UpdateEntitiesWindow() {
		ImGui::Begin("Entities##window", nullptr, ImGuiWindowFlags_MenuBar);

		Scene* currentScene = SceneManager::GetCurrentScene();
		if (!currentScene) {
			ImGui::Text("No scene loaded");
			ImGui::End();
			return;
		}

		// Get registry
		entt::registry& registry = currentScene->GetRegistry();

		// Draw the Add in the menu bar of this window
		if (ImGui::BeginPopupContextWindow()) {
			DrawNewItemTree(currentScene, false);
			ImGui::EndPopup();
		}
		else {
			DrawNewItemTree(currentScene, true);
		}

		// Iterate through all entities with a tag, and without a parent

		auto view = registry.view<TagComponent>(entt::exclude<Parent>);
		for (auto& entityHandle : view) {
			Entity entity = { currentScene, entityHandle };

			TagComponent& tag = entity.GetComponent<TagComponent>();

			ImGuiTreeNodeFlags leafFlags = ImGuiTreeNodeFlags_Leaf;
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

			if (selectedEntity == entityHandle) {
				leafFlags |= ImGuiTreeNodeFlags_Selected;
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
			}

			bool opened = false;
			if (!entity.HasComponent<Children>()) {
				opened = ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, leafFlags, tag.Tag.c_str());
			}
			else {
				opened = ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, nodeFlags, tag.Tag.c_str());
			}

			if (ImGui::IsItemClicked()) {
				// Select entity
				DOG_INFO("Entity {0} Selected!", tag.Tag.c_str());
				selectedEntity = entity;
			}

			// end treenodex
			if (opened) {
				ImGui::TreePop();
			}
		}


		ImGui::End(); // End of Entities
	}

}

#endif