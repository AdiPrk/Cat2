#include <PCH/pch.h>
#include "WindowResource.h"
#include "Graphics/Window/Window.h"

namespace Dog
{
    WindowResource::WindowResource(int w, int h, std::string name)
        : window(std::make_unique<Window>(w, h, name))
    {
    }
}

