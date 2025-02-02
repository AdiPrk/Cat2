#pragma once

#include "inputMap.h"

struct GLFWwindow;
struct GLFWcursor;

namespace Dog {

	/**
	 * The Input class is meant for use in anything that's inside the Scene.
	 * If the scene is not focused, then the Input class will get locked and not update.
	 * When the Editor is not active, the Input class will always update.
	 */

	class Input {
	public:
		static void Init(GLFWwindow* window);
		static void Update();

		static bool isKeyDown(const Key& key);
		static bool isMouseDown(const Mouse& button);
		static void UpdateMousePosition();

		// screen mouse positions
		static glm::vec2 getMouseScreenPosition() { return { mouseScreenX, mouseScreenY }; }
		static float getMouseScreenX() { return mouseScreenX; }
		static float getMouseScreenY() { return mouseScreenY; }
		static float getMouseScreenXDelta() { return mouseScreenX - lastMouseScreenX; }
		static float getMouseScreenYDelta() { return mouseScreenY - lastMouseScreenY; }

		// world mouse positions
		static glm::vec2 getMouseWorldPosition() { return { mouseWorldX, mouseWorldY }; }
		static float getMouseWorldX() { return mouseWorldX; }
		static float getMouseWorldY() { return mouseWorldY; }
		static float getMouseWorldXDelta() { return mouseWorldX - lastMouseWorldX; }
		static float getMouseWorldYDelta() { return mouseWorldY - lastMouseWorldY; }

		// mouse scroll
		static float getMouseScrollX() { return scrollX; }
		static float getMouseScrollY() { return scrollY; }
		static float getMouseScrollDeltaX() { return scrollX - lastScrollX; }
		static float getMouseScrollDeltaY() { return scrollY - lastScrollY; }

		static void SetKeyInputLocked(bool locked);
		static void SetMouseInputLocked(bool locked);

	private:
		static GLFWwindow* pwindow;

		struct KeyStates
		{
			bool keyDown = false;
			bool keyTriggered = false;
			bool keyReleased = false;
			bool prevTriggered = false;
			bool doTriggered = false;
		};

		struct MouseStates
		{
			bool mouseDown = false;
			bool mouseTriggered = false;
			bool mouseReleased = false;
			bool prevTriggered = false;
			bool doTriggered = false;
		};

		static KeyStates keyStates[static_cast<int>(Key::LAST)];
		static MouseStates mouseStates[static_cast<int>(Mouse::LAST)];
		static GLFWcursor* standardCursor;

		static float scrollX;
		static float scrollY;
		static float lastScrollX;
		static float lastScrollY;
		static float lastMouseScreenX;
		static float lastMouseScreenY;
		static float mouseScreenX;
		static float mouseScreenY;
		static float lastMouseWorldX;
		static float lastMouseWorldY;
		static float mouseWorldX;
		static float mouseWorldY;

		static bool keyInputLocked;
		static bool mouseInputLocked;

		static void keyPressCallback(GLFWwindow* windowPointer, int key, int scanCode, int action, int mod);
		static void mouseButtonCallback(GLFWwindow* windowPointer, int mouseButton, int action, int mod);
		static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	};

}