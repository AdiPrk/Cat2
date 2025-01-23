#pragma once

// Simple frame rate controller.

namespace Dog {

    class FrameRateController {
    public:
        explicit FrameRateController(unsigned int targetFrameRate);

        // Returns the time in seconds since the last frame (dt)
        float WaitForNextFrame();

        // Set the target frame rate
        void SetTargetFPS(unsigned int targetFPS);

    private:
        unsigned int targetFPS;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_lastFrameTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> targetFrameDuration;
    };

} // namespace Dog