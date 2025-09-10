#pragma once

namespace Dog
{
	class Device;

	struct Shader
	{
		static std::string ReadShader(const std::string& filePath);
		static void CreateShaderModule(Device& device, const std::vector<uint32_t>& code, VkShaderModule* shaderModule);
		static std::vector<uint32_t> CompileGLSLtoSPV(const std::string& source, EShLanguage stage, std::string debugFileName = "");
	};
}
