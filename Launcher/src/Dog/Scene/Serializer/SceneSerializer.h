#pragma once

namespace Dog {

	class Scene;

	class SceneSerializer {
	public:
		static void Serialize(Scene* scene, const std::string& filepath);
		static void Deserialize(Scene* scene, const std::string& filepath);

	private:
		static void SerializeEntity(YAML::Emitter& out, class Entity* entity);
	};

}
