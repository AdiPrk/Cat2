#pragma once

namespace Dog {

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag)
		{
		}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
			: Translation(translation)
			, Rotation(rotation)
			, Scale(scale)
		{
		}

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
		uint32_t ModelIndex;

		ModelComponent();
	};
}
