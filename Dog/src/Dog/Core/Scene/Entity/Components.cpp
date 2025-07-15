#include <PCH/pch.h>
#include "components.h"
#include "Dog/engine.h"
#include "Dog/Graphics/Vulkan/Window/Window.h"
#include "Dog/Graphics/Vulkan/Texture/Texture.h"
#include "Dog/Graphics/Vulkan/Models/ModelLibrary.h"
#include "Dog/Graphics/Vulkan/Models/Model.h"

namespace Dog {

	glm::mat4 TransformComponent::mat4() const {
		const float c3 = glm::cos(Rotation.z);
		const float s3 = glm::sin(Rotation.z);
		const float c2 = glm::cos(Rotation.x);
		const float s2 = glm::sin(Rotation.x);
		const float c1 = glm::cos(Rotation.y);
		const float s1 = glm::sin(Rotation.y);
		return glm::mat4{
			{
				Scale.x * (c1 * c3 + s1 * s2 * s3),
				Scale.x * (c2 * s3),
				Scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				Scale.y * (c3 * s1 * s2 - c1 * s3),
				Scale.y * (c2 * c3),
				Scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				Scale.z * (c2 * s1),
				Scale.z * (-s2),
				Scale.z * (c1 * c2),
				0.0f,
			},
			{Translation.x, Translation.y, Translation.z, 1.0f} };
	}

	glm::mat3 TransformComponent::normalMatrix() const {
		const float c3 = glm::cos(Rotation.z);
		const float s3 = glm::sin(Rotation.z);
		const float c2 = glm::cos(Rotation.x);
		const float s2 = glm::sin(Rotation.x);
		const float c1 = glm::cos(Rotation.y);
		const float s1 = glm::sin(Rotation.y);
		const glm::vec3 invScale = 1.0f / Scale;

		return glm::mat3{
			{
				invScale.x * (c1 * c3 + s1 * s2 * s3),
				invScale.x * (c2 * s3),
				invScale.x * (c1 * s2 * s3 - c3 * s1),
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			},
		};
	}

	/*void CameraComponent::SetAsCurrentCamera()
	{
		//
	}

	CameraComponent::CameraComponent()
	{
		float width = (float)Engine::Get().GetWindow().GetWidth();
		float height = (float)Engine::Get().GetWindow().GetHeight();
		float aspectRatio = width / height;

		//orthoCamera = std::make_unique<OrthographicCamera>(aspectRatio, OrthographicSize);
		//perspCamera = std::make_unique<PerspectiveCamera>();
	}

	CameraComponent::CameraComponent(const CameraComponent& other)
		: Projection(other.Projection)
		, OrthographicSize(other.OrthographicSize)
		, OrthographicNear(other.OrthographicNear)
		, OrthographicFar(other.OrthographicFar)
		, PerspectiveFOV(other.PerspectiveFOV)
		, PerspectiveNear(other.PerspectiveNear)
		, PerspectiveFar(other.PerspectiveFar)
		//, orthoCamera(std::make_unique<OrthographicCamera>(*other.orthoCamera))
		//, perspCamera(std::make_unique<PerspectiveCamera>(*other.perspCamera))
	{
	}

	CameraComponent::CameraComponent(CameraType type)
		: Projection(type)
	{
		float width = (float)Engine::Get().GetWindow().GetWidth();
		float height = (float)Engine::Get().GetWindow().GetHeight();
		float aspectRatio = width / height;

		//orthoCamera = std::make_unique<OrthographicCamera>(aspectRatio, OrthographicSize);
		//perspCamera = std::make_unique<PerspectiveCamera>();
	}

	void CameraComponent::UpdateCamera()
	{

	}*/

	ModelComponent::ModelComponent()
		: ModelIndex(ModelLibrary::INVALID_MODEL_INDEX)
	{
	}

	ModelComponent::ModelComponent(const ModelComponent& other)
		: ModelIndex(ModelLibrary::INVALID_MODEL_INDEX)
		, ModelPath(other.ModelPath)
		, MaterialOverrides(other.MaterialOverrides)
	{
	}

	ModelComponent::ModelComponent(const std::string& modelPath)
		: ModelIndex(ModelLibrary::INVALID_MODEL_INDEX)
		, ModelPath(modelPath)
	{
		// Get the model's index from the model library
		ModelIndex = Engine::Get().GetModelLibrary().AddModel(modelPath);

		SetMaterialOverrides();
	}

	void ModelComponent::SetModel(const std::string& modelPath)
	{
		ModelPath = modelPath;
		ModelIndex = Engine::Get().GetModelLibrary().AddModel(ModelPath);

		SetMaterialOverrides();
	}

	void ModelComponent::SetMaterialOverrides()
	{
		// get model
		Model* model = Engine::Get().GetModelLibrary().GetModelByIndex(ModelIndex);
		if (model)
		{
			MaterialOverrides.resize(model->meshes.size());

			for (int i = 0; i < MaterialOverrides.size(); ++i)
			{
				MaterialOverrides[i].AlbedoTexture = model->meshes[i].materialComponent.AlbedoTexture;
				MaterialOverrides[i].NormalTexture = model->meshes[i].materialComponent.NormalTexture;
			}
		}
	}

	void CameraComponent::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
	{
		ProjectionMatrix = glm::mat4{ 1.0f };
		ProjectionMatrix[0][0] = 2.f / (right - left);
		ProjectionMatrix[1][1] = 2.f / (bottom - top);
		ProjectionMatrix[2][2] = 1.f / (far - near);
		ProjectionMatrix[3][0] = -(right + left) / (right - left);
		ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		ProjectionMatrix[3][2] = -near / (far - near);
	}

	void CameraComponent::SetPerspectiveProjection(float fovy, float aspect, float near, float far)
	{
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		const float tanHalfFovy = tan(fovy / 2.f);
		ProjectionMatrix = glm::mat4{ 0.0f };
		ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
		ProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
		ProjectionMatrix[2][2] = far / (far - near);
		ProjectionMatrix[2][3] = 1.f;
		ProjectionMatrix[3][2] = -(far * near) / (far - near);
	}

	void CameraComponent::SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
	{
		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
		const glm::vec3 v{ glm::cross(w, u) };

		ViewMatrix = glm::mat4{ 1.f };
		ViewMatrix[0][0] = u.x;
		ViewMatrix[1][0] = u.y;
		ViewMatrix[2][0] = u.z;
		ViewMatrix[0][1] = v.x;
		ViewMatrix[1][1] = v.y;
		ViewMatrix[2][1] = v.z;
		ViewMatrix[0][2] = w.x;
		ViewMatrix[1][2] = w.y;
		ViewMatrix[2][2] = w.z;
		ViewMatrix[3][0] = -glm::dot(u, position);
		ViewMatrix[3][1] = -glm::dot(v, position);
		ViewMatrix[3][2] = -glm::dot(w, position);

		InverseViewMatrix = glm::mat4{ 1.f };
		InverseViewMatrix[0][0] = u.x;
		InverseViewMatrix[0][1] = u.y;
		InverseViewMatrix[0][2] = u.z;
		InverseViewMatrix[1][0] = v.x;
		InverseViewMatrix[1][1] = v.y;
		InverseViewMatrix[1][2] = v.z;
		InverseViewMatrix[2][0] = w.x;
		InverseViewMatrix[2][1] = w.y;
		InverseViewMatrix[2][2] = w.z;
		InverseViewMatrix[3][0] = position.x;
		InverseViewMatrix[3][1] = position.y;
		InverseViewMatrix[3][2] = position.z;
	}

	void CameraComponent::SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
	{
		SetViewDirection(position, target - position, up);
	}

	void CameraComponent::SetViewYXZ(glm::vec3 position, glm::vec3 rotation)
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
		ViewMatrix = glm::mat4{ 1.f };
		ViewMatrix[0][0] = u.x;
		ViewMatrix[1][0] = u.y;
		ViewMatrix[2][0] = u.z;
		ViewMatrix[0][1] = v.x;
		ViewMatrix[1][1] = v.y;
		ViewMatrix[2][1] = v.z;
		ViewMatrix[0][2] = w.x;
		ViewMatrix[1][2] = w.y;
		ViewMatrix[2][2] = w.z;
		ViewMatrix[3][0] = -glm::dot(u, position);
		ViewMatrix[3][1] = -glm::dot(v, position);
		ViewMatrix[3][2] = -glm::dot(w, position);

		InverseViewMatrix = glm::mat4{ 1.f };
		InverseViewMatrix[0][0] = u.x;
		InverseViewMatrix[0][1] = u.y;
		InverseViewMatrix[0][2] = u.z;
		InverseViewMatrix[1][0] = v.x;
		InverseViewMatrix[1][1] = v.y;
		InverseViewMatrix[1][2] = v.z;
		InverseViewMatrix[2][0] = w.x;
		InverseViewMatrix[2][1] = w.y;
		InverseViewMatrix[2][2] = w.z;
		InverseViewMatrix[3][0] = position.x;
		InverseViewMatrix[3][1] = position.y;
		InverseViewMatrix[3][2] = position.z;
	}

} // namespace Dog
