#pragma once

#ifndef DOG_SHIP

namespace Dog {

	class FileBrowser;
	class TextEditorWrapper;
	class ChatWindow;

	class Editor {
	public:
		Editor();
		~Editor();

		static Editor& Get() {
			static Editor instance;
			return instance;
		}

		Editor(const Editor&) = delete;
		Editor& operator=(const Editor&) = delete;

		void Init();
		void Exit();

		void RefreshProjectList();

		void BeginFrame();
		void EndFrame(VkCommandBuffer commandBuffer);

		void DrawFPS();

		void SetEditorEnabled(bool enabled) { renderEditor = enabled; }
		bool GetEditorEnabled() const { return renderEditor; }

		void UpdateVisibility(unsigned windowWidth, unsigned windowHeight);
		bool IsActive() const { return isActive; }

		void DoSceneResize() { doResize = true; }

		void CaptureInput(bool capture) { 
			m_CapturingInput = m_CapturingInput || capture; 
		}

        void LaunchApplication(const std::string& appName);
        bool ShouldLoadProject() const { return m_ShouldLoadProject; }
        const std::string& GetProjectToOpen() const { return projectToOpen; }

		VkDescriptorPool imGuiDescriptorPool;
		VkDescriptorSetLayout samplerSetLayout;
		
	private:
		bool isActive = true;
		bool doResize = false;

		bool renderEditor = true;
		bool startedRenderingFrame = false;

		bool m_CapturingInput = false;

        std::string projectToOpen;
        bool m_ShouldLoadProject = false;

		int selectedProject = -1; // -1 means no project is selected
		std::vector<std::string> projectNames;  // Store project names
	};

} // namespace Dog

#endif