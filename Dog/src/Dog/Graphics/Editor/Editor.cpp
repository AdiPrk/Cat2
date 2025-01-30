#include <PCH/pch.h>

#ifndef DOG_SHIP

#include "editor.h"
#include "Dog/engine.h"
#include "Dog/Input/input.h"

// #include "Windows/sceneWindow.h"
// #include "Windows/consoleWindow.h"
#include "Windows/EntitiesWindow.h"
#include "Windows/InspectorWindow.h"
// #include "Windows/toolbarWindow.h"
#include "Windows/assetsWindow.h"
// #include "Windows/textEditorWindow.h"
// #include "Windows/noEditorWindow.h"
#include "Windows/ChatWindow.h"

#include "Dog/Scene/sceneManager.h"
#include "Dog/Scene/scene.h"
#include "Dog/Graphics/Vulkan/Window/Window.h"
#include "Dog/Graphics/Vulkan/Core/Device.h"
#include "Dog/Graphics/Vulkan/Core/SwapChain.h"

#include "Scene/Serializer/SceneSerializer.h"

#include "ImGuizmo.h"

#include "Scene/Entity/Entity.h"

namespace ImGui {
	/**
	 * Draw an ImGui::Image using Vulkan.
	 *
	 * \param mx: The texture manager to use.
	 * \param index: The index of the texture to use. Index depends on the order in which images and models are loaded.
	 * \param image_size: The size of the image.
	 */
	void VulkanImage(Dog::TextureLibrary& mx, const size_t& index, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
	{
		//auto ds = mx.GetDescriptorSet
		//ImGui::Image(reinterpret_cast<void*>(ds), image_size, uv0, uv1, tint_col, border_col);
	}

	/**
	 * Draw an ImGui::Image using Vulkan.
	 *
	 * \param mx: The texture manager to use.
	 * \param texturePath: The path to the texture to use.
	 * \param image_size: The size of the image.
	 */
	void VulkanImage(Dog::TextureLibrary& mx, const std::string& texturePath, const ImVec2& image_size, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0))
	{
		auto ds = mx.GetDescriptorSet(texturePath);
		ImGui::Image(reinterpret_cast<void*>(ds), image_size, uv0, uv1, tint_col, border_col);
	}

} // namespace ImGui

namespace Dog {
	
	Editor::Editor()
	{
		fileBrowser = std::make_unique<FileBrowser>();
        chatWindow = std::make_unique<ChatWindow>();
		// textEditorWrapper = std::make_unique<TextEditorWrapper>();
	}

	Editor::~Editor()
	{
	}

	void Editor::Init()
	{
		Device& device = Engine::Get().GetDevice();
		Window& window = Engine::Get().GetWindow();
		SwapChain& swapChain = Engine::Get().GetRenderer().GetSwapChain();

		VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = TextureLibrary::MAX_TEXTURE_COUNT;
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		vkCreateDescriptorPool(device, &pool_info, VK_NULL_HANDLE, &imGuiDescriptorPool);

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 0;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr;  // Use your own sampler

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &samplerLayoutBinding;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &samplerSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout!");
		}

		// init imgui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable multi-viewport / platform windows

		ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = device.getInstance();
		init_info.PhysicalDevice = device.getPhysicalDevice();
		init_info.Device = device;
		init_info.QueueFamily = device.GetGraphicsFamily();
		init_info.Queue = device.graphicsQueue();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = imGuiDescriptorPool;// device.getImGuiDescriptorPool();
		init_info.RenderPass = swapChain.getRenderPass();
		init_info.Subpass = 0;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = static_cast<uint32_t>(swapChain.imageCount());
		init_info.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&init_info);

		ImGui::StyleColorsDark();
	}

	void Editor::Exit()
	{
		Device& device = Engine::Get().GetDevice();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		vkDestroyDescriptorSetLayout(device, samplerSetLayout, nullptr);
		vkDestroyDescriptorPool(device, imGuiDescriptorPool, nullptr);
	}

	void Editor::BeginFrame()
	{
		m_CapturingInput = false;
		Input::SetKeyInputLocked(m_CapturingInput);
		Input::SetMouseInputLocked(m_CapturingInput);

		if (!isActive) {
			return;
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();


		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		UpdateGuizmo();

		ImGui::BeginMainMenuBar();

		// Variable to track whether the popup should be open
		static bool openSaveScenePopup = false;

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene")) {}
			
			// Clicking on "Save Scene" opens the popup
			if (ImGui::MenuItem("Save Scene"))
			{
				openSaveScenePopup = true;
			}

			// open scene, which shows all scenes
			if (ImGui::BeginMenu("Open Scene")) 
			{
				// get all files in the scene directory (std filesystem)
				std::vector<std::string> files;
				for (const auto& entry : std::filesystem::directory_iterator("assets/scenes")) {
					std::string path = entry.path().string();

					std::string extension = path.substr(path.find_last_of(".") + 1);
					if (extension == "yaml") {
						files.push_back(path.substr(0, path.find_last_of(".")));
					}
				}

				// display all files in the scene directory
				for (const auto& file : files)
				{
					// remove the directory path
					std::string fileName = file.substr(file.find_last_of("/\\") + 1);

					if (ImGui::MenuItem(fileName.c_str())) 
					{
						Scene* currentScene = SceneManager::GetCurrentScene();

						if (currentScene != nullptr)
						{
							std::string sceneName = currentScene->GetName();
							
							SceneSerializer::Deserialize(currentScene, Assets::ScenesPath + fileName + ".yaml");
							ResetSelectedEntity();
						}
						else 
						{
							DOG_WARN("No scene to deserialize to????");
						}

					}
				}

				ImGui::EndMenu(); // Open Scene
			}


			if (ImGui::MenuItem("Exit")) {
				Engine::Get().Exit();
			}
			if (ImGui::MenuItem("Create Asset Pack")) {
				// DogFilePacker::packageAssets("DogAssets", "fetch");
			}
			ImGui::EndMenu();
		}

		/*ImVec2 textSize = ImGui::CalcTextSize(fpsText, NULL, true);
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textSize.x - 10);
		ImGui::Text(fpsText);*/

		ImGui::EndMainMenuBar(); // File
		
		// Open the Save Scene popup if the flag is set
		if (openSaveScenePopup)
		{
			ImGui::OpenPopup("Save Scene As");
			openSaveScenePopup = false; // Reset flag to prevent reopening every frame
		}


		// Render the modal popup outside of the menu
		if (ImGui::BeginPopupModal("Save Scene As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			CaptureInput(true);

			std::string currName = SceneManager::GetCurrentScene()->GetName().c_str();
			
			static char sceneName[32] = "scene";

			// Initialize the scene name only once when the modal first opens
			if (ImGui::IsWindowAppearing()) {
				strncpy_s(sceneName, currName.c_str(), sizeof(sceneName) - 1);
				sceneName[sizeof(sceneName) - 1] = '\0';
			}

			ImGui::InputText("Scene Name", sceneName, IM_ARRAYSIZE(sceneName));

			bool firstButton = true;
			int numButton = 0;
			
			for (const auto& entry : std::filesystem::directory_iterator(Assets::ScenesPath)) {
				std::string path = entry.path().string();

				std::string extension = path.substr(path.find_last_of(".") + 1);
				if (extension == "yaml") {
					std::string fileName = path.substr(path.find_last_of("/\\") + 1);
					fileName = fileName.substr(0, fileName.find_last_of("."));
					
					if (!firstButton && numButton % 4 != 0) {
						ImGui::SameLine();
					}
					
					if (ImGui::Button(fileName.c_str())) {
						strcpy_s(sceneName, fileName.c_str());
					}

					firstButton = false;
					++numButton;
				}
			}

			// draw a horizontal line through
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 padding = ImGui::GetStyle().WindowPadding;
			draw_list->AddLine(
				ImVec2(p.x, p.y), 
				ImVec2(p.x + ImGui::GetWindowWidth() - padding.x * 2.f, p.y), 
				IM_COL32(255, 255, 255, 255), 
				1.0f);
			ImGui::Spacing();

			if (ImGui::Button("Save", ImVec2(120, 0)))
			{
				// Save the scene
				Scene* currentScene = SceneManager::GetCurrentScene();
				if (currentScene != nullptr)
				{
					std::string sceneNameStr = sceneName;
					SceneSerializer::Serialize(currentScene, Assets::ScenesPath + sceneNameStr + ".yaml");
				}
				else
				{
					DOG_WARN("No scene to save!");
				}

				ImGui::CloseCurrentPopup();  // Close popup after saving
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();  // Close popup if canceled
			}

			ImGui::EndPopup();
		}

		// Update and render all the different windows
		// UpdateSceneWindow(doResize);
		// UpdateConsoleWindow();
		UpdateEntitiesWindow();
		UpdateInspectorWindow();
		// UpdateToolbarWindow();
		UpdateAssetsWindow(*fileBrowser);
		chatWindow->Render();
		// UpdateTextEditorWindow(*textEditorWrapper);

		ImGui::Begin("Textures");

		TextureLibrary& mx = Engine::Get().GetTextureLibrary();
		
		ImGui::VulkanImage(mx, "assets/textures/dog.png", ImVec2(400, 400));

		ImGui::End();

		DrawFPS();

		Input::SetKeyInputLocked(m_CapturingInput);
		Input::SetMouseInputLocked(m_CapturingInput);

	}

	void Editor::EndFrame(VkCommandBuffer commandBuffer)
	{
		if (!isActive) {
			return;
		}

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

		// platform windwos
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void Editor::DrawFPS()
	{
		// Store FPS history (up to 100 samples)
		static std::vector<float> fpsHistory(200, 0);
		static const int maxFPSHistory = 200;
		static float timeElapsed = 0.0f;
		static const float timeDelay = 0.016f;

		// Accumulate time passed since the last update
		timeElapsed += ImGui::GetIO().DeltaTime;

		// Only push FPS value to history every `timeDelay` seconds
		if (timeElapsed >= timeDelay) {
			fpsHistory.push_back(ImGui::GetIO().Framerate);
			if (fpsHistory.size() > maxFPSHistory)
				fpsHistory.erase(fpsHistory.begin());

			timeElapsed = 0.0f;  // Reset the timer
		}

		// Get the main viewport and its size
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		// Set the next window position relative to the main viewport
		ImVec2 windowSize(200, 100);  // You can adjust this size as needed
		ImVec2 windowPos = ImVec2(viewport->Pos.x + viewport->Size.x - windowSize.x,
			viewport->Pos.y + viewport->Size.y - windowSize.y);

		ImGui::SetNextWindowPos(windowPos);
		ImGui::SetNextWindowSize(windowSize);
		ImGui::SetNextWindowViewport(viewport->ID); // Ensure window stays within the main viewport

		// Begin the FPS window with no decoration, no movement, and no background
		ImGui::Begin("FPS Plot", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking);

		// Add the line plot
		ImGui::PlotLines("##FPS", fpsHistory.data(), (int)fpsHistory.size(), 0, NULL, 0.0f, 120.0f, ImVec2(windowSize.x - 20, windowSize.y - 30));

		// Optionally show the current FPS as text
		char fpsText[32];
		sprintf_s(fpsText, "FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text(fpsText);

		// End the FPS window
		ImGui::End();
	}

	void Editor::UpdateGuizmo()
	{
		Scene* currentScene = SceneManager::GetCurrentScene();
		if (!currentScene) return;

		entt::registry& registry = currentScene->GetRegistry();
		std::vector<Entity> selectedEntities;
		/*auto view = registry.view<TransformComponent>(entt::exclude<Parent>);
		for (auto& entityHandle : view) {
			TagComponent& tag = registry.get<TagComponent>(entityHandle);

			selectedEntities.emplace_back(currentScene, entityHandle);
		}*/

		Entity selectedEntity = GetSelectedEntity();
		if (!selectedEntity) return;

		selectedEntities.push_back(selectedEntity);

		Scene* currScene = SceneManager::GetCurrentScene();
		if (!currScene) return;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		// Set the rect to the entire ImGui viewport (fullscreen gizmo manipulation)
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);

		// Camera
		Entity camera = currScene->GetPrimaryCamera();
		if (!camera) return;

		CameraComponent& cam = camera.GetComponent<CameraComponent>();

		glm::mat4 cameraProjection = cam.GetProjection();
		cameraProjection[1][1] *= -1; // Flip the y-axis

		const glm::mat4& cameraView = cam.GetView();

		// Snapping
		bool snap = Input::isKeyDown(Key::LEFTSHIFT);
		float snapValue = 0.1f;
		float snapValues[3] = { snapValue, snapValue, snapValue };

		// 1. Calculate the average or central transform of all selected entities
		glm::vec3 averageTranslation = glm::vec3(0.0f);
		glm::vec3 averageScale = glm::vec3(0.0f);
		glm::quat averageRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
		int rotationCount = 0;

		for (Entity selectedEntity : selectedEntities)
		{
			if (!selectedEntity) continue;

			TransformComponent& tc = selectedEntity.GetComponent<TransformComponent>();
			averageTranslation += tc.Translation;
			averageScale += tc.Scale;

			// Averate rotations
			glm::quat rotationQuat = glm::quat(tc.Rotation);
			if (rotationCount == 0) {
				averageRotation = rotationQuat; // Initialize with first rotation
			}
			else {
				averageRotation = glm::normalize(glm::slerp(averageRotation, rotationQuat, 1.0f / (rotationCount + 1)));
			}
			rotationCount++;
		}

		averageTranslation /= (float)selectedEntities.size();
		averageScale /= (float)selectedEntities.size();

		glm::mat4 groupTransform = glm::translate(glm::mat4(1.0f), averageTranslation) *
								   glm::toMat4(averageRotation) *
								   glm::scale(glm::mat4(1.0f), averageScale);

		// 2. Manipulate the Gizmo for the group transform
		glm::mat4 manipulatedTransform = groupTransform;
		ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
			ImGuizmo::OPERATION::UNIVERSAL, ImGuizmo::WORLD, glm::value_ptr(manipulatedTransform),
			nullptr, snap ? snapValues : nullptr);

		// 3. If Gizmo is being used, apply the transformation to all selected entities
		if (ImGuizmo::IsUsing())
		{
			glm::vec3 newTranslation, newRotationEuler, newScale;
			ImGuizmo::DecomposeMatrixToComponents(
				glm::value_ptr(manipulatedTransform),
				glm::value_ptr(newTranslation),
				glm::value_ptr(newRotationEuler),
				glm::value_ptr(newScale));

			glm::quat newRotation = glm::quat(glm::radians(newRotationEuler));

			// Compute the difference between the old and new group transforms
			glm::vec3 translationDelta = newTranslation - averageTranslation;
			glm::vec3 scaleDelta = newScale / averageScale; // Scale should be proportional
			glm::quat rotationDelta = newRotation * glm::inverse(averageRotation); // Relative rotation

			for (Entity selectedEntity : selectedEntities)
			{
				if (!selectedEntity) continue;

				TransformComponent& tc = selectedEntity.GetComponent<TransformComponent>();

				// Apply translation delta
				tc.Translation += translationDelta;

				// Apply scale delta proportionally to each entity
				tc.Scale *= scaleDelta;

				// Apply the relative rotation delta to each entity
				if (selectedEntities.size() == 1) {
					glm::quat entityRotation = glm::quat(tc.Rotation);
					glm::quat newEntityRotation = entityRotation * rotationDelta;
					tc.Rotation = glm::eulerAngles(newEntityRotation); // Convert back to Euler angles
				}
			}
		}
	}

	void Editor::UpdateVisibility(unsigned windowWidth, unsigned windowHeight)
	{
		ImGuiIO& io = ImGui::GetIO();

		static bool renderEditor = isActive;
		static bool keyHeld = false;
		bool firstGameFrame = false;
		if (io.KeyCtrl && io.KeyShift && io.KeysDown[ImGuiKey_J])
		{
			if (!keyHeld) {
				renderEditor = !renderEditor;
				isActive = renderEditor;
				keyHeld = true;
				firstGameFrame = true;

				if (!renderEditor) {
					PUBLISH_EVENT(Event::SceneResize, (int)windowWidth, (int)windowHeight);
					Input::SetKeyInputLocked(false);
					Input::SetMouseInputLocked(false);
				}
			}
		}
		else {
			keyHeld = false;
		}
	}

} // namespace Dog

#endif // DOG_SHIP
