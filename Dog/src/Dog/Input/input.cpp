#include <PCH/pch.h>
#include "input.h"
#include "inputMap.h"
#include "Dog/Logger/logger.h"

#define DO_INPUT_LOGGING 0

namespace Dog {
	
#define IND(x) static_cast<int>(x)

	Input::KeyStates Input::keyStates[static_cast<int>(Key::LAST)];
	Input::MouseStates Input::mouseStates[static_cast<int>(Mouse::LAST)];

	bool Input::keyInputLocked = false;
	bool Input::mouseInputLocked = false;

	float Input::scrollX = 0;
	float Input::scrollY = 0;
	float Input::lastScrollX = 0;
	float Input::lastScrollY = 0;
	float Input::mouseScreenX = 0;
	float Input::mouseScreenY = 0;
	float Input::lastMouseScreenX = 0;
	float Input::lastMouseScreenY = 0;
	float Input::mouseWorldX = 0;
	float Input::mouseWorldY = 0;
	float Input::lastMouseWorldX = 0;
	float Input::lastMouseWorldY = 0;
	GLFWcursor* Input::standardCursor = nullptr;
	GLFWwindow* Input::pwindow = nullptr;

	void Input::Init(GLFWwindow* window)
	{
		pwindow = window;
		glfwSetKeyCallback(pwindow, keyPressCallback);
		glfwSetMouseButtonCallback(pwindow, mouseButtonCallback);
		glfwSetScrollCallback(pwindow, mouseScrollCallback);

		standardCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
		glfwSetCursor(pwindow, standardCursor);
	}

	struct ViewportInfo {
		GLint x, y, width, height;

		ViewportInfo() : x(), y(), width(), height() {};
		ViewportInfo(GLint info[4]) : x(info[0]), y(info[1]), width(info[2]), height(info[3]) {};
	};

	void Input::Update()
	{
		lastScrollX = scrollX;
		lastScrollY = scrollY;
		glfwPollEvents();
		UpdateMousePosition();
	}

	bool Input::isKeyDown(const Key& key)
	{
		return keyStates[IND(key)].keyDown;
	}

	bool Input::isMouseDown(const Mouse& button)
	{
		return mouseStates[IND(button)].mouseDown;
	}

	void Input::UpdateMousePosition()
	{
		/*
		lastMouseScreenX = mouseScreenX;
		lastMouseScreenY = mouseScreenY;
		lastMouseWorldX = mouseWorldX;
		lastMouseWorldY = mouseWorldY;

		int windowWidth = 0, windowHeight = 0;
		glfwGetWindowSize(pwindow, &windowWidth, &windowHeight);
		if (windowWidth <= 0 || windowHeight <= 0)
			return;

		glm::vec2 windowSize(windowWidth, windowHeight);


		glm::vec2 imguiTranslation = glm::vec2(0.0f, 0.0f);
		glm::vec2 imguiScale = glm::vec2(1.0f, 1.0f);

#ifndef DOG_SHIP
		// check if imgui window was rendered
		auto imguiWin = ImGui::FindWindowByName("Scene");
		if (imguiWin) {
			glm::vec2 relPos = GetRelativeSceneImagePosition();
			imguiTranslation = glm::vec2(imguiWin->Pos.x, imguiWin->Pos.y) + relPos;
			ImVec2 winSize = imguiWin->Size;
			winSize.y -= relPos.y;

			imguiScale = glm::vec2(winSize) / windowSize;
		}
#endif

		// Get mouse coordinates in screen space
		double cursorX, cursorY;
		glfwGetCursorPos(pwindow, &cursorX, &cursorY);
		glm::vec2 screenMousePos(cursorX, cursorY);
		mouseScreenX = static_cast<float>(cursorX);
		mouseScreenY = static_cast<float>(cursorY);

		// Convert mouse screen coords to FBO's coordinates inside ImGui's window
		glm::vec2 framebufferMousePos = (screenMousePos - imguiTranslation) / imguiScale;

		// Convert fbo coords to NDC
		glm::vec2 ndc;
		ndc.x = (2.0f * framebufferMousePos.x) / windowSize.x - 1.0f;
		ndc.y = (2.0f * framebufferMousePos.y) / windowSize.y - 1.0f;
		ndc.y = -ndc.y;

		// Take NDC to world
		Scene* activeScene = SceneManager::GetCurrentScene();
		if (!activeScene) {
			DOG_WARN("No active scene found.");
			return;
		}
		glm::mat4 proj = activeScene->GetProjectionMatrix();
		glm::mat4 view = activeScene->GetViewMatrix();
		glm::mat4 invProjView = glm::inverse(proj * view);

		glm::vec4 worldPos = invProjView * glm::vec4(ndc.x, ndc.y, 0.0f, 1.0f);
		worldPos /= worldPos.w;

		mouseWorldX = worldPos.x;
		mouseWorldY = worldPos.y;
		*/
	}

	void Input::SetKeyInputLocked(bool locked)
	{
		keyInputLocked = locked;
	}

	void Input::SetMouseInputLocked(bool locked)
	{
		mouseInputLocked = locked;
	}

	void Input::keyPressCallback(GLFWwindow* windowPointer, int key, int scanCode, int action, int mod)
	{
		// ImGui_ImplGlfw_KeyCallback(windowPointer, key, scanCode, action, mod);

		// check if imgui is capturing the keyboard
		if (keyInputLocked) {
#if DO_INPUT_LOGGING
			DOG_INFO("Key {0} Ignored - ImGui is capturing it.", key);
#endif
			return;
	}

		if (key < 0 || key > static_cast<int>(Key::LAST))
			return;

		if (action == GLFW_PRESS)
		{
			keyStates[key].keyDown = true;
#if DO_INPUT_LOGGING
			DOG_INFO("Key Pressed: {0}", key);
#endif
		}
		else if (action == GLFW_RELEASE)
		{
			keyStates[key].keyDown = false;
#if DO_INPUT_LOGGING
			DOG_INFO("Key Released: {0}", key);
#endif
		}
	}

	void Input::mouseButtonCallback(GLFWwindow* windowPointer, int mouseButton, int action, int mod)
	{
		// ImGui_ImplGlfw_MouseButtonCallback(windowPointer, mouseButton, action, mod);

		// check if imgui is capturing the mouse
		if (mouseButton < 0 || mouseButton > static_cast<int>(Mouse::LAST))
			return;

		if (action == GLFW_PRESS && !mouseInputLocked)
		{
			mouseStates[mouseButton].mouseDown = true;
#if DO_INPUT_LOGGING
			DOG_INFO("Mouse Pressed: {0}", mouseButton);
#endif
		}
		else if (action == GLFW_RELEASE)
		{
			mouseStates[mouseButton].mouseDown = false;
#if DO_INPUT_LOGGING
			DOG_INFO("Mouse Released: {0}", mouseButton);
#endif
		}
		}

	void Input::mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		// ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

		if (mouseInputLocked) {
#if DO_INPUT_LOGGING
			DOG_INFO("Mouse Scroll Ignored - ImGui is capturing it.");
#endif
			return;
		}

		scrollX += float(xoffset);
		scrollY += float(yoffset);
#if DO_INPUT_LOGGING
		//DOG_INFO("Mouse Scrolled: {0}", degree);
#endif
	}

}