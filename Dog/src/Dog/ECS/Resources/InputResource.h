#pragma once

#include "IResource.h"

struct GLFWwindow;

namespace Dog
{
    struct InputResource : public IResource
    {
        InputResource(GLFWwindow* win) : window(win) {}

        GLFWwindow* window;
    };
}
