#include <PCH/pch.h>

#include "Shader.h"
#include "../Core/Device.h"

#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"
#include "../Uniform/ShaderTypes.h"

namespace Dog
{
	void Shader::CreateShaderModule(Device& device, const std::vector<uint32_t>& code, VkShaderModule* shaderModule)
	{
		//Struct to hold information on how to create this shader module
		VkShaderModuleCreateInfo createInfo{};

		//Set type of object to create to shader module
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		//Set the size of the passed code
		createInfo.codeSize = code.size() * sizeof(uint32_t);

		//Set pointer to code (cast from array of chars to a int32 pointer)
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		//Call create, what each parameter is: (gets Vulcan device handle, Create Info, no allication callbacks, Shader module to create)
		if (vkCreateShaderModule(device, &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		{
			//Throw error if failed
			DOG_CRITICAL("Failed to create shader module");
		}
	}
}