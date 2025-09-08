#include <PCH/pch.h>
#include "Window.h"

namespace BSR
{
    Window::Window(int width, int height, std::string name)
        : m_Width(width)
        , m_Height(height)
        , m_WindowName(name)
        , m_Window(CreateWindow())
        , m_Surface(VK_NULL_HANDLE)
    {
        
    }
    Window::~Window()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    GLFWwindow* Window::CreateWindow(bool resize) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        if (!resize) glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        return glfwCreateWindow(1024, 1024, m_WindowName.c_str(), NULL, NULL);
    }
    
    VkSurfaceKHR Window::CreateSurface(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator) {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkResult err = glfwCreateWindowSurface(instance, window, allocator, &surface);
        if (err) {
            const char* error_msg;
            int ret = glfwGetError(&error_msg);
            if (ret != 0) {
                std::cout << ret << " ";
                if (error_msg != nullptr) std::cout << error_msg;
                std::cout << "\n";
            }
            surface = VK_NULL_HANDLE;
        }
        return surface;
    }
}