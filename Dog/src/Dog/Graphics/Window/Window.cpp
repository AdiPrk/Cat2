#include <PCH/pch.h>
#include "Window.h"

namespace Dog {

    Window::Window(int w, int h, std::string name) : mWidth{ w }, mHeight{ h }, mWindowName{ name } {
        InitWindow();
    }

    Window::~Window() {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    void Window::InitWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        mWindow = glfwCreateWindow(mWidth, mHeight, mWindowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(mWindow, this);
        glfwSetFramebufferSizeCallback(mWindow, FramebufferResizeCallback);
    }

    void Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, mWindow, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        win->mFramebufferResized = true;
        win->mWidth = width;
        win->mHeight = height;
    }

} // namespace Dog