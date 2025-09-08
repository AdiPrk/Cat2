#pragma once

namespace BSR
{
    class Window
    {
    public:
        Window(int width, int height, std::string name);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

    private:
        GLFWwindow* CreateWindow(bool resize = true);
        VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window, VkAllocationCallbacks* allocator = nullptr);

        int m_Width;
        int m_Height;
        std::string m_WindowName;

        GLFWwindow* m_Window;
        VkSurfaceKHR m_Surface;
    };
}
