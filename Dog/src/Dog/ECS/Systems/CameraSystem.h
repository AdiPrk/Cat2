#pragma once

#include "ISystem.h"

namespace Dog
{
    class CameraSystem : public ISystem
    {
    public:
        CameraSystem() : ISystem("CameraSystem") {};
        ~CameraSystem() {}

        void Update(float dt);
        
    private:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        KeyMappings mKeys{};
        float mMoveSpeed{ 20.f };
        float mLookSpeed{ 1.5f };
    };
}