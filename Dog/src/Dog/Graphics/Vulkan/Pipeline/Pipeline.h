#pragma once

#include "../Core/Device.h"

namespace Dog
{
	class Uniform;
	class Device;

	//Holds also configuration info for a pipeline
	struct PipelineConfigInfo
	{
		//Delete copy operations
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		/*********************************************************************
		 * brief:  Create config info object
		 *********************************************************************/
		PipelineConfigInfo()
			: viewportCreateInfo(),
			inputAssemblyCreateInfo(),
			rasterizationCreateInfo(),
			multisampleCreateInfo(),
			colorBlendAttachment(),
			colorBlendCreateInfo(),
			depthStencilCreateInfo(),
			dynamicStateCreateInfo(),
			dynamicStateEnables(),
            pipeLineLayout(nullptr),
            renderPass(nullptr),
            colorFormat(VK_FORMAT_UNDEFINED),
            depthFormat(VK_FORMAT_UNDEFINED),
            subpass(0)
		{
		};

		VkPipelineViewportStateCreateInfo viewportCreateInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo;
		VkPipelineMultisampleStateCreateInfo multisampleCreateInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo;
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineLayout pipeLineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		VkFormat colorFormat;
		VkFormat depthFormat;
		uint32_t subpass = 0;
	};

	class Pipeline
	{
	public:
		// MUST BE TRUE to recompile shaders!
		/*
		* True: Read from .frag/.vert files and save to .spv files. This *always* recompiles shaders
		* False: Try reading from .spv file directly. This will NOT recompile shaders. If file doesn't exist default to Option 1.
		*/
		static const bool RecompileShaders = true;

		// Shader Directories
		inline static const std::string ShaderDir = "Assets/shaders/";
		inline static const std::string SpvDir = "Assets/shaders/spv/";

		Pipeline(Device& device, VkFormat colorFormat, VkFormat depthFormat, const std::vector<Uniform*>& uniforms, bool wireframe, const std::string& vertFile, const std::string& fragFile);
		Pipeline(Device& device, VkFormat colorFormat, VkFormat depthFormat, const std::vector<Uniform*>& uniforms, bool wireframe, const std::string& vertFile, const std::string& fragFile, const std::string& tescFile, const std::string& teseFile);

		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;

		void Bind(VkCommandBuffer commandBuffer);

		VkPipelineLayout& GetLayout() { return mPipelineLayout; };

	private:

		void CreatePipelineLayout(const std::vector<Uniform*>& uniforms);

		void CreatePipeline(VkFormat colorFormat, VkFormat depthFormat);

		void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

		void CreateGraphicsPipeline(const PipelineConfigInfo& configInfo);
		
		Device& device;				     
		VkPipeline mGraphicsPipeline;		         
		VkShaderModule mVertShaderModule;            
		VkShaderModule mFragShaderModule;            
		VkShaderModule mTessCtrlShaderModule = NULL; 
		VkShaderModule mTessEvalShaderModule = NULL; 
		VkPipelineLayout mPipelineLayout;            
		bool isWireframe;

		// Shader paths
		std::string mVertPath;
		std::string mFragPath;
		std::string mTescPath;
		std::string mTesePath;
		std::string mSpvVertPath;
		std::string mSpvFragPath;
		std::string mSpvTescPath;
		std::string mSpvTesePath;
	};
}