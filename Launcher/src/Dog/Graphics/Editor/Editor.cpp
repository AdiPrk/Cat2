#include <PCH/pch.h>

#ifndef DOG_SHIP

#include "editor.h"
#include "Dog/engine.h"
#include "Dog/Input/input.h"

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

	void Editor::RefreshProjectList()
	{
		projectNames.clear();
		std::string clientDir = "../Client/";

		// Iterate through all directories in "../Client/"
		for (const auto& entry : std::filesystem::directory_iterator(clientDir))
		{
			if (entry.is_directory())  // Only add directories (projects)
			{
                std::string projectName = entry.path().filename().string();
                if (projectName == "Base" || projectName == "Dev") continue;
                
				projectNames.push_back(projectName);
			}
		}
	}

	void Editor::BeginFrame()
	{
		m_CapturingInput = true;
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

		// Launcher UI state variables
		static bool showCreateProject = false;

		// Configure the main window to cover the entire viewport
		ImVec2 windowSize = ImGui::GetMainViewport()->Size;
		ImVec2 windowPos = ImGui::GetMainViewport()->Pos;
		ImGui::SetNextWindowSize(windowSize);
		ImGui::SetNextWindowPos(windowPos);

		ImGui::Begin("Dog Hub", nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoMove);

		// Determine fixed panel widths
		float leftPanelWidth = windowSize.x * 0.3f;
		float rightPanelWidth = windowSize.x - leftPanelWidth;

		// --- Left Panel: Recent Projects ---
		RefreshProjectList();
		ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth, 0), false);
		{
			ImGui::Text("Recent Projects");
			ImGui::Separator();

            for (int i = 0; i < projectNames.size(); ++i)
            {
                // Mark the project as selected if it matches the stored selectedProject
                if (ImGui::Selectable(projectNames[i].c_str(), selectedProject == i))
                {
                    selectedProject = i;
                }
            }

			ImGui::Spacing();
			if (ImGui::Button("Create New Project", ImVec2(-1, 40)))
			{
				showCreateProject = true;
			}
		}
		ImGui::EndChild();

		// --- Right Panel: Project Details or Welcome Message ---
		ImGui::SameLine();
		ImGui::BeginChild("RightPanel", ImVec2(rightPanelWidth, 0), false);
		{
			if (selectedProject != -1)
			{
				ImGui::Text("Project Details");
				ImGui::Separator();

				// Display details for the selected project.
				// Replace this with your actual project details.
				char projectDetail[128];
				sprintf(projectDetail, "Details for Project %d", selectedProject + 1);
				ImGui::TextWrapped("%s", projectDetail);
				ImGui::Spacing();

				// Button to load the selected project.
				if (ImGui::Button("Load Project", ImVec2(200, 40)))
				{
                    LaunchApplication(projectNames[selectedProject]);
                    printf("Loading project: %s\n", projectNames[selectedProject].c_str());
				}
			}
			else
			{
				ImGui::Text("Welcome to Dog Hub");
				ImGui::Spacing();
				ImGui::TextWrapped("Select a project from the list or create a new one to get started.");
			}
		}
		ImGui::EndChild();

		// --- Create New Project Popup ---
		if (showCreateProject)
		{
			ImGui::OpenPopup("Create New Project");
			showCreateProject = false;
		}
		if (ImGui::BeginPopupModal("Create New Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			static char newProjectName[128] = "";
			static bool invalidProjectName = false;
			static bool projectAlreadyExists = false;
			static bool reservedProjectName = false;
			ImGui::InputText("Project Name", newProjectName, IM_ARRAYSIZE(newProjectName));
            if (invalidProjectName)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Project name cannot be empty!");
            }
            if (reservedProjectName)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Project name is reserved!");
            }
            if (projectAlreadyExists)
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "A project with this name already exists!");
            }
			ImGui::Spacing();
			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				invalidProjectName = false;
				projectAlreadyExists = false;
				reservedProjectName = false;

                std::string pName = newProjectName;
				std::string clientDir = "../Client/";
				std::string projectPath = clientDir + newProjectName;
				std::transform(pName.begin(), pName.end(), pName.begin(), ::tolower);

				if (strlen(newProjectName) == 0)
				{
					invalidProjectName = true;
				}
				else if (pName == "base" || pName == "dev")
				{
					reservedProjectName = true;
				}
				else if (std::filesystem::exists(projectPath))
				{
					projectAlreadyExists = true;
				}
				else
				{
					// Create the new project inside "../Client/"
					std::filesystem::create_directory(projectPath);

					// Copy the contents of "Base" directory into "../Client/{newProjectName}"
					std::filesystem::copy("Base", projectPath, std::filesystem::copy_options::recursive);

					LaunchApplication(pName);

					// Close the popup
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		DrawFPS();
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

	void Editor::LaunchApplication(const std::string& appName)
	{
        projectToOpen = appName;
        m_ShouldLoadProject = true;
	}

} // namespace Dog

#endif // DOG_SHIP
