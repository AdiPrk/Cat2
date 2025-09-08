#pragma once

class Window;

namespace BSR
{
    struct RenderingResource
    {
        std::unique_ptr<Window>    window;
        vkb::Instance              instance;
        vkb::InstanceDispatchTable inst_disp;
        VkSurfaceKHR               surface;
        vkb::Device                device;
        vkb::DispatchTable         disp;
        vkb::Swapchain             swapchain;
    };
}
