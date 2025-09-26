#pragma once

namespace Dog {

	struct TagComponent
	{
		std::string Tag;
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		glm::mat4 GetTransform() const
		{
			// SRT - Translate * Rotate * Scale
			return glm::translate(glm::mat4(1.0f), Translation) *
				   glm::toMat4(glm::quat(Rotation)) *
				   glm::scale(glm::mat4(1.0f), Scale);
		}

		glm::mat4 mat4() const;
		glm::mat3 normalMatrix() const;
	};

	struct ModelComponent
	{
		uint32_t ModelIndex = 0;
        glm::vec4 tintColor = glm::vec4(1.0f);
	};

	struct AnimationComponent
	{
        bool IsPlaying = true;
        uint32_t AnimationIndex = 10001; // AnimationLibrary::INVALID_ANIMATION_INDEX
        float AnimationTime = 0.0f;
		bool inPlace = false;

        uint32_t BoneOffset = 0; // Used internally
	};

	struct CameraComponent
	{
		float FOV = 45.0f;
		float Near = 0.1f;
		float Far = 1000.0f;

        glm::vec3 Forward = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

		float Yaw{ 0.0f };
		float Pitch{ 0.0f };
		float MouseSensitivity{ 1.f };
		bool InvertY{ true };
		float MoveSpeed{ 10.f };

		// --- Internal ---
		// in CameraComponent
		float TargetYaw = 0.0f;
		float TargetPitch = 0.0f;

		// Smoothed current values (what we write to transform and use for lookAt)
		float SmoothedYaw = 0.0f;
		float SmoothedPitch = 0.0f;
		glm::vec3 SmoothedPosition = glm::vec3(0.0f);

		// Mouse smoothing buffer
		float SmoothedMouseDX = 0.0f;
		float SmoothedMouseDY = 0.0f;

		// Tweakable smoothing params (tune these)
		float MouseSmoothness = 20.0f; // higher -> less smoothing of raw mouse (more snappy)
		float RotationSmoothness = 18.0f; // higher -> faster rotation follow (less smoothing)
		float PositionSmoothness = 12.0f; // higher -> faster position follow (less smoothing)
		bool isInitialized{ false };
	};
}
