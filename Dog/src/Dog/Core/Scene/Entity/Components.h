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

	struct MaterialComponent
	{
		int AlbedoTexture = INVALID_TEXTURE_INDEX;
		int NormalTexture = INVALID_TEXTURE_INDEX;
	};

	class Mesh;
	struct ModelComponent
	{
		uint32_t ModelIndex;
		std::string ModelPath;
		std::vector<MaterialComponent> MaterialOverrides;

		ModelComponent();
		ModelComponent(const ModelComponent& other);
		ModelComponent(const std::string& modelPath);

		void SetModel(const std::string& modelPath);

	private:
		void SetMaterialOverrides();
	};

	struct CameraComponent
	{
		// The camera that is currently being used to render the scene
		bool MainCamera = true;

		enum class CameraType
		{
			Orthographic = 0,
			Perspective
		};

		CameraType Projection = CameraType::Perspective;

		float OrthographicLeft = -1.0f;
		float OrthographicRight = 1.0f;
		float OrthographicBottom = 1.0f;
		float OrthographicTop = -1.0f;
		float OrthographicNear = -1.0f;
		float OrthographicFar = 1.0f;

		float PerspectiveFOV = 45.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 1000.0f;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(float fov, float near, float far)
			: PerspectiveFOV(fov)
			, PerspectiveNear(near)
			, PerspectiveFar(far) {}
		CameraComponent(float left, float right, float bottom, float top, float near, float far)
			: OrthographicLeft(left)
			, OrthographicRight(right)
			, OrthographicBottom(bottom)
			, OrthographicTop(top)
			, OrthographicNear(near)
			, OrthographicFar(far) {}

		glm::mat4 ProjectionMatrix{ 1.f };
		glm::mat4 ViewMatrix{ 1.f };
		glm::mat4 InverseViewMatrix{ 1.f };

		const glm::mat4& GetProjection() const { return ProjectionMatrix; }
		const glm::mat4& GetView() const { return ViewMatrix; }
		const glm::mat4& GetInverseView() const { return InverseViewMatrix; }

	public:
		void SetOrthographicProjection(
			float left, float right, float top, float bottom, float near, float far);
		void SetPerspectiveProjection(float fovy, float aspect, float near, float far);

		void SetViewDirection(
			glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void SetViewTarget(
			glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);
	};

	/*class OrthographicCamera;
	class PerspectiveCamera;

	struct CameraComponent
	{
		void SetAsCurrentCamera();

		enum class CameraType
		{
			Orthographic = 0,
			Perspective
		};

		bool MainCamera = false;

		CameraType Projection = CameraType::Orthographic;

		float OrthographicSize = 1.0f;
		float OrthographicNear = -1.0f;
		float OrthographicFar = 1.0f;

		float PerspectiveFOV = 45.0f;
		float PerspectiveNear = 0.01f;
		float PerspectiveFar = 100.0f;

		std::unique_ptr<OrthographicCamera> orthoCamera;
		std::unique_ptr<PerspectiveCamera> perspCamera;

		CameraComponent();
		CameraComponent(const CameraComponent& other);
		CameraComponent(CameraType type);
		void UpdateCamera();
	};*/

}
