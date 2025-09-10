/*********************************************************************
 * file:   Shader.cpp
 * author: aditya.prakash (aditya.prakash@digipen.edu) and evan.gray (evan.gray@digipen.edu)
 * date:   October 15, 2024
 * Copyright © 2024 DigiPen (USA) Corporation. 
 * 
 * brief:  Handles the creation of shader modules
 *********************************************************************/
#include <PCH/pch.h>

#include "Shader.h"
#include "../Core/Device.h"

#include "glslang/Public/ResourceLimits.h"
#include "glslang/SPIRV/GlslangToSpv.h"

namespace Dog
{
	// Helper function to read a file's content into a string
	std::string ReadFile(const std::string& filePath) 
	{
		std::ifstream file(filePath);
		if (!file.is_open()) 
		{
			std::string error = "Failed to open file: " + filePath;
			DOG_CRITICAL(error);
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	// Recursive function to preprocess shader source for #include directives
	std::string PreprocessShader(const std::string& shaderSource, const std::string& shaderPath,
		std::unordered_set<std::string>& includedFiles) {
		std::stringstream processedSource;
		std::istringstream sourceStream(shaderSource);
		std::string line;

		// Directory of the current shader file
		std::string shaderDirectory = shaderPath.substr(0, shaderPath.find_last_of("/\\") + 1);

		while (std::getline(sourceStream, line)) {
			// Check for #include directive
			if (line.find("#include") == 0) {
				// Extract the path between quotes
				size_t start = line.find('"');
				size_t end = line.rfind('"');
				if (start == std::string::npos || end == std::string::npos || start >= end) {
					DOG_CRITICAL("Malformed #include directive: {0}", line);
				}

				std::string includePath = line.substr(start + 1, end - start - 1);
				std::string resolvedPath = shaderDirectory + includePath;

				// Skip if this file was already included
				if (includedFiles.find(resolvedPath) != includedFiles.end()) {
					continue;
				}
				includedFiles.insert(resolvedPath);

				// Read the included file's content
				std::string includeSource = ReadFile(resolvedPath);

				// Recursively preprocess the included file
				processedSource << PreprocessShader(includeSource, resolvedPath, includedFiles) << "\n";
			}
			else {
				processedSource << line << "\n";
			}
		}

		return processedSource.str();
	}

	// Public function to preprocess shader, initializes included files tracking
	std::string PreprocessShaderWithIncludes(const std::string& shaderSource, const std::string& shaderPath) {
		std::unordered_set<std::string> includedFiles;
		return PreprocessShader(shaderSource, shaderPath, includedFiles);
	}

	std::string ReadShader(const std::string& filePath)
	{
		//Read file        path       start at end     read as binaryCompile Shaders
		std::ifstream file{ filePath, std::ios::ate | std::ios::binary };

		//Check if not open
		if (!file.is_open())
		{
			//Throw error
			std::string error = "Failed to open file: " + filePath;
			DOG_CRITICAL(error);
		}

		//Get file size (tellg gets last position, which is end since we started there)
		size_t fileSize = static_cast<size_t>(file.tellg());

		//Make a vector big enough to hold full file
		std::vector<char> buffer(fileSize);

		//Go back to start of file and read whole file into vector
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		std::string shaderSource(buffer.begin(), buffer.end());

		return PreprocessShaderWithIncludes(shaderSource, filePath);
	}

	void CreateShaderModule(Device& device, const std::vector<uint32_t>& code, VkShaderModule* shaderModule)
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

	std::vector<uint32_t> CompileGLSLtoSPV(const std::string& source, EShLanguage stage)
	{
		const char* shaderStrings[1];
		shaderStrings[0] = source.c_str();
		glslang::TShader shader(stage);
		shader.setStrings(shaderStrings, 1);

		// macros
		static std::vector<std::pair<std::string, std::string>> macros = 
		{
			{ "NL_MAX_INSTANCES", std::to_string(1000) },
			{ "NL_MAX_GLYPHS", std::to_string(95) },
			{ "NL_MAX_MODELS", std::to_string(100) },
			{ "NL_MAX_TEXTURES", std::to_string(10000) },
			{ "NL_MAX_BONES ", std::to_string(200) },
			{ "NL_MAX_BONE_INFLUENCE ", std::to_string(4) },
			{ "NL_INVALID_FONT_INDEX", std::to_string(9999) },
			{ "NL_INVALID_ANIMATION_INDEX", std::to_string(9999) },
			{ "NL_WIREFRAME_TEXTURE_INDEX", std::to_string(10002) },
			{ "NL_NO_TEXTURES", std::to_string(10001) },
			{ "NL_PATCH_SIZE", std::to_string(4)}
		};

		// Construct the preamble string from the macros
		std::string preamble;
		for (const auto& macro : macros) 
		{
			preamble += "#define " + macro.first + " " + macro.second + "\n";
		}

		// Set the preamble for the shader
		shader.setPreamble(preamble.c_str());

		//Set the target language environment.
		int clientInputSemanticsVersion = 460; // Vulkan semantics
		shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, clientInputSemanticsVersion);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
		shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_6);

		static const TBuiltInResource* Resources = GetDefaultResources();

		EShMessages messages = EShMsgSuppressWarnings;
		if (!shader.parse(Resources, 460, false, messages)) 
		{
			std::string infoLog = shader.getInfoLog();
			std::string infoDebugLog = shader.getInfoDebugLog();

			DOG_CRITICAL("GLSL Parsing Failed:\n{0}\n{1}", infoLog, infoDebugLog);
		}

		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages)) 
		{
			std::string infoLog = program.getInfoLog();
			std::string infoDebugLog = program.getInfoDebugLog();
			std::string error = "GLSL Linking Failed:\n" + infoLog + "\n" + infoDebugLog;
			DOG_CRITICAL(error);
		}

		std::vector<uint32_t> spirv;
		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

		return spirv;
	}
}