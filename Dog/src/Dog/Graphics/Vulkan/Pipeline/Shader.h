#pragma once

namespace Dog
{
	class Device;

	struct Shader
	{
		static void CreateShaderModule(Device& device, const std::vector<uint32_t>& code, VkShaderModule* shaderModule);
	};
}
