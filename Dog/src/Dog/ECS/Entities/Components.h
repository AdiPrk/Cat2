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
		uint32_t ModelIndex = 9999;
	};

	struct CameraComponent
	{
		float FOV = 45.0f;
		float Near = 0.1f;
		float Far = 1000.0f;
	};
}
