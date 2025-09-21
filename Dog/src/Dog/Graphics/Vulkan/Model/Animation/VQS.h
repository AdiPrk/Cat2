#pragma once

namespace Dog
{

    struct VQS {
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f }; // Identity quaternion
        glm::vec3 translation{ 0.0f };
        float padding1;
        glm::vec3 scale{ 1.0f };
        float padding2;
    };

    // This replaces matrix multiplication for combining transforms
    inline VQS compose(const VQS& parent, const VQS& local) {
        VQS result;

        // Combine scales
        result.scale = parent.scale * local.scale;

        // Combine rotations
        result.rotation = parent.rotation * local.rotation;

        // Combine translations
        // The local translation must be scaled and rotated by the parent's transform
        glm::vec3 scaledLocalTrans = parent.scale * local.translation;
        glm::vec3 rotatedLocalTrans = parent.rotation * scaledLocalTrans;
        result.translation = parent.translation + rotatedLocalTrans;

        return result;
    }

}