#pragma once

namespace Dog 
{
	// Class now only holds the paths to important folders
    class Assets
    {
    public:
		inline static const std::string AssetsDir = "assets/";

		inline static const std::string EditorDir = "editor/";
		inline static const std::string ShadersDir = "shaders/";
		inline static const std::string TexturesDir = "textures/";
		inline static const std::string ScenesDir = "scenes/";

		inline static const std::string EditorPath = AssetsDir + "editor/";
		inline static const std::string ShadersPath = AssetsDir + "shaders/";
		inline static const std::string TexturesPath = AssetsDir + "textures/";
		inline static const std::string ScenesPath = AssetsDir + "scenes/";
    };

}
