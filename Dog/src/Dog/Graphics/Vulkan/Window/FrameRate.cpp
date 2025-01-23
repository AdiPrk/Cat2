#include <PCH/pch.h>
#include "FrameRate.h"

using namespace std::chrono;

namespace Dog {

    FrameRateController::FrameRateController(unsigned int targetFrameRate)
        : targetFPS(targetFrameRate) {
        targetFrameDuration = duration<double>(1.0 / targetFrameRate);
    }

    float FrameRateController::WaitForNextFrame() {
        auto now = high_resolution_clock::now();
        auto timeSinceLastFrame = now - m_lastFrameTime;

        // Sleep until approximately just before the targetFrameDuration
        if (timeSinceLastFrame < targetFrameDuration - 1ms) {
            std::this_thread::sleep_for(targetFrameDuration - timeSinceLastFrame - 1ms);
        }

        // Fine-tune with busy-waiting for the last small part
        while (timeSinceLastFrame < targetFrameDuration) {
            now = high_resolution_clock::now();
            timeSinceLastFrame = now - m_lastFrameTime;
        }

        m_lastFrameTime = high_resolution_clock::now();
        return duration<float>(timeSinceLastFrame).count();
    }

    void FrameRateController::SetTargetFPS(unsigned int targetFPS) {
        targetFPS = targetFPS;
        targetFrameDuration = duration<double>(1.0 / targetFPS);
    }

}
