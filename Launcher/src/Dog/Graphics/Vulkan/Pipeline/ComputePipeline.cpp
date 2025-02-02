/*********************************************************************
 * file:   ComputePipeline.cpp
 * author: evan.gray (evan.gray@digipen.edu)
 * date:   October 15, 2024
 * Copyright © 2024 DigiPen (USA) Corporation.
 *
 * brief:  Pipeline for compute shaders
 *********************************************************************/
#include <PCH/pch.h>

#include "ComputePipeline.hpp"
#include "Backend/Rendering/Uniform/Uniform.hpp"
#include "Backend/Rendering/Uniform/UniformData.hpp"
#include "Shader.hpp"

namespace Dog
{
	ComputePipeline::ComputePipeline(Device& device, const std::vector<Uniform*>& uniforms, const std::string& computeFilePath)
		: mPipelineDevice(device)
		, mPath(computeFilePath)
	{
		CreatePipelineLayout(uniforms);
		CreatePipeline();
	}

	ComputePipeline::~ComputePipeline()
	{
		//Destroy shaders
		vkDestroyShaderModule(mPipelineDevice.GetDevice(), mComputeShaderModule, nullptr);

		//Destroy pipeline
		vkDestroyPipeline(mPipelineDevice.GetDevice(), mComputePipeline, nullptr);

		//Destroy created layout
		vkDestroyPipelineLayout(mPipelineDevice.GetDevice(), mPipelineLayout, nullptr);
	}

	void ComputePipeline::Bind(VkCommandBuffer commandBuffer)
	{
		//Bind the pipeline    
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, mComputePipeline);
	}

	void ComputePipeline::CreatePipelineLayout(const std::vector<Uniform*>& uniforms)
	{
		//Will hold create info for this layout
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};

		//Set what will be created to a pipeline layout
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		//Set uniform binding indexes and make vector of discriptions
		std::vector<VkDescriptorSetLayout> uniformsDescriptorSetLayouts;
		for (int i = 0; i < uniforms.size(); ++i)
		{
			uniforms[i]->SetBinding(i);
			uniformsDescriptorSetLayouts.push_back(uniforms[i]->GetDescriptorLayout()->GetDescriptorSetLayout());
		}

		//Give the pipeline info about the uniform buffer
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(uniformsDescriptorSetLayouts.size()); //Set how many layouts are being provided
		pipelineLayoutCreateInfo.pSetLayouts = uniformsDescriptorSetLayouts.data();                           //Provide layouts

		//Create pipeline
		if (vkCreatePipelineLayout(mPipelineDevice.GetDevice(), &pipelineLayoutCreateInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
		{
			//If failed throw error
      NL_CRITICAL("Failed to create pipeline layout");
		}
	}

	void ComputePipeline::CreatePipeline()
	{
		//Make sure layout and renderpass are real
		if(mPipelineLayout == VK_NULL_HANDLE)
		{
			NL_CRITICAL("Cannot create compute pipeline: no pipelineLayout provided");
		}

		//Read and compile code file
		const std::string& computeCode = ReadShader(mPath);
		std::vector<uint32_t> vertShaderSPV = CompileGLSLtoSPV(computeCode, EShLangCompute);
		CreateShaderModule(mPipelineDevice, vertShaderSPV, &mComputeShaderModule);

		//Make create infos for compute shader stage
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; //Set what will be created to a shader module
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;												  //Set type to vertex shader
		computeShaderStageInfo.module = mComputeShaderModule;															  //Vertex shader to use
		computeShaderStageInfo.pName = "main";																						  //Name of entry function in vertex shader
		computeShaderStageInfo.flags = 0;																							      //Using no flags//Using no flags
		computeShaderStageInfo.pNext = nullptr;																						  //Curently unsure what these two are used, but not currently using
		computeShaderStageInfo.pSpecializationInfo = nullptr;															  //Curently unsure what these two are used, but not currently using

		//Create pipeline object using everything we have set up
		VkComputePipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO; //Set what will be created to a graphics pipeline
		pipelineCreateInfo.stage = computeShaderStageInfo;												 //Programable shader stages to use
		pipelineCreateInfo.layout = mPipelineLayout;												       //Set data from config

		//Create the pipeline 
		if (vkCreateComputePipelines(mPipelineDevice.GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mComputePipeline) != VK_SUCCESS)
		{
			//If failed throw error
      NL_CRITICAL("Failed to create compute pipeline");
		}
	}
}
