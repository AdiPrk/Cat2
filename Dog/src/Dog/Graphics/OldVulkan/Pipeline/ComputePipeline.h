/*********************************************************************
 * file:   ComputePipeline.hpp
 * author: evan.gray (evan.gray@digipen.edu)
 * date:   October 15, 2024
 * Copyright © 2024 DigiPen (USA) Corporation. 
 * 
 * brief:  Pipeline for compute shaders
 *********************************************************************/
#pragma once

#include "../Core/Device.h"

namespace Dog
{
//Forward declerations
	class Uniform;

	class ComputePipeline
	{
		public:
			//Remove copy constructor/operation from class
			ComputePipeline(const ComputePipeline&) = delete;
			ComputePipeline& operator=(const ComputePipeline&) = delete;

			/*********************************************************************
				* param:  device: Device this pipeline uses
				* param:  uniforms ComputePipeline's uniforms
				* param:  computeFilePath: Path to compute shader code file
				*
				* brief:  Creates a new Graphics pipeline
				*********************************************************************/
			ComputePipeline(Device& device, const std::vector<Uniform*>& uniforms, const std::string& computeFilePath);

			/*********************************************************************
				* brief:  Destroys this pipeline object
				*********************************************************************/
			~ComputePipeline();

			/*********************************************************************
				* param:  commandBuffer: Command buffer to bind too
				*
				* brief:  Binds this pipeline to passed command buffer for rendering
				*********************************************************************/
			void Bind(VkCommandBuffer commandBuffer);

			////////////////////////////////////////////////////////////////////////////////////////////
			/// Getters ////////////////////////////////////////////////////////////////////////////////

			/*********************************************************************
				* return: Layout of the pipeline
				*
				* brief:  Returns the layout of the pipeline
				*********************************************************************/
			VkPipelineLayout& GetLayout() { return mPipelineLayout; };

		private:

			////////////////////////////////////////////////////////////////////////////////////////////
			/// Helpers ////////////////////////////////////////////////////////////////////////////////

			/*********************************************************************
				* param:  uniforms: uniforms of this pipeline
				*
				* brief:  Creates the layout for this simpleRenderSystems pipeline
				*********************************************************************/
			void CreatePipelineLayout(const std::vector<Uniform*>& uniforms);

			/*********************************************************************
				* brief: Creates this compute pipeline
				*********************************************************************/
			void CreatePipeline();

			////////////////////////////////////////////////////////////////////////////////////////////
			/// Varibles ///////////////////////////////////////////////////////////////////////////////

			Device& mPipelineDevice;						 //Holds reference to the device this pipeline uses
      std::string mPath;                  //Path to compute shader code file
			VkPipeline mComputePipeline;				 //Handle to Vulkan pipeline object
			VkShaderModule mComputeShaderModule; //Compute shader
			VkPipelineLayout mPipelineLayout;    //Pipeline's layout (Holds data about the programable shaders and sending data to them)
	};
}
