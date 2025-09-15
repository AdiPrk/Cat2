#include <PCH/pch.h>
//#include "Graphics/Vulkan/Window/Window.h"
#include "KeyboardController.h"
#include "ECS/Entities/Components.h"
#include "ECS/Systems/InputSystem.h"

namespace Dog {

    void KeyboardMovementController::moveInPlaneXZ(
        GLFWwindow* window, float dt, TransformComponent& transform) {
        glm::vec3 rotate{ 0 };
        if (InputSystem::isKeyDown(Key::RIGHT)) rotate.y += 1.f;
        if (InputSystem::isKeyDown(Key::LEFT)) rotate.y -= 1.f;
        if (InputSystem::isKeyDown(Key::UP)) rotate.x += 1.f;
        if (InputSystem::isKeyDown(Key::DOWN)) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            transform.Rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        transform.Rotation.x = glm::clamp(transform.Rotation.x, -1.5f, 1.5f);
        transform.Rotation.y = glm::mod(transform.Rotation.y, glm::two_pi<float>());

        float yaw = transform.Rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };

        glm::vec3 moveDir{ 0.f };

        if (InputSystem::isKeyDown(Key::W)) moveDir += forwardDir;
        if (InputSystem::isKeyDown(Key::S)) moveDir -= forwardDir;
        if (InputSystem::isKeyDown(Key::D)) moveDir += rightDir;
        if (InputSystem::isKeyDown(Key::A)) moveDir -= rightDir;
        if (InputSystem::isKeyDown(Key::E)) moveDir += upDir;
        if (InputSystem::isKeyDown(Key::Q)) moveDir -= upDir;

        // check shift
        moveSpeed = (InputSystem::isKeyDown(Key::LEFTSHIFT)) ? 1.f : 4.f;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            transform.Translation += moveSpeed * dt * glm::normalize(moveDir);
        }

        // log translation
    }

} // namespace Dog