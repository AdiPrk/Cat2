/*********************************************************************
 * file:   Shader.hpp
 * author: aditya.prakash (aditya.prakash@digipen.edu) and evan.gray (evan.gray@digipen.edu)
 * date:   October 15, 2024
 * Copyright © 2024 DigiPen (USA) Corporation. 
 * 
 * brief:  Handles the creation of shader modules
 *********************************************************************/
#pragma once

namespace Dog
{
  //Fowrard declerations
  class Device;

	/*********************************************************************
	 * param:  filePath: Path of file to read in
	 * return: All characters of the file in a vector
	 *
	 * brief:  Reads file at passed file path into a vector of chars,
	 *		     will throw error if filepath cannot be opened
	 *********************************************************************/
	std::string ReadShader(const std::string& filePath);

	/*********************************************************************
	 * param:  code: Code to create shader with
	 * param:  shaderModule: Gets set to created shader module
	 *
	 * brief:  Creates a shader module object with passed code and sets
	 *				 passed shaderModule to it
	 *********************************************************************/
	void CreateShaderModule(Device& device, const std::vector<uint32_t>& code, VkShaderModule* shaderModule);

	/*********************************************************************
	 * param:  source: The GLSL code to compile
	 * param:  stage: The shader stage to compile for
	 * return: The compiled SPIR-V code
	 *
	 * brief: Compiles the passed GLSL code into SPIR-V code
	 *********************************************************************/
	std::vector<uint32_t> CompileGLSLtoSPV(const std::string& source, EShLanguage stage);
}
